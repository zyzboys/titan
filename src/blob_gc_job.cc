#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include "blob_gc_job.h"

#include <cinttypes>

#include <memory>

#include "titan_logging.h"

namespace rocksdb {
namespace titandb {

// Write callback for garbage collection to check if key has been updated
// since last read. Similar to how OptimisticTransaction works.
class BlobGCJob::GarbageCollectionWriteCallback : public WriteCallback {
 public:
  GarbageCollectionWriteCallback(ColumnFamilyHandle* cfh, std::string&& _key,
                                 BlobIndex blob_index, BlobIndex new_blob_index)
      : cfh_(cfh),
        key_(std::move(_key)),
        blob_index_(blob_index),
        new_blob_index_(new_blob_index),
        read_bytes_(0) {
    assert(!key_.empty());
  }
  // peiqi: gc callback
  virtual Status Callback(DB* db) override {
    auto* db_impl = reinterpret_cast<DBImpl*>(db);
    PinnableSlice index_entry;
    bool is_blob_index;
    DBImpl::GetImplOptions gopts;
    gopts.column_family = cfh_;
    gopts.value = &index_entry;
    gopts.is_blob_index = &is_blob_index;
    auto s = db_impl->GetImpl(ReadOptions(), key_, gopts);
    if (!s.ok() && !s.IsNotFound()) {
      return s;
    }
    read_bytes_ = key_.size() + index_entry.size();
    if (s.IsNotFound()) {
      // Either the key is deleted or updated with a newer version which is
      // inlined in LSM.
      s = Status::Busy("key deleted");
    } else if (!is_blob_index) {
      s = Status::Busy("key overwritten with other value");
    }

    if (s.ok()) {
      BlobIndex other_blob_index;
      s = other_blob_index.DecodeFrom(&index_entry);
      if (!s.ok()) {
        return s;
      }

      if (!(blob_index_ == other_blob_index)) {
        s = Status::Busy("key overwritten with other blob");
      }
    }

    return s;
  }

  virtual bool AllowWriteBatching() override { return false; }

  std::string key() { return key_; }

  uint64_t read_bytes() { return read_bytes_; }

  uint64_t blob_record_size() { return blob_index_.blob_handle.size; }

  const BlobIndex& new_blob_index() { return new_blob_index_; }

 private:
  ColumnFamilyHandle* cfh_;
  // Key to check
  std::string key_;
  BlobIndex blob_index_;
  // Empty means the new record is inlined.
  BlobIndex new_blob_index_;
  uint64_t read_bytes_;
};

BlobGCJob::BlobGCJob(BlobGC* blob_gc, DB* db, port::Mutex* mutex,
                     const TitanDBOptions& titan_db_options, Env* env,
                     const EnvOptions& env_options,
                     BlobFileManager* blob_file_manager,
                     BlobFileSet* blob_file_set, LogBuffer* log_buffer,
                     std::atomic_bool* shuting_down, TitanStats* stats, std::string& dbname, 
                     std::string& db_id, std::string& db_session_id, ShadowSet* shadow_set)
    : blob_gc_(blob_gc),
      base_db_(db),
      base_db_impl_(reinterpret_cast<DBImpl*>(base_db_)),
      mutex_(mutex),
      db_options_(titan_db_options),
      env_(env),
      env_options_(env_options),
      blob_file_manager_(blob_file_manager),
      blob_file_set_(blob_file_set),
      log_buffer_(log_buffer),
      shuting_down_(shuting_down),
      stats_(stats),
      dbname_(dbname),
      db_id_(db_id),
      db_session_id_(db_session_id),
      shadow_set_(shadow_set) {}

BlobGCJob::BlobGCJob(BlobGC* blob_gc, DB* db, port::Mutex* mutex,
                     const TitanDBOptions& titan_db_options, Env* env,
                     const EnvOptions& env_options,
                     BlobFileManager* blob_file_manager,
                     BlobFileSet* blob_file_set, LogBuffer* log_buffer,
                     std::atomic_bool* shuting_down, TitanStats* stats)
    : blob_gc_(blob_gc),
      base_db_(db),
      base_db_impl_(reinterpret_cast<DBImpl*>(base_db_)),
      mutex_(mutex),
      db_options_(titan_db_options),
      env_(env),
      env_options_(env_options),
      blob_file_manager_(blob_file_manager),
      blob_file_set_(blob_file_set),
      log_buffer_(log_buffer),
      shuting_down_(shuting_down),
      stats_(stats) {}

BlobGCJob::~BlobGCJob() {
  if (log_buffer_) {
    log_buffer_->FlushBufferToLog();
    LogFlush(db_options_.info_log.get());
  }
  // flush metrics
  RecordTick(statistics(stats_), TITAN_GC_BYTES_READ_CHECK, metrics_.gc_bytes_read_check);
  RecordTick(statistics(stats_), TITAN_GC_BYTES_READ_BLOB, metrics_.gc_bytes_read_blob);
  RecordTick(statistics(stats_), TITAN_GC_BYTES_READ_CALLBACK, metrics_.gc_bytes_read_callback);
  RecordTick(statistics(stats_), TITAN_GC_BYTES_WRITTEN_LSM,
             metrics_.gc_bytes_written_lsm);
  RecordTick(statistics(stats_), TITAN_GC_BYTES_WRITTEN_BLOB,
             metrics_.gc_bytes_written_blob);
  RecordTick(statistics(stats_), TITAN_GC_NUM_KEYS_TOTAL,
             metrics_.gc_num_keys_total);
  RecordTick(statistics(stats_), TITAN_GC_NUM_KEYS_OVERWRITTEN_BITSET,
             metrics_.gc_num_keys_overwritten_bitset);
  RecordTick(statistics(stats_), TITAN_GC_NUM_KEYS_OVERWRITTEN_CHECK,
             metrics_.gc_num_keys_overwritten_check);
  RecordTick(statistics(stats_), TITAN_GC_NUM_KEYS_OVERWRITTEN_CALLBACK,
             metrics_.gc_num_keys_overwritten_callback);
  RecordTick(statistics(stats_), TITAN_GC_BYTES_OVERWRITTEN_CHECK,
             metrics_.gc_bytes_overwritten_check);
  RecordTick(statistics(stats_), TITAN_GC_BYTES_OVERWRITTEN_CALLBACK,
             metrics_.gc_bytes_overwritten_callback);
  RecordTick(statistics(stats_), TITAN_GC_NUM_KEYS_RELOCATED,
             metrics_.gc_num_keys_relocated);
  RecordTick(statistics(stats_), TITAN_GC_BYTES_RELOCATED,
             metrics_.gc_bytes_relocated);
  RecordTick(statistics(stats_), TITAN_GC_NUM_KEYS_FALLBACK,
             metrics_.gc_num_keys_fallback);
  RecordTick(statistics(stats_), TITAN_GC_BYTES_FALLBACK,
             metrics_.gc_bytes_fallback);
  RecordTick(statistics(stats_), TITAN_GC_NUM_NEW_FILES,
             metrics_.gc_num_new_files);
  RecordTick(statistics(stats_), TITAN_GC_NUM_FILES, metrics_.gc_num_files);
}

Status BlobGCJob::Prepare() {
  SavePrevIOBytes(&prev_bytes_read_, &prev_bytes_written_);
  return Status::OK();
}

Status BlobGCJob::Run() {
  std::string tmp;
  uint64_t total_size = 0;
  uint64_t total_live_data_size = 0;
  for (const auto& f : blob_gc_->inputs()) {
    if (!tmp.empty()) {
      tmp.append(" ");
    }
    tmp.append(std::to_string(f->file_number()));
    total_size += f->file_size();
    total_live_data_size += f->live_data_size();
  }
  TITAN_LOG_INFO(db_options_.info_log,
                 "[%s] Titan GC job start with %" PRIu64
                 " files, %" PRIu64 " bytes, %" PRIu64 " live bytes, %" PRIu64 " garbage bytes",
                 blob_gc_->column_family_handle()->GetName().c_str(),
                 blob_gc_->inputs().size(), total_size, total_live_data_size, total_size - total_live_data_size);
  TITAN_LOG_BUFFER(log_buffer_, "[%s] Titan GC candidates[%s]",
                   blob_gc_->column_family_handle()->GetName().c_str(),
                   tmp.c_str());
  return DoRunGC();
}

Status BlobGCJob::DoRunGC() {
  Status s;

  std::unique_ptr<BlobFileMergeIterator> gc_iter;
  s = BuildIterator(&gc_iter);
  if (!s.ok()) return s;
  if (!gc_iter) return Status::Aborted("Build iterator for gc failed");

  // Similar to OptimisticTransaction, we obtain latest_seq from
  // base DB, which is guaranteed to be no smaller than the sequence of
  // current key. We use a WriteCallback on write to check the key sequence
  // on write. If the key sequence is larger than latest_seq, we know
  // a new versions is inserted and the old blob can be discard.
  //
  // We cannot use OptimisticTransaction because we need to pass
  // is_blob_index flag to GetImpl.
  std::unique_ptr<BlobFileHandle> blob_file_handle;
  std::unique_ptr<BlobFileBuilder> blob_file_builder;

  ColumnFamilyData *cfd = blob_gc_->GetColumnFamilyData();
  std::vector<std::unique_ptr<ShadowHandle>> shadow_handles(7);
  // for (int i = 0; i < 7; i++) {
  //   shadow_handles[i].reset();
  // }

  // std::vector<std::unique_ptr<TableBuilder>> level_shadow_builders;
  // std::vector<std::unique_ptr<WritableFileWriter>> level_shadow_files;
  // // preallocate 7(max) levels for shadow builders
  // level_shadow_builders.resize(7);
  // level_shadow_files.resize(7);

  //  uint64_t drop_entry_num = 0;
  //  uint64_t drop_entry_size = 0;
  //  uint64_t total_entry_num = 0;
  //  uint64_t total_entry_size = 0;

  uint64_t file_size = 0;
  uint64_t shadow_size = 0;
  uint64_t discardable_count = 0;
  uint64_t total_count = 0;
  uint64_t valid_count = 0;

  std::string last_key;
  bool last_key_is_fresh = false;
  gc_iter->SeekToFirst();
  assert(gc_iter->Valid());
  for (; gc_iter->Valid(); gc_iter->Next()) {
    metrics_.gc_num_keys_total++;
    total_count++;
    if (IsShutingDown()) {
      s = Status::ShutdownInProgress();
      break;
    }
    BlobIndex blob_index = gc_iter->GetBlobIndex();
    // count read bytes for blob record of gc candidate files
    metrics_.gc_bytes_read_blob += blob_index.blob_handle.size;

    if (!last_key.empty() && (gc_iter->key().compare(last_key) == 0)) {
      if (last_key_is_fresh) {
        // We only need to rewrite the newest version. Blob files containing
        // the older versions will not be purged if there's a snapshot
        // referencing them.
        continue;
      }
    } else {
      last_key = gc_iter->key().ToString();
      last_key_is_fresh = false;
    }

    bool discardable = false;
    bool in_shadow = false;
    int level = 0;
    SequenceNumber seq = 0;
    {
      StopWatch gc_sw(env_->GetSystemClock().get(), statistics(stats_),
                    TITAN_GC_CHECK_MICROS);
      //先查看是否在shadow_cache中
      // s = DiscardEntryWithShadow(gc_iter->key(), blob_index, &discardable, &in_shadow);
      // if (!s.ok()) {
      //   break;
      // }       

      //未开启shadow_cache或者shadow_cache中没有该key，正常检查
      if (blob_gc_->titan_cf_options().drop_key_bitset) {
        s = DiscardEntryWithBitset(blob_index, &discardable);
        if (!s.ok()) {
          break;
        }
        if (!discardable) {
          // maybe valid, check again in LSM and get the level of valid key
          s = DiscardEntry(gc_iter->key(), blob_index, &discardable, &level, &seq);
          if (!s.ok()) {
            break;
          }
        } else {
          metrics_.gc_num_keys_overwritten_bitset++;
        }
      } else {
        s = DiscardEntry(gc_iter->key(), blob_index, &discardable, &level, &seq);
        if (!s.ok()) {
          break;
        }
      }
    }
    
    if (discardable) {
      // if (level == 0) {
      //   std::cout << "L0 discardable" << std::endl;
      // }
      metrics_.gc_num_keys_overwritten_check++;
      metrics_.gc_bytes_overwritten_check += blob_index.blob_handle.size;
      discardable_count++;
      continue;
    }
    // assert(level != -1);
    valid_count++;
    last_key_is_fresh = true;

    if (blob_gc_->titan_cf_options().blob_run_mode ==
        TitanBlobRunMode::kFallback) {
      auto* cfh = blob_gc_->column_family_handle();
      GarbageCollectionWriteCallback callback(cfh, gc_iter->key().ToString(),
                                              blob_index, BlobIndex());
      rewrite_batches_.emplace_back(
          std::make_pair(WriteBatch(), std::move(callback)));
      auto& wb = rewrite_batches_.back().first;
      s = WriteBatchInternal::Put(&wb, cfh->GetID(), gc_iter->key(),
                                  gc_iter->value());
      if (!s.ok()) {
        break;
      } else {
        continue;
      }
    }

    // Rewrite entry to new blob file
    if ((!blob_file_handle && !blob_file_builder) ||
        file_size >= blob_gc_->titan_cf_options().blob_file_target_size) {
      if (file_size >= blob_gc_->titan_cf_options().blob_file_target_size) {
        assert(blob_file_builder);
        assert(blob_file_handle);
        assert(blob_file_builder->status().ok());
        blob_file_builders_.emplace_back(std::make_pair(
            std::move(blob_file_handle), std::move(blob_file_builder)));
      }
      s = blob_file_manager_->NewFile(&blob_file_handle,
                                      Env::IOPriority::IO_LOW);
      if (!s.ok()) {
        break;
      }
      TITAN_LOG_INFO(db_options_.info_log,
                     "Titan new GC output file %" PRIu64 ".",
                     blob_file_handle->GetNumber());
      blob_file_builder = std::unique_ptr<BlobFileBuilder>(
          new BlobFileBuilder(db_options_, blob_gc_->titan_cf_options(),
                              blob_file_handle->GetFile()));
      file_size = 0;
    }
    assert(blob_file_handle);
    assert(blob_file_builder);

    BlobRecord blob_record;
    blob_record.key = gc_iter->key();
    blob_record.value = gc_iter->value();
    // count written bytes for new blob record,
    // blob index's size is counted in `RewriteValidKeyToLSM`
    metrics_.gc_bytes_written_blob += blob_record.size();

    // BlobRecordContext require key to be an internal key. We encode key to
    // internal key in spite we only need the user key.
    std::unique_ptr<BlobFileBuilder::BlobRecordContext> ctx(
        new BlobFileBuilder::BlobRecordContext);
    InternalKey ikey(blob_record.key, 1, kTypeValue);
    ctx->key = ikey.Encode().ToString();
    ctx->original_blob_index = blob_index;
    ctx->new_blob_index.file_number = blob_file_handle->GetNumber();

    BlobFileBuilder::OutContexts contexts;
    blob_file_builder->Add(blob_record, std::move(ctx), &contexts);

    if (blob_gc_->titan_cf_options().shadow_cache) {
      // Add to shadow cache
      AddToShadowCache(contexts);
    }

    if (blob_gc_->titan_cf_options().rewrite_shadow) {
      if (shadow_handles[level] == nullptr) {
        shadow_handles[level].reset(new ShadowHandle());
        s = OpenGCOutputShadow(shadow_handles[level].get(), level);
        if (!s.ok()) {
          break;
        }
        shadow_size = 0;
      }
      assert(shadow_handles[level]->shadow_builder);
      assert(shadow_handles[level]->shadow_file);
      s = AddToShadow(shadow_handles[level].get(), contexts, seq);
      if (!s.ok()) {
        break;
      }
      shadow_size = shadow_handles[level]->shadow_builder->EstimatedFileSize();
      if (shadow_size >= blob_gc_->titan_cf_options().shadow_target_size) {
        s = FinishGCOutputShadow(shadow_handles[level].get(), level);
        shadow_handles[level].reset();
        if (!s.ok()) {
          break;
        }
      }

    }

    if (!blob_gc_->titan_cf_options().rewrite_shadow && !blob_gc_->titan_cf_options().shadow_cache) {
      BatchWriteNewIndices(contexts, &s);
      if (!s.ok()) {
        break;
      }
    }

  }

  TITAN_LOG_INFO(db_options_.info_log, "Titan GC total key count: %" PRIu64
                                       " valid key count: %" PRIu64
                                       " discardable key count: %" PRIu64,
                 total_count, valid_count, discardable_count);
  
  // for (auto redirect_info : redirect_entries_map_diff_) {
  //   FileMetaData* meta = vstorage->GetFileMetaDataByNumber(redirect_info.first);
  //   if (meta) {
  //     meta->num_redirect_entries = redirect_info.second;
  //     std::cout << "sst file number: " << redirect_info.first <<" total entries: "<< meta->num_entries<< " redirect entries: " << meta->num_redirect_entries << std::endl;
  //   }
  // }

  if (gc_iter->status().ok() && s.ok()) {
    if (blob_file_builder && blob_file_handle) {
      assert(blob_file_builder->status().ok());
      blob_file_builders_.emplace_back(std::make_pair(
          std::move(blob_file_handle), std::move(blob_file_builder)));
    } else {
      assert(!blob_file_builder);
      assert(!blob_file_handle);
    }
  } else if (!gc_iter->status().ok()) {
    return gc_iter->status();
  }

  if (blob_gc_->titan_cf_options().rewrite_shadow) {
    int level = 0;
    for (auto& handle : shadow_handles) {
      if (handle) {
        s = FinishGCOutputShadow(handle.get(), level);
        handle.reset();
        if (!s.ok()) {
          break;
        }
      }
      level++;
    }
  }    

  return s;
}

Status BlobGCJob::OpenGCOutputShadow(ShadowHandle *handle, int level) {
  Status s;
  uint64_t shadow_number = base_db_impl_->GetVersionSet()->NewFileNumber();
  std::string shadow_name = ShadowFileName(db_options_.dirname, shadow_number) + "_" + std::to_string(level);
  ColumnFamilyData *cfd = blob_gc_->GetColumnFamilyData();
  
  std::unique_ptr<FSWritableFile> writable_file;
  
  FileOptions fo_copy = FileOptions(env_options_);
  Temperature temperature = Temperature::kUnknown;
  fo_copy.temperature = temperature;

  s = env_->GetFileSystem()->NewWritableFile(
      shadow_name, FileOptions(env_options_), &writable_file, nullptr /*dbg*/);
  if (!s.ok()) return s;


  auto& ioptions = *blob_gc_->GetColumnFamilyData()->ioptions();
  FileTypeSet tmp_set = ioptions.checksum_handoff_file_types;
  writable_file->SetIOPriority(Env::IOPriority::IO_LOW);
  writable_file->SetWriteLifeTimeHint(Env::WriteLifeTimeHint::WLTH_NOT_SET);

  handle->shadow_file.reset(new WritableFileWriter(std::move(writable_file), shadow_name, FileOptions(env_options_), 
          ioptions.clock, io_tracer_, ioptions.stats, ioptions.listeners,
          ioptions.file_checksum_gen_factory.get(),
          tmp_set.Contains(FileType::kTableFile), false));

  TITAN_LOG_INFO(db_options_.info_log,
                 "Titan open new GC shadow %" PRIu64 ".", shadow_number);

  int64_t temp_current_time = 0;
  auto get_time_status = ioptions.clock->GetCurrentTime(&temp_current_time);
  // Safe to proceed even if GetCurrentTime fails. So, log and proceed.
  if (!get_time_status.ok()) {
    TITAN_LOG_WARN(db_options_.info_log,
                   "Failed to get current time. Status: %s",
                   get_time_status.ToString().c_str());
  }
  uint64_t current_time = static_cast<uint64_t>(temp_current_time);
  uint64_t oldest_ancester_time = current_time;
  {
    handle->shadow_meta.fd = FileDescriptor(shadow_number, 0/*0 also for shadow*/, 0);
    handle->shadow_meta.is_shadow = true;
    handle->shadow_meta.oldest_ancester_time = oldest_ancester_time;
    handle->shadow_meta.file_creation_time = current_time;
    handle->shadow_meta.temperature = temperature;
  }

  TableBuilderOptions tboptions(
      *cfd->ioptions(), *(cfd->GetLatestMutableCFOptions()),
      cfd->internal_comparator(), cfd->int_tbl_prop_collector_factories(),
      blob_gc_->titan_cf_options().blob_file_compression, blob_gc_->titan_cf_options().blob_file_compression_options, cfd->GetID(),
      cfd->GetName(), level, false, TableFileCreationReason::kShadow,
      oldest_ancester_time, 0, current_time, db_id_, db_session_id_, blob_gc_->titan_cf_options().shadow_target_size, shadow_number);
  
  handle->shadow_builder.reset(NewTableBuilder(tboptions, handle->shadow_file.get()));

  return s;
}

Status BlobGCJob::AddToShadow(ShadowHandle *handle, BlobFileBuilder::OutContexts& contexts, SequenceNumber seq) {
  assert(handle->shadow_builder);
  Status s;
  for (const std::unique_ptr<BlobFileBuilder::BlobRecordContext>& ctx :
       contexts) {
    BlobIndex blob_index;
    blob_index.file_number = ctx->new_blob_index.file_number;
    blob_index.blob_handle = ctx->new_blob_index.blob_handle;

    std::string index_entry;
    blob_index.EncodeTo(&index_entry);
    ParsedInternalKey ikey;
    s = ParseInternalKey(ctx->key, &ikey, false /*log_err_key*/);
    if (!s.ok()) {
      return s;
    }
    InternalKey shadow_ikey(ikey.user_key, seq, kTypeBlobIndex);
    handle->shadow_builder->Add(Slice(shadow_ikey.Encode().ToString()), Slice(index_entry)); 
    handle->shadow_meta.UpdateBoundaries(Slice(shadow_ikey.Encode().ToString()), Slice(index_entry), ikey.sequence, ikey.type);
    if (!s.ok()) break;
  }
  return s;
}

Status BlobGCJob::FinishGCOutputShadow(ShadowHandle *handle, int level) {
  assert(handle);
  Status s;
  s = handle->shadow_builder->Finish();
  if (!s.ok()) {
    return s;
  }
  IOStatus io_s = handle->shadow_builder->io_status();
  s = io_s;
  
  if (s.ok()) {
    handle->shadow_meta.InitLiveDataBitset(handle->shadow_builder->NumEntries());
    handle->shadow_meta.fd.file_size = handle->shadow_builder->FileSize();
  }
  
  if (s.ok()) {
    io_s = handle->shadow_file->Sync(db_options_.use_fsync);
  }

  if (s.ok() && io_s.ok()) {
    io_s = handle->shadow_file->Close();
  }

  if (s.ok() && io_s.ok()) {
    handle->shadow_meta.file_checksum = handle->shadow_file->GetFileChecksum();
    handle->shadow_meta.file_checksum_func_name = handle->shadow_file->GetFileChecksumFuncName();
  }

  TITAN_LOG_INFO(db_options_.info_log,
                     "Titan adding shadow file [%" PRIu64 "] range [%s, %s]",
                     handle->shadow_meta.fd.GetNumber(),
                     handle->shadow_meta.smallest.user_key().ToString(true).c_str(),
                     handle->shadow_meta.largest.user_key().ToString(true).c_str());

  shadow_metas_.emplace_back(std::make_pair(level, handle->shadow_meta));
  
  handle->shadow_builder.reset();
  handle->shadow_file.reset();
  
  return s;

}

Status BlobGCJob::AddToShadowCache(BlobFileBuilder::OutContexts& contexts) {
  Status s;
  for (const std::unique_ptr<BlobFileBuilder::BlobRecordContext>& ctx :
       contexts) {
    BlobIndex blob_index;
    blob_index.file_number = ctx->new_blob_index.file_number;
    blob_index.blob_handle = ctx->new_blob_index.blob_handle;
    uint64_t father_number = ctx->original_blob_index.file_number;

    std::string index_entry;
    blob_index.EncodeTo(&index_entry);
    ParsedInternalKey ikey;
    s = ParseInternalKey(ctx->key, &ikey, false /*log_err_key*/);
    if (!s.ok()) {
      return s;
    }
    cache_addition_[ikey.user_key.ToString()] = std::make_pair(father_number, index_entry);
  }
  return s;

}

void BlobGCJob::BatchWriteNewIndices(BlobFileBuilder::OutContexts& contexts,
                                     Status* s) {
  auto* cfh = blob_gc_->column_family_handle();
  for (const std::unique_ptr<BlobFileBuilder::BlobRecordContext>& ctx :
       contexts) {
    BlobIndex blob_index;
    blob_index.file_number = ctx->new_blob_index.file_number;
    blob_index.blob_handle = ctx->new_blob_index.blob_handle;

    std::string index_entry;
    BlobIndex original_index = ctx->original_blob_index;
    ParsedInternalKey ikey;
    *s = ParseInternalKey(ctx->key, &ikey, false /*log_err_key*/);
    if (!s->ok()) {
      return;
    }
    blob_index.EncodeTo(&index_entry);
    // Store WriteBatch for rewriting new Key-Index pairs to LSM
    GarbageCollectionWriteCallback callback(cfh, ikey.user_key.ToString(),
                                            original_index, blob_index);
    rewrite_batches_.emplace_back(
        std::make_pair(WriteBatch(), std::move(callback)));
    auto& wb = rewrite_batches_.back().first;
    *s = WriteBatchInternal::PutBlobIndex(&wb, cfh->GetID(), ikey.user_key,
                                          index_entry);
    if (!s->ok()) break;
  }
}

Status BlobGCJob::BuildIterator(
    std::unique_ptr<BlobFileMergeIterator>* result) {
  Status s;
  const auto& inputs = blob_gc_->inputs();
  assert(!inputs.empty());
  std::vector<std::unique_ptr<BlobFileIterator>> list;
  for (std::size_t i = 0; i < inputs.size(); ++i) {
    std::unique_ptr<RandomAccessFileReader> file;
    // TODO(@DorianZheng) set read ahead size
    s = NewBlobFileReader(inputs[i]->file_number(), 0, db_options_,
                          env_options_, env_, &file);
    if (!s.ok()) {
      break;
    }
    list.emplace_back(std::unique_ptr<BlobFileIterator>(new BlobFileIterator(
        std::move(file), inputs[i]->file_number(), inputs[i]->file_size(),
        blob_gc_->titan_cf_options())));
  }

  if (s.ok())
    result->reset(new BlobFileMergeIterator(
        std::move(list), blob_gc_->titan_cf_options().comparator));

  return s;
}

Status BlobGCJob::DiscardEntryWithShadow(const Slice& key, const BlobIndex& blob_index, bool *discardable, bool *in_shadow) {
  std::string cache_value;
  if (shadow_set_->GetShadowCache()->KeyExist(key.ToString(), cache_value)) {
    *in_shadow = true;
    BlobIndex shadow_index;
    Slice copy = cache_value;
    shadow_index.DecodeFrom(&copy);
    if (blob_index == shadow_index) {
      //shadow里有，但是一样，说明还没来得及合并，不丢弃
      //fprintf(stderr, "in shadow, same, remain\n");
      *discardable = false;
    } else {
      //shadow里有，但是不一样，丢弃
      //fprintf(stderr, "in shadow, different, discard\n");
      *discardable = true;
    }
  }
  //不在shadow里，可能已经合并了或者是LSM本来的key，需要进一步检查
  return Status::OK();
}


Status BlobGCJob::DiscardEntryWithBitset(const BlobIndex &blob_index, bool *discardable) {
  assert(discardable != nullptr);
  StopWatch gc_sw(env_->GetSystemClock().get(), statistics(stats_),
                    TITAN_GC_CHECK_BITSET_MICROS);
  std::shared_ptr<BlobFileMeta> file;
  // find blob file meta
  for (const auto& f : blob_gc_->inputs()) {
    if (f->file_number() == blob_index.file_number) {
      file = f;
      break;
    }
  }
  // can't find blob file meta
  if (!file) {
    return Status::NotFound("Blob file meta not found");
  }
  // check bitset
  if (file->IsLiveData(blob_index.blob_handle.order)) {
    *discardable = false;
  } else {
    *discardable = true;
  }

  return Status::OK();
}

Status BlobGCJob::DiscardEntry(const Slice& key, const BlobIndex& blob_index,
                               bool* discardable, int* level, SequenceNumber* seq) {
  StopWatch gc_sw(env_->GetSystemClock().get(), statistics(stats_),
                    TITAN_GC_CHECK_LSM_MICROS);
  TitanStopWatch sw(env_, metrics_.gc_read_lsm_micros);
  assert(discardable != nullptr);
  PinnableSlice index_entry;
  bool is_blob_index = false;
  DBImpl::GetImplOptions gopts;
  gopts.column_family = blob_gc_->column_family_handle();
  gopts.value = &index_entry;
  gopts.is_blob_index = &is_blob_index;
  // get the level of the key
  gopts.return_level = true;
  gopts.set_redirect = true;
  Status s = base_db_impl_->GetImpl(ReadOptions(), key, gopts);
  *level = gopts.level;
  *seq = gopts.seq;
  // if (*level == 0) {
  //   std::cout << "level 0" << std::endl;
  // }
  if (!s.ok() && !s.IsNotFound()) {
    return s;
  }
  // count read bytes for checking LSM entry
  metrics_.gc_bytes_read_check += key.size() + index_entry.size();
  if (s.IsNotFound() || !is_blob_index) {
    // Either the key is deleted or updated with a newer version which is
    // inlined in LSM.
    *discardable = true;
    return Status::OK();
  }

  BlobIndex other_blob_index;
  s = other_blob_index.DecodeFrom(&index_entry);
  if (!s.ok()) {
    return s;
  }

  //*discardable = !(blob_index == other_blob_index);
  if (other_blob_index.file_number > blob_index.file_number) {
    //fprintf(stderr, "wrong discardable, get filenum:%ld, gc filenum:%ld\n", other_blob_index.file_number, blob_index.file_number);
    //当lsm里面的filenumber比gc的大时才丢弃，如果lsm的比gc的小，说明还没来得及合入lsm，可能在shadow里
    *discardable = true;
  } else {
    *discardable = false;
    //std::cout << "file number: " << gopts.redirect_file_number << std::endl;
    //std::cout << "writeback to shadow cache, redirect entries++" << std::endl;
    redirect_entries_map_diff_[gopts.redirect_file_number]++;
  }
  return Status::OK();
}

// We have to make sure crash consistency, but LSM db MANIFEST and BLOB db
// MANIFEST are separate, so we need to make sure all new blob file have
// added to db before we rewrite any key to LSM
Status BlobGCJob::Finish() {
  Status s;
  {
    mutex_->Unlock();
    s = InstallOutputBlobFiles();
    // rewrite_shadow is false, rewrite to LSM
    if (s.ok()) {
      TEST_SYNC_POINT("BlobGCJob::Finish::BeforeRewriteValidKeyToLSM");
      if (blob_gc_->titan_cf_options().rewrite_shadow) {
        s = InstallOutputShadows();
        if (!s.ok()) {
          TITAN_LOG_ERROR(db_options_.info_log,
                          "[%s] GC job failed to install output shadows: %s",
                          blob_gc_->column_family_handle()->GetName().c_str(),
                          s.ToString().c_str());
        }
      }
      if (blob_gc_->titan_cf_options().shadow_cache) {
        s = InstallOutputShadowCache();
        if (!s.ok()) {
          TITAN_LOG_ERROR(db_options_.info_log,
                          "[%s] GC job failed to install output shadow cache: %s",
                          blob_gc_->column_family_handle()->GetName().c_str(),
                          s.ToString().c_str());
        }
      }
      if (!blob_gc_->titan_cf_options().rewrite_shadow && !blob_gc_->titan_cf_options().shadow_cache) {
        s = RewriteValidKeyToLSM();
        if (!s.ok()) {
          TITAN_LOG_ERROR(db_options_.info_log,
                          "[%s] GC job failed to rewrite keys to LSM: %s",
                          blob_gc_->column_family_handle()->GetName().c_str(),
                          s.ToString().c_str());
        }
      }
      
    } else {
      TITAN_LOG_ERROR(db_options_.info_log,
                      "[%s] GC job failed to install output blob files: %s",
                      blob_gc_->column_family_handle()->GetName().c_str(),
                      s.ToString().c_str());
    }
    mutex_->Lock();
  }

  ColumnFamilyData *cfd = blob_gc_->GetColumnFamilyData();
  if (cfd->shadow_set() == nullptr) {
    cfd->SetShadowSet(shadow_set_);
    std::cout<<"set shadow_set in cf"<<std::endl;
  }
  if (cfd->shadow_set() == nullptr) {
    std::cout<<"ERROR: shadow_set is null"<<std::endl;
  }
  
  if (s.ok() && !blob_gc_->GetColumnFamilyData()->IsDropped()) {
    s = DeleteInputBlobFiles();
  }
  TEST_SYNC_POINT("BlobGCJob::Finish::AfterRewriteValidKeyToLSM");

  if (s.ok()) {
    UpdateInternalOpStats();
  }

  return s;
}

Status BlobGCJob::InstallOutputShadows() {
  //peiqi: how to update version?
  TITAN_LOG_INFO(db_options_.info_log, "in InstallOutputShadows()");
  shadow_set_->AddShadows(shadow_metas_);
  return Status::OK();
}

Status BlobGCJob::InstallOutputShadowCache() {
  Status s;
  TITAN_LOG_INFO(db_options_.info_log, "in InstallOutputShadowCache()");
  TITAN_LOG_INFO(db_options_.info_log, "add cache count: %ld\n", cache_addition_.size());
  shadow_set_->GetShadowCache()->AddMulti(cache_addition_, &drop_keys);
  TITAN_LOG_INFO(db_options_.info_log, "current cache entries count: %ld\n", shadow_set_->GetShadowCache()->GetCacheSize());
  shadow_set_->GetRedirectEntriesMap()->AddMulti(redirect_entries_map_diff_);
  //shadow_set_->GetRedirectEntriesMap()->PrintBrief();
  //std::cout <<"install shadow cache done" << std::endl;
  mutex_->Lock();
  auto cf_id = blob_gc_->column_family_handle()->GetID();
  for (auto blobfile : drop_keys) {
    auto blob_storage = blob_file_set_->GetBlobStorage(cf_id).lock();
    if (blob_storage) {
      auto file = blob_storage->FindFile(blobfile.first).lock();
      if (!file) {
        continue;
      }
      for (auto order : blobfile.second) {
        file->SetLiveDataBitset(order, false);
      }
    }

  }
  mutex_->Unlock();
  return s;
}

Status BlobGCJob::InstallOutputBlobFiles() {
  TITAN_LOG_INFO(db_options_.info_log, "in InstallOutputBlobFiles()");
  StopWatch gc_sw(env_->GetSystemClock().get(), statistics(stats_),
                    TITAN_GC_WRITE_MICROS);
  Status s;
  std::vector<
      std::pair<std::shared_ptr<BlobFileMeta>, std::unique_ptr<BlobFileHandle>>>
      files;
  std::string tmp;
  for (auto& builder : blob_file_builders_) {
    BlobFileBuilder::OutContexts contexts;
    s = builder.second->Finish(&contexts);
    BatchWriteNewIndices(contexts, &s);
    if (!s.ok()) {
      break;
    }
    metrics_.gc_num_new_files++;

    auto file = std::make_shared<BlobFileMeta>(
        builder.first->GetNumber(), builder.first->GetFile()->GetFileSize(), builder.second->NumEntries(),
        0, builder.second->GetSmallestKey(), builder.second->GetLargestKey());
    file->set_live_data_size(builder.second->live_data_size());
    file->InitLiveDataBitset(builder.second->NumEntries());
    file->FileStateTransit(BlobFileMeta::FileEvent::kGCOutput);
    RecordInHistogram(statistics(stats_), TITAN_GC_OUTPUT_FILE_SIZE,
                      file->file_size());
    if (!tmp.empty()) {
      tmp.append(" ");
    }
    tmp.append(std::to_string(file->file_number()));
    files.emplace_back(std::make_pair(file, std::move(builder.first)));
  }
  if (s.ok()) {
    TITAN_LOG_BUFFER(log_buffer_, "[%s] output[%s]",
                     blob_gc_->column_family_handle()->GetName().c_str(),
                     tmp.c_str());
    s = blob_file_manager_->BatchFinishFiles(
        blob_gc_->column_family_handle()->GetID(), files);
    if (s.ok()) {
      for (auto& file : files) {
        blob_gc_->AddOutputFile(file.first.get());
      }
    }
  } else {
    std::vector<std::unique_ptr<BlobFileHandle>> handles;
    std::string to_delete_files;
    for (auto& builder : blob_file_builders_) {
      if (!to_delete_files.empty()) {
        to_delete_files.append(" ");
      }
      to_delete_files.append(std::to_string(builder.first->GetNumber()));
      handles.emplace_back(std::move(builder.first));
    }
    TITAN_LOG_BUFFER(log_buffer_,
                     "[%s] InstallOutputBlobFiles failed. Delete GC output "
                     "files: %s",
                     blob_gc_->column_family_handle()->GetName().c_str(),
                     to_delete_files.c_str());
    // Do not set status `s` here, cause it may override the non-okay-status
    // of `s` so that in the outer funcation it will rewrite blob indexes to
    // LSM by mistake.
    Status status = blob_file_manager_->BatchDeleteFiles(handles);
    if (!status.ok()) {
      TITAN_LOG_WARN(db_options_.info_log,
                     "Delete GC output files[%s] failed: %s",
                     to_delete_files.c_str(), status.ToString().c_str());
    }
  }

  return s;
}

Status BlobGCJob::RewriteValidKeyToLSM() {
  TITAN_LOG_INFO(db_options_.info_log, "in RewriteValidKeyToLSM()");
  TitanStopWatch sw(env_, metrics_.gc_update_lsm_micros);
  StopWatch gc_sw(env_->GetSystemClock().get(), statistics(stats_),
                    TITAN_GC_WRITEBACK_MICROS);
  Status s;
  auto* db_impl = reinterpret_cast<DBImpl*>(base_db_);

  WriteOptions wo;
  wo.low_pri = true;
  wo.ignore_missing_column_families = true;

  std::unordered_map<uint64_t, std::pair<uint64_t, std::set<uint64_t>>>
      dropped;  // blob_file_number -> dropped_size
  for (auto& write_batch : rewrite_batches_) {
    if (blob_gc_->GetColumnFamilyData()->IsDropped()) {
      s = Status::Aborted("Column family drop");
      break;
    }
    if (IsShutingDown()) {
      s = Status::ShutdownInProgress();
      break;
    }
    s = db_impl->WriteWithCallback(wo, &write_batch.first, &write_batch.second);
    const auto& new_blob_index = write_batch.second.new_blob_index();
    if (s.ok()) {
      if (new_blob_index.blob_handle.size > 0) {
        // Rewritten as blob record.
        // count written bytes for new blob index.
        metrics_.gc_bytes_written_lsm += write_batch.first.GetDataSize();
        metrics_.gc_num_keys_relocated++;
        metrics_.gc_bytes_relocated += write_batch.second.blob_record_size();
      } else {
        // Rewritten as inline value due to fallback mode.
        metrics_.gc_num_keys_fallback++;
        metrics_.gc_bytes_fallback += write_batch.second.blob_record_size();
      }
    } else if (s.IsBusy()) {
      metrics_.gc_num_keys_overwritten_callback++;
      metrics_.gc_bytes_overwritten_callback += write_batch.second.blob_record_size();
      // The key is overwritten in the meanwhile. Drop the blob record.
      // Though record is dropped, the diff won't counted in discardable
      // ratio,
      // so we should update the live_data_size here.
      dropped[new_blob_index.file_number].first += new_blob_index.blob_handle.size;
      dropped[new_blob_index.file_number].second.insert(new_blob_index.blob_handle.order);
    } else {
      // We hit an error.
      break;
    }
    // count read bytes in write callback
    metrics_.gc_bytes_read_callback += write_batch.second.read_bytes();
  }
  if (s.IsBusy()) {
    s = Status::OK();
  }

  mutex_->Lock();
  auto cf_id = blob_gc_->column_family_handle()->GetID();
  for (auto blob_file : dropped) {
    auto blob_storage = blob_file_set_->GetBlobStorage(cf_id).lock();
    if (blob_storage) {
      auto file = blob_storage->FindFile(blob_file.first).lock();
      if (!file) {
        TITAN_LOG_ERROR(db_options_.info_log,
                        "Blob File %" PRIu64 " not found when GC.",
                        blob_file.first);
        continue;
      }
      for (auto order : blob_file.second.second) {
        file->SetLiveDataBitset(order, false);
      }
      SubStats(stats_, cf_id, file->GetDiscardableRatioLevel(), 1);
      file->UpdateLiveDataSize(-blob_file.second.first);
      AddStats(stats_, cf_id, file->GetDiscardableRatioLevel(), 1);

      blob_storage->ComputeGCScore();
    } else {
      TITAN_LOG_ERROR(db_options_.info_log,
                      "Column family id:%" PRIu32 " not Found when GC.", cf_id);
    }
  }
  mutex_->Unlock();

  if (s.ok()) {
    // Flush and sync WAL.
    s = db_impl->FlushWAL(true /*sync*/);
  }

  return s;
}

Status BlobGCJob::DeleteInputBlobFiles() {
  SequenceNumber obsolete_sequence = base_db_impl_->GetLatestSequenceNumber();

  Status s;
  VersionEdit edit;
  edit.SetColumnFamilyID(blob_gc_->column_family_handle()->GetID());
  for (const auto& file : blob_gc_->inputs()) {
    TITAN_LOG_INFO(db_options_.info_log,
                   "Titan add obsolete file [%" PRIu64 "] range [%s, %s]",
                   file->file_number(),
                   Slice(file->smallest_key()).ToString(true).c_str(),
                   Slice(file->largest_key()).ToString(true).c_str());
    metrics_.gc_num_files++;
    RecordInHistogram(statistics(stats_), TITAN_GC_INPUT_FILE_SIZE,
                      file->file_size());
    if (file->is_obsolete()) {
      // There may be a concurrent DeleteBlobFilesInRanges or GC,
      // so the input file is already deleted.
      continue;
    }
    edit.DeleteBlobFile(file->file_number(), obsolete_sequence);
  }
  s = blob_file_set_->LogAndApply(edit);
  return s;
}

bool BlobGCJob::IsShutingDown() {
  return (shuting_down_ && shuting_down_->load(std::memory_order_acquire));
}

void BlobGCJob::UpdateInternalOpStats() {
  if (stats_ == nullptr) {
    return;
  }
  UpdateIOBytes(prev_bytes_read_, prev_bytes_written_, &io_bytes_read_,
                &io_bytes_written_);
  uint32_t cf_id = blob_gc_->column_family_handle()->GetID();
  TitanInternalStats* internal_stats = stats_->internal_stats(cf_id);
  if (internal_stats == nullptr) {
    return;
  }
  InternalOpStats* internal_op_stats =
      internal_stats->GetInternalOpStatsForType(InternalOpType::GC);
  assert(internal_op_stats != nullptr);
  AddStats(internal_op_stats, InternalOpStatsType::COUNT);
  AddStats(internal_op_stats, InternalOpStatsType::BYTES_READ,
           metrics_.gc_bytes_read_check + metrics_.gc_bytes_read_blob + metrics_.gc_bytes_read_callback);
  AddStats(internal_op_stats, InternalOpStatsType::BYTES_WRITTEN,
           metrics_.gc_bytes_written_lsm + metrics_.gc_bytes_written_blob);
  AddStats(internal_op_stats, InternalOpStatsType::IO_BYTES_READ,
           io_bytes_read_);
  AddStats(internal_op_stats, InternalOpStatsType::IO_BYTES_WRITTEN,
           io_bytes_written_);
  AddStats(internal_op_stats, InternalOpStatsType::INPUT_FILE_NUM,
           metrics_.gc_num_files);
  AddStats(internal_op_stats, InternalOpStatsType::OUTPUT_FILE_NUM,
           metrics_.gc_num_new_files);
  AddStats(internal_op_stats, InternalOpStatsType::GC_READ_LSM_MICROS,
           metrics_.gc_read_lsm_micros);
  AddStats(internal_op_stats, InternalOpStatsType::GC_UPDATE_LSM_MICROS,
           metrics_.gc_update_lsm_micros);
}

}  // namespace titandb
}  // namespace rocksdb
