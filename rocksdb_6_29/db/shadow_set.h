#pragma once

#include "db/version_edit.h"
#include "rocksdb/options.h"

#define LSM_MAX_LEVEL 7

namespace ROCKSDB_NAMESPACE {

struct ShadowHandle {
    FileMetaData shadow_meta;
    std::shared_ptr<TableBuilder> shadow_builder;
    std::shared_ptr<WritableFileWriter> shadow_file;

    ShadowHandle() : shadow_meta(), shadow_builder(nullptr), shadow_file(nullptr) {}
};

struct ShadowScore {
  uint64_t shadow_number;
  double score;
};

// ShadowSet is the set of all the shadows generated during Titan GC.
// 
class ShadowSet {
public:
  explicit ShadowSet();

  ~ShadowSet();

  uint64_t NewFileNumber() { return next_file_number_.fetch_add(1); }

  void AddShadows(std::vector<std::pair<int, FileMetaData>>& shadows);

  void UpdateMaxLevel();

  port::Mutex* GetMutex();

  void PrintShadowSummary();

  void ComputeShadowScore();


  // void AddGuard(GuardMetaData* g, int level) {
  //   assert(level >=0 && level < LSM_MAX_LEVEL);
  //   guards_[level].emplace_back(g);
  // }


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
  mutable port::Mutex shadow_mutex_; //protect一些会被compaction和gc同时更新的东西，目前是考虑单线程compaction单线程gc
  std::vector<FileMetaData*> shadow_files_[LSM_MAX_LEVEL];
  std::vector<ShadowScore> shadow_scores_;
  // List of guards per level which are persisted to disk and already committed to a MANIFEST
  //std::vector<GuardMetaData*> guards_[LSM_MAX_LEVEL];
  std::unordered_map<uint64_t, FileLocation> file_locations_;
  int base_level_ = 1;
  int max_level_;
  
};


}  // namespace rocksdb