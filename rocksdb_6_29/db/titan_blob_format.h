#include "rocksdb/options.h"
#include "rocksdb/slice.h"
#include "rocksdb/status.h"
#include "table/format.h"
#include "util/crc32c.h"

#include "util.h"
namespace ROCKSDB_NAMESPACE {

class TitanOwnedSlice : public Slice {
 public:
  void reset(CacheAllocationPtr _data, size_t _size) {
    data_ = _data.get();
    size_ = _size;
    buffer_ = std::move(_data);
  }

  void reset(CacheAllocationPtr buffer, const Slice& s) {
    data_ = s.data();
    size_ = s.size();
    buffer_ = std::move(buffer);
  }

  char* release() {
    data_ = nullptr;
    size_ = 0;
    return buffer_.release();
  }

  static void CleanupFunc(void* buffer, void*) {
    delete[] reinterpret_cast<char*>(buffer);
  }

 private:
  CacheAllocationPtr buffer_;
};

// Compresses the input data according to the compression context.
// Returns a slice with the output data and sets "*type" to the output
// compression type.
//
// If compression is actually performed, fills "*output" with the
// compressed data. However, if the compression ratio is not good, it
// returns the input slice directly and sets "*type" to
// kNoCompression.
Slice TitanCompress(const CompressionInfo& info, const Slice& input,
               std::string* output, CompressionType* type);

// Uncompresses the input data according to the uncompression type.
// If successful, fills "*buffer" with the uncompressed data and
// points "*output" to it.
Status TitanUncompress(const UncompressionInfo& info, const Slice& input,
                  TitanOwnedSlice* output);

const uint64_t kTitanBlobMaxHeaderSize = 12;
const uint64_t kTitanRecordHeaderSize = 9;
const uint64_t kTitanBlobFooterSize = BlockHandle::kMaxEncodedLength + 8 + 4;

// Format of blob record (not fixed size):
//
//    +--------------------+----------------------+
//    |        key         |        value         |
//    +--------------------+----------------------+
//    | Varint64 + key_len | Varint64 + value_len |
//    +--------------------+----------------------+
//
struct TitanBlobRecord {
  Slice key;
  Slice value;

  void EncodeTo(std::string* dst) const;
  Status DecodeFrom(Slice* src);

  size_t size() const { return key.size() + value.size(); }

  friend bool operator==(const TitanBlobRecord& lhs, const TitanBlobRecord& rhs);
};

class TitanBlobEncoder {
 public:
  TitanBlobEncoder(CompressionType compression, CompressionOptions compression_opt,
              const CompressionDict* compression_dict)
      : compression_opt_(compression_opt),
        compression_ctx_(compression),
        compression_dict_(compression_dict),
        compression_info_(new CompressionInfo(
            compression_opt_, compression_ctx_, *compression_dict_, compression,
            0 /*sample_for_compression*/)) {}
  TitanBlobEncoder(CompressionType compression)
      : TitanBlobEncoder(compression, CompressionOptions(),
                    &CompressionDict::GetEmptyDict()) {}
  TitanBlobEncoder(CompressionType compression,
              const CompressionDict* compression_dict)
      : TitanBlobEncoder(compression, CompressionOptions(), compression_dict) {}
  TitanBlobEncoder(CompressionType compression, CompressionOptions compression_opt)
      : TitanBlobEncoder(compression, compression_opt,
                    &CompressionDict::GetEmptyDict()) {}

  void EncodeRecord(const TitanBlobRecord& record);
  void EncodeSlice(const Slice& record);
  void SetCompressionDict(const CompressionDict* compression_dict) {
    compression_dict_ = compression_dict;
    compression_info_.reset(new CompressionInfo(
        compression_opt_, compression_ctx_, *compression_dict_,
        compression_info_->type(), compression_info_->SampleForCompression()));
  }

  Slice GetHeader() const { return Slice(header_, sizeof(header_)); }
  Slice GetRecord() const { return record_; }

  size_t GetEncodedSize() const { return sizeof(header_) + record_.size(); }

 private:
  char header_[kTitanRecordHeaderSize];
  Slice record_;
  std::string record_buffer_;
  std::string compressed_buffer_;
  CompressionOptions compression_opt_;
  CompressionContext compression_ctx_;
  const CompressionDict* compression_dict_;
  std::unique_ptr<CompressionInfo> compression_info_;
};

class TitanBlobDecoder {
 public:
  TitanBlobDecoder(const UncompressionDict* uncompression_dict,
              CompressionType compression = kNoCompression)
      : compression_(compression), uncompression_dict_(uncompression_dict) {}

  TitanBlobDecoder()
      : TitanBlobDecoder(&UncompressionDict::GetEmptyDict(), kNoCompression) {}

  Status DecodeHeader(Slice* src);
  Status DecodeRecord(Slice* src, TitanBlobRecord* record, TitanOwnedSlice* buffer);

  void SetUncompressionDict(const UncompressionDict* uncompression_dict) {
    uncompression_dict_ = uncompression_dict;
  }

  size_t GetRecordSize() const { return record_size_; }

 private:
  uint32_t crc_{0};
  uint32_t header_crc_{0};
  uint32_t record_size_{0};
  CompressionType compression_{kNoCompression};
  const UncompressionDict* uncompression_dict_;
};

// Format of blob handle (not fixed size):
//
//    +----------+----------+----------+
//    |  offset  |   size   |   order  |
//    +----------+----------+----------+
//    | Varint64 | Varint64 | Varint64 |
//    +----------+----------+----------+
//
struct TitanBlobHandle {
  uint64_t offset{0};
  uint64_t size{0};
  uint64_t order{0};  //start from 0

  void EncodeTo(std::string* dst) const;
  Status DecodeFrom(Slice* src);

  friend bool operator==(const TitanBlobHandle& lhs, const TitanBlobHandle& rhs);
};

// Format of blob index (not fixed size):
//
//    +------+-------------+-----------------------------------------------------+
//    | type | file number |                    blob handle                      |
//    +------+-------------+-----------------------------------------------------+
//    | char |  Varint64   | Varint64(offsest) + Varint64(size) + Varint64(order)|
//    +------+-------------+-----------------------------------------------------+
//
// It is stored in LSM-Tree as the value of key, then Titan can use this blob
// index to locate actual value from blob file.
struct TitanBlobIndex {
  enum Type : unsigned char {
    kTitanBlobRecord = 1,
  };
  uint64_t file_number{0};
  TitanBlobHandle blob_handle;

  void EncodeTo(std::string* dst) const;
  Status DecodeFrom(Slice* src);

  friend bool operator==(const TitanBlobIndex& lhs, const TitanBlobIndex& rhs);
};

template <typename T>
Status TitanDecodeInto(const Slice& src, T* target,
                  bool ignore_extra_bytes = false) {
  Slice tmp = src;
  Status s = target->DecodeFrom(&tmp);
  if (!ignore_extra_bytes && s.ok() && !tmp.empty()) {
    s = Status::Corruption("redundant bytes when decoding blob file");
  }
  return s;
}

}