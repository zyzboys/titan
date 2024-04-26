#include "db/titan_blob_format.h"
namespace ROCKSDB_NAMESPACE {

const uint32_t kTitanCompressionFormat = 2;

bool TitanGoodCompressionRatio(size_t compressed_size, size_t raw_size) {
  // Check to see if compressed less than 12.5%
  return compressed_size < raw_size - (raw_size / 8u);
}

Slice TitanCompress(const CompressionInfo& info, const Slice& input,
               std::string* output, CompressionType* type) {
  *type = info.type();
  if (info.type() == kNoCompression) {
    return input;
  }
  if (!CompressData(input, info, kTitanCompressionFormat, output)) {
    // Compression method is not supported, or not good compression
    // ratio, so just fall back to uncompressed form.
    *type = kNoCompression;
    return input;
  }
  return *output;
}

Status TitanUncompress(const UncompressionInfo& info, const Slice& input,
                  TitanOwnedSlice* output) {
  assert(info.type() != kNoCompression);
  size_t usize = 0;
  CacheAllocationPtr ubuf = UncompressData(info, input.data(), input.size(),
                                           &usize, kTitanCompressionFormat);
  if (!ubuf.get()) {
    return Status::Corruption("Corrupted compressed blob");
  }
  output->reset(std::move(ubuf), usize);
  return Status::OK();
}

bool TitanGetChar(Slice* src, unsigned char* value) {
  if (src->size() < 1) return false;
  *value = *src->data();
  src->remove_prefix(1);
  return true;
}


void TitanBlobRecord::EncodeTo(std::string* dst) const {
  PutLengthPrefixedSlice(dst, key);
  PutLengthPrefixedSlice(dst, value);
}

Status TitanBlobRecord::DecodeFrom(Slice* src) {
  if (!GetLengthPrefixedSlice(src, &key) ||
      !GetLengthPrefixedSlice(src, &value)) {
    return Status::Corruption("TitanBlobRecord");
  }
  return Status::OK();
}

bool operator==(const TitanBlobRecord& lhs, const TitanBlobRecord& rhs) {
  return lhs.key == rhs.key && lhs.value == rhs.value;
}

void TitanBlobEncoder::EncodeRecord(const TitanBlobRecord& record) {
  record_buffer_.clear();
  record.EncodeTo(&record_buffer_);
  EncodeSlice(record_buffer_);
}

void TitanBlobEncoder::EncodeSlice(const Slice& record) {
  compressed_buffer_.clear();
  CompressionType compression;
  record_ =
      TitanCompress(*compression_info_, record, &compressed_buffer_, &compression);

  assert(record_.size() < std::numeric_limits<uint32_t>::max());
  EncodeFixed32(header_ + 4, static_cast<uint32_t>(record_.size()));
  header_[8] = compression;

  uint32_t crc = crc32c::Value(header_ + 4, sizeof(header_) - 4);
  crc = crc32c::Extend(crc, record_.data(), record_.size());
  EncodeFixed32(header_, crc);
}

Status TitanBlobDecoder::DecodeHeader(Slice* src) {
  if (!GetFixed32(src, &crc_)) {
    return Status::Corruption("TitanBlobHeader");
  }
  header_crc_ = crc32c::Value(src->data(), kTitanRecordHeaderSize - 4);

  unsigned char compression;
  if (!GetFixed32(src, &record_size_) || !TitanGetChar(src, &compression)) {
    return Status::Corruption("TitanBlobHeader");
  }

  compression_ = static_cast<CompressionType>(compression);
  return Status::OK();
}

Status TitanBlobDecoder::DecodeRecord(Slice* src, TitanBlobRecord* record,
                                 TitanOwnedSlice* buffer) {
  TEST_SYNC_POINT_CALLBACK("TitanBlobDecoder::DecodeRecord", &crc_);

  Slice input(src->data(), record_size_);
  src->remove_prefix(record_size_);
  uint32_t crc = crc32c::Extend(header_crc_, input.data(), input.size());
  if (crc != crc_) {
    return Status::Corruption("TitanBlobRecord", "checksum mismatch");
  }

  if (compression_ == kNoCompression) {
    return TitanDecodeInto(input, record);
  }
  UncompressionContext ctx(compression_);
  UncompressionInfo info(ctx, *uncompression_dict_, compression_);
  Status s = TitanUncompress(info, input, buffer);
  if (!s.ok()) {
    return s;
  }
  return TitanDecodeInto(*buffer, record);
}

void TitanBlobHandle::EncodeTo(std::string* dst) const {
  PutVarint64(dst, offset);
  PutVarint64(dst, size);
  PutVarint64(dst, order);
}

Status TitanBlobHandle::DecodeFrom(Slice* src) {
  if (!GetVarint64(src, &offset) || !GetVarint64(src, &size) || !GetVarint64(src, &order)) {
    return Status::Corruption("TitanBlobHandle");
  }
  return Status::OK();
}

bool operator==(const TitanBlobHandle& lhs, const TitanBlobHandle& rhs) {
  return lhs.offset == rhs.offset && lhs.size == rhs.size &&
         lhs.order == rhs.order;
}

void TitanBlobIndex::EncodeTo(std::string* dst) const {
  dst->push_back(kTitanBlobRecord);
  PutVarint64(dst, file_number);
  blob_handle.EncodeTo(dst);
}

Status TitanBlobIndex::DecodeFrom(Slice* src) {
  unsigned char type;
  if (!TitanGetChar(src, &type) || type != kTitanBlobRecord ||
      !GetVarint64(src, &file_number)) {
    return Status::Corruption("TitanBlobIndex");
  }
  Status s = blob_handle.DecodeFrom(src);
  if (!s.ok()) {
    return Status::Corruption("TitanBlobIndex", s.ToString());
  }
  return s;
}

bool operator==(const TitanBlobIndex& lhs, const TitanBlobIndex& rhs) {
  return (lhs.file_number == rhs.file_number &&
          lhs.blob_handle == rhs.blob_handle);
}

}