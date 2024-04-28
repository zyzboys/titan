#pragma once

#include "db/db_impl/db_impl.h"
#include "rocksdb/statistics.h"
#include "rocksdb/status.h"
#include "rocksdb/types.h"

#include "blob_file_builder.h"
#include "blob_file_iterator.h"
#include "blob_file_manager.h"
#include "blob_file_set.h"
#include "db/shadow_set.h"
#include "blob_gc.h"
#include "titan/options.h"
#include "titan_stats.h"
#include "version_edit.h"
#include "util/hash.h"

namespace rocksdb {
namespace titandb {

class BlobGCJob {
 public:
  BlobGCJob(BlobGC* blob_gc, DB* db, port::Mutex* mutex,
                     const TitanDBOptions& titan_db_options, Env* env,
                     const EnvOptions& env_options,
                     BlobFileManager* blob_file_manager,
                     BlobFileSet* blob_file_set, LogBuffer* log_buffer,
                     std::atomic_bool* shuting_down, TitanStats* stats, std::string& dbname, std::string& db_id, 
                     std::string& db_session_id, ShadowSet* shadow_set);
  
  BlobGCJob(BlobGC *blob_gc, DB *db, port::Mutex *mutex,
            const TitanDBOptions &titan_db_options, Env *env,
            const EnvOptions &env_options, BlobFileManager *blob_file_manager,
            BlobFileSet *blob_file_set, LogBuffer *log_buffer, std::atomic_bool *shuting_down, TitanStats *stats);

  // No copying allowed
  BlobGCJob(const BlobGCJob &) = delete;
  void operator=(const BlobGCJob &) = delete;

  ~BlobGCJob();

  // REQUIRE: mutex held
  Status Prepare();
  // REQUIRE: mutex not held
  Status Run();
  // REQUIRE: mutex held
  Status Finish();

 private:
  class GarbageCollectionWriteCallback;
  friend class BlobGCJobTest;

  void UpdateInternalOpStats();

  BlobGC *blob_gc_;
  DB *base_db_;
  DBImpl *base_db_impl_;
  // DBImpl state
  const std::string dbname_;
  const std::string db_id_;
  const std::string db_session_id_;
  std::shared_ptr<IOTracer> io_tracer_ = nullptr;
  ShadowSet *shadow_set_;

  port::Mutex *mutex_;
  TitanDBOptions db_options_;
  Env *env_;
  EnvOptions env_options_;
  BlobFileManager *blob_file_manager_;
  BlobFileSet *blob_file_set_;
  LogBuffer *log_buffer_{nullptr};

  std::vector<std::pair<std::unique_ptr<BlobFileHandle>,
                        std::unique_ptr<BlobFileBuilder>>>
      blob_file_builders_;
  
  std::vector<std::pair<int, FileMetaData>> shadow_metas_;
  std::unordered_map<std::string, std::pair<SequenceNumber, std::string>> cache_addition_;
  std::map<uint64_t, std::set<uint64_t>> drop_keys;
  uint64_t add_cache_count_ = 0;
  std::vector<std::pair<WriteBatch, GarbageCollectionWriteCallback>>
      rewrite_batches_;

  std::atomic_bool *shuting_down_{nullptr};

  TitanStats *stats_;

  struct {
    uint64_t gc_bytes_read_check = 0;
    uint64_t gc_bytes_read_blob = 0;
    uint64_t gc_bytes_read_callback = 0;
    uint64_t gc_bytes_written_lsm = 0;
    uint64_t gc_bytes_written_blob = 0;
    uint64_t gc_num_keys_overwritten_check = 0;
    uint64_t gc_num_keys_overwritten_callback = 0;
    uint64_t gc_bytes_overwritten_check = 0;
    uint64_t gc_bytes_overwritten_callback = 0;
    uint64_t gc_num_keys_relocated = 0;
    uint64_t gc_bytes_relocated = 0;
    uint64_t gc_num_keys_fallback = 0;
    uint64_t gc_bytes_fallback = 0;
    uint64_t gc_num_new_files = 0;
    uint64_t gc_num_files = 0;
    uint64_t gc_read_lsm_micros = 0;
    uint64_t gc_update_lsm_micros = 0;
  } metrics_;

  uint64_t prev_bytes_read_ = 0;
  uint64_t prev_bytes_written_ = 0;
  uint64_t io_bytes_read_ = 0;
  uint64_t io_bytes_written_ = 0;

  Status DoRunGC();
  void BatchWriteNewIndices(BlobFileBuilder::OutContexts &contexts, Status *s);
  Status BuildIterator(std::unique_ptr<BlobFileMergeIterator> *result);
  Status DiscardEntry(const Slice &key, const BlobIndex &blob_index,
                      bool *discardable, int *level, SequenceNumber* seq);
  Status DiscardEntryWithBitset(const BlobIndex &blob_index, bool *discardable);
  Status InstallOutputShadows();
  Status InstallOutputShadowCache();
  Status InstallOutputBlobFiles();
  Status RewriteValidKeyToLSM();
  Status OpenGCOutputShadow(ShadowHandle *handle, int level);
  Status AddToShadow(ShadowHandle *handle, BlobFileBuilder::OutContexts& contexts, SequenceNumber seq);
  Status AddToShadowCache(BlobFileBuilder::OutContexts& contexts, SequenceNumber seq);
  Status FinishGCOutputShadow(ShadowHandle *handle, int level);
  Status DeleteInputBlobFiles();

  bool IsShutingDown();
};

}  // namespace titandb
}  // namespace rocksdb
