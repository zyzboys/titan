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

  // void ShadowSet::AddCacheMuti(std::unordered_map<std::string, std::pair<SequenceNumber, std::string>>& cache_addition) {
  //   fprintf(stderr, "Add cache begin, cache size: %ld\n", GetShadowCache()->CacheSize());
  //   GetShadowCacheMutex()->Lock();
  //   for (auto& cache : cache_addition) {
  //     shadow_cache_.Add(cache.first, cache.second);
  //   }
  //   GetShadowCacheMutex()->Unlock();
  //   fprintf(stderr, "Add cache done, cache size: %ld\n", GetShadowCache()->CacheSize());
  // }

  // void ShadowSet::AddCacheMuti(std::unordered_map<std::string, std::pair<SequenceNumber, std::string>>& cache_addition, std::map<uint64_t, std::set<uint64_t>>* drop_keys) {
  //   fprintf(stderr, "Add cache begin, cache size: %ld\n", GetShadowCache()->CacheSize());
  //   GetShadowCacheMutex()->Lock();
  //   for (auto& cache : cache_addition) {
  //     shadow_cache_.Add(cache.first, cache.second, drop_keys);
  //   }
  //   GetShadowCacheMutex()->Unlock();
  //   fprintf(stderr, "Add cache done, cache size: %ld\n", GetShadowCache()->CacheSize());
  // }

  // void ShadowSet::DeleteCacheMuti(std::unordered_map<std::string, std::string>& cache_deletion) {
  //   fprintf(stderr, "Delete cache begin, cache size: %ld\n", GetShadowCache()->CacheSize());
  //   GetShadowCacheMutex()->Lock();
  //   for (auto& cache : cache_deletion) {
  //     shadow_cache_.Delete(cache.first, cache.second);   
  //   }
  //   GetShadowCacheMutex()->Unlock();
  //   fprintf(stderr, "Delete cache done, cache size: %ld\n", GetShadowCache()->CacheSize());
  // }


}  // namespace rocksdb