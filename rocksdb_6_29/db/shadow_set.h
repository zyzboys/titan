#pragma once

#include <unordered_map>
#include "db/version_edit.h"
#include "rocksdb/options.h"
#include "blob_format.h"
#include "db/titan_blob_format.h"


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

class ShadowCache {
  public:
    ShadowCache() {}
    ~ShadowCache() {}

    struct ShadowCacheCompare {
      bool operator()(const std::string& a, const std::string& b) const {
        return BytewiseComparator()->Compare(Slice(a), Slice(b)) < 0;
      }
    };

    void Add(const std::string& key, std::pair<SequenceNumber, std::string>& value, std::map<uint64_t, std::set<uint64_t>>* drop_keys) {
      MutexLock l(&cache_mutex_);
      auto it = cache_.find(key);
      // if key already exists, add the key to drop_keys
      if (it != cache_.end()) {
        TitanBlobIndex index;
        Slice copy = value.second;
        index.DecodeFrom(&copy);
        // record drop keys in set
        (*drop_keys)[index.file_number].insert(index.blob_handle.order);
      }
      cache_[key] = value;
    }

    void Add(const std::string& key, std::pair<SequenceNumber, std::string>& value) {
      MutexLock l(&cache_mutex_);
      cache_[key] = value;
    }

    void AddMuti(std::unordered_map<std::string, std::pair<SequenceNumber, std::string>>& cache_addition, std::map<uint64_t, std::set<uint64_t>>* drop_keys) {
      MutexLock l(&cache_mutex_);
      //fprintf(stderr, "Add cache begin, cache size: %ld, addition size: %ld\n", cache_.size(), cache_addition.size());
      for (auto& cache : cache_addition) {
        auto it = cache_.find(cache.first);
        // if key already exists, add the key to drop_keys
        if (it != cache_.end()) {
          TitanBlobIndex index;
          Slice copy = cache.second.second;
          index.DecodeFrom(&copy);
          TitanBlobIndex original_index;
          Slice original_copy = it->second.second;
          original_index.DecodeFrom(&original_copy);
          if (original_index.file_number > index.file_number) {
            fprintf(stderr, "Error: shadow cache file number is not in order\n");
          }
          // record drop keys in set
          // if (it->second.first > cache.second.first) {
          //   //已有的序列号比新的大，说明是乱序的，需要报错
          //   fprintf(stderr, "Error: shadow cache seq number is not in order\n");
          // }
          (*drop_keys)[index.file_number].insert(index.blob_handle.order);
        }
        cache_[cache.first] = cache.second;
      }
      //fprintf(stderr, "Add cache done, cache size: %ld\n", cache_.size());
    }

    void AddMuti(std::unordered_map<std::string, std::pair<SequenceNumber, std::string>>& cache_addition) {
      MutexLock l(&cache_mutex_);
      for (auto& cache : cache_addition) {
        cache_[cache.first] = cache.second;
      }
    }

    void DeleteMuti(std::unordered_map<std::string, std::string>& cache_deletion) {
      MutexLock l(&cache_mutex_);
      //fprintf(stderr, "Delete cache begin, cache size: %ld, deletion size: %ld\n", cache_.size(), cache_deletion.size());
      for (auto& cache : cache_deletion) {
        TitanBlobIndex delete_blob_index;
        Slice delete_slice = cache.second;
        delete_blob_index.DecodeFrom(&delete_slice);
        auto it = cache_.find(cache.first);
        if (it != cache_.end()) {
          TitanBlobIndex blob_index;
          Slice original_slice = it->second.second;
          blob_index.DecodeFrom(&original_slice);
          // make sure we only delete the shadow cache that is older or equal than the new one
          if (blob_index.file_number <= delete_blob_index.file_number) {
            cache_.erase(it);
          }
        }
        //else it means the key is already deleted
      }
      //fprintf(stderr, "Delete cache done, cache size: %ld\n", cache_.size());
    }

    void Delete(const std::string& key, std::string& value) {
      MutexLock l(&cache_mutex_);
      TitanBlobIndex delete_blob_index;
      Slice delete_slice = value;
      delete_blob_index.DecodeFrom(&delete_slice);
      auto it = cache_.find(key);
      if (it != cache_.end()) {
        TitanBlobIndex blob_index;
        Slice original_slice = it->second.second;
        blob_index.DecodeFrom(&original_slice);
        // make sure we only delete the shadow cache that is older or equal than the new one
        if (blob_index.file_number <= delete_blob_index.file_number) {
          cache_.erase(it);
        }
      }
      //else it means the key is already deleted
    }

    bool KeyExist(const std::string& key) const {
      MutexLock l(&cache_mutex_);
      if (cache_.size() == 0) {
        return false;
      }
      return cache_.find(key) != cache_.end();
    }

    bool KeyExist(const std::string& key, std::string& value) const {
      MutexLock l(&cache_mutex_);
      if (cache_.size() == 0) {
        return false;
      }
      auto it = cache_.find(key);
      if (it != cache_.end()) {
        value = it->second.second;
        return true;
      }
      return false;
    }

    bool KeyExist(const std::string& key, SequenceNumber* seq, std::string& value) const {
      MutexLock l(&cache_mutex_);
      if (cache_.size() == 0) {
        return false;
      }
      auto it = cache_.find(key);
      if (it != cache_.end()) {
        *seq = it->second.first;
        value = it->second.second;
        return true;
      }
      return false;
    }

    port::Mutex* GetMutex() { return &cache_mutex_; }


  private:
    mutable port::Mutex cache_mutex_; 
    //std::map<std::string, std::pair<SequenceNumber, std::string>, ShadowCacheCompare> cache_;
    std::unordered_map<std::string, std::pair<SequenceNumber, std::string>> cache_;
    
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

  port::Mutex* GetShadowCacheMutex() { return shadow_cache_.GetMutex(); }

  void PrintShadowSummary();

  void ComputeShadowScore();

  // void AddCacheMuti(std::unordered_map<std::string, std::pair<SequenceNumber, std::string>>& cache_addition);

  // void AddCacheMuti(std::unordered_map<std::string, std::pair<SequenceNumber, std::string>>& cache_addition, std::map<uint64_t, std::set<uint64_t>>* drop_keys);

  // void DeleteCacheMuti(std::unordered_map<std::string, std::string>& cache_deletion);

  ShadowCache* GetShadowCache() { return &shadow_cache_; }

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
  // userkey-><value>
  ShadowCache shadow_cache_;
  // List of guards per level which are persisted to disk and already committed to a MANIFEST
  //std::vector<GuardMetaData*> guards_[LSM_MAX_LEVEL];
  std::unordered_map<uint64_t, FileLocation> file_locations_;
  int base_level_ = 1;
  int max_level_;
  
};


}  // namespace rocksdb