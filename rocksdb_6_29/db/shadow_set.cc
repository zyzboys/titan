#include "db/shadow_set.h"

namespace ROCKSDB_NAMESPACE {


ShadowSet::ShadowSet() {}
ShadowSet::~ShadowSet() {}

void ShadowSet::AddShadows(std::vector<std::pair<int, FileMetaData>>& shadows) {
    MutexLock l(&shadow_mutex_);
    for (auto& shadow : shadows) {
      FileMetaData* meta = new FileMetaData(shadow.second);
      meta->refs++;
      int level = shadow.first;
      shadow_files_[level].emplace_back(meta);
      const uint64_t shadow_number = meta->fd.GetNumber();

      assert(file_locations_.find(shadow_number) == file_locations_.end());
      file_locations_.emplace(shadow_number, FileLocation(level, shadow_files_[level].size() - 1));
    }
    UpdateMaxLevel();
  }

  void ShadowSet::UpdateMaxLevel() {
    for (int i = base_level_; i < LSM_MAX_LEVEL; i++) {
      if (shadow_files_[i].size() != 0 && max_level_ < i ) {
        max_level_ = i;
      }
    }
  }

  port::Mutex* ShadowSet::GetMutex() {
    return &shadow_mutex_;
  }

  void ShadowSet::PrintShadowSummary() {

  }

  void ShadowSet::ComputeShadowScore() {
    MutexLock l(&shadow_mutex_);    
    shadow_scores_.clear();
  }

  void ShadowSet::AddCache(std::unordered_map<std::string, std::string>& cache_addition) {
    MutexLock l(&shadow_mutex_);
    fprintf(stderr, "Add cache begin, cache size: %d\n", shadow_cache_.size());
    for (auto& cache : cache_addition) {
      shadow_cache_[cache.first] = cache.second;
    }
    fprintf(stderr, "Add cache done, cache size: %d\n", shadow_cache_.size());
  }

  void ShadowSet::DeleteCache(std::unordered_map<std::string, std::string>& cache_deletion) {
    MutexLock l(&shadow_mutex_);
    for (auto& cache : cache_deletion) {
      TitanBlobIndex delete_blob_index;
      Slice delete_slice(cache.second);
      delete_blob_index.DecodeFrom(&delete_slice);
      auto it = shadow_cache_.find(cache.first);
      if (it != shadow_cache_.end()) {
        TitanBlobIndex blob_index;
        Slice original_slice(it->second);
        blob_index.DecodeFrom(&original_slice);
        // make sure we only delete the shadow cache that is older or equal than the new one
        if (blob_index.file_number <= delete_blob_index.file_number) {
          shadow_cache_.erase(it);
        }
      }     
    }
  }

  bool ShadowSet::KeyExistInCache(const std::string& key) const {
    return shadow_cache_.find(key) != shadow_cache_.end();
  }


}  // namespace rocksdb