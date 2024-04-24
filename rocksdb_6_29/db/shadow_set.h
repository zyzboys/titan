#pragma once

#include "db/version_edit.h"

#define LSM_MAX_LEVEL 7

namespace ROCKSDB_NAMESPACE {

// ShadowSet is the set of all the shadows generated during Titan GC.
// 
class ShadowSet {
public:
  explicit ShadowSet();

  ~ShadowSet();

  uint64_t NewFileNumber() { return next_file_number_.fetch_add(1); }

  void AddShadows(std::vector<std::pair<int, FileMetaData>>& shadows) {
    for (auto& shadow : shadows) {
      FileMetaData* meta = new FileMetaData(shadow.second);
      shadow_files_[shadow.first].emplace_back(meta);
    }
    UpdateMaxLevel();
  }

  void UpdateMaxLevel() {
    for (int i = base_level_; i < LSM_MAX_LEVEL; i++) {
      if (shadow_files_[i].size() != 0 && max_level_ < i ) {
        max_level_ = i;
      }
    }
  }

  void AddGuard(GuardMetaData* g, int level) {
    assert(level >=0 && level < LSM_MAX_LEVEL);
    guards_[level].emplace_back(g);
  }

  void AddToCompleteGuards(GuardMetaData* g, int level) {
    assert(level >=0 && level < LSM_MAX_LEVEL);
    complete_guards_[level].emplace_back(g);
  }

  class FileLocation {
   public:
    FileLocation() = default;
    FileLocation(int level, size_t position)
        : level_(level), position_(position) {}

    int GetLevel() const { return level_; }
    size_t GetPosition() const { return position_; }

    bool IsValid() const { return level_ >= 0; }

    bool operator==(const FileLocation& rhs) const {
      return level_ == rhs.level_ && position_ == rhs.position_;
    }

    bool operator!=(const FileLocation& rhs) const { return !(*this == rhs); }

    static FileLocation Invalid() { return FileLocation(); }

   private:
    int level_ = -1;
    size_t position_ = 0;
  };


 private:
  std::atomic<uint64_t> next_file_number_{1};
  //port::Mutex *mutex_;
  std::vector<FileMetaData*> shadow_files_[LSM_MAX_LEVEL];
  // List of guards per level which are persisted to disk and already committed to a MANIFEST
  std::vector<GuardMetaData*> guards_[LSM_MAX_LEVEL];
  // List of guards per level including the guards which are present in memory (not yet checkpointed)
  std::vector<GuardMetaData*> complete_guards_[LSM_MAX_LEVEL];
  std::unordered_map<uint64_t, FileLocation> file_locations_;
  int base_level_ = 1;
  int max_level_;
  
};

struct ShadowHandle {
    FileMetaData shadow_meta;
    std::shared_ptr<TableBuilder> shadow_builder;
    std::shared_ptr<WritableFileWriter> shadow_file;

    ShadowHandle() : shadow_meta(), shadow_builder(nullptr), shadow_file(nullptr) {}
};

}  // namespace rocksdb