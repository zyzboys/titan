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


}  // namespace rocksdb