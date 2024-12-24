#pragma once

#include <unordered_map>
#include "db/version_edit.h"
#include "rocksdb/options.h"
#include "db/titan_blob_format.h"
#include <tbb/concurrent_hash_map.h>

#define LSM_MAX_LEVEL 7

typedef tbb::concurrent_hash_map<std::string, std::pair<uint64_t, std::string>> ConcurrentMap;
typedef ConcurrentMap::accessor ConcurrentMapAccessor;

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

class RedirectMap {
  public:
    RedirectMap() {}
    ~RedirectMap() {}

    void AddMulti(std::unordered_map<uint64_t, uint64_t>& redirect_map_diff) {
      MutexLock l(&redirect_mutex_);
      for (auto& entry_diff : redirect_map_diff) {
        if(redirect_map_.find(entry_diff.first) == redirect_map_.end()) {
          redirect_map_[entry_diff.first] = entry_diff.second;
        } else {
          redirect_map_[entry_diff.first] += entry_diff.second;
        }
      }
    }

    void DeleteMulti(std::unordered_set<uint64_t>& to_delete) {
      MutexLock l(&redirect_mutex_);
      for (auto& one_delete : to_delete) {
        if(redirect_map_.find(one_delete) != redirect_map_.end()) {
          redirect_map_.erase(one_delete);
          //std::cout<<"sst number: "<<one_delete<<" is deleted from redirect map"<<std::endl;
        }
      }
    }

    uint64_t GetRedirectNum(uint64_t file_number) {
      MutexLock l(&redirect_mutex_);
      auto it = redirect_map_.find(file_number);
      if (it != redirect_map_.end()) {
        return it->second;
      }
      return 0;
    }
    
    void GetRedirectMapCopy(std::unordered_map<uint64_t, uint64_t>& redirect_map_copy) {
      MutexLock l(&redirect_mutex_);
      for (auto& entry : redirect_map_) {
        redirect_map_copy[entry.first] = entry.second;
      }
    }

    void PrintBrief() {
      MutexLock l(&redirect_mutex_);
      for (auto& it : redirect_map_) {
        std::cout<<"sst file number: "<<it.first<<" redirect entries: "<<it.second<<std::endl;
      }
    }

  private:
    mutable port::Mutex redirect_mutex_; 
    std::unordered_map<uint64_t, uint64_t> redirect_map_;//sst file number -> redirect entries num
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

    void Add(const std::string& key, std::pair<uint64_t, std::string>& value, std::map<uint64_t, std::set<uint64_t>>* drop_keys) {
      //MutexLock l(&cache_mutex_);
      ConcurrentMapAccessor cacheAssessor;
      const auto isFound = cache_.find(cacheAssessor, key);
      // if key already exists, add the key to drop_keys
      if (isFound == true) {
        TitanBlobIndex index;
        Slice copy = value.second;
        index.DecodeFrom(&copy);
        // record drop keys in set
        (*drop_keys)[index.file_number].insert(index.blob_handle.order);
      }
      cache_.insert(cacheAssessor, key);
      cacheAssessor->second = value;
    }

    void Add(const std::string& key, std::pair<uint64_t, std::string>& value) {
      //MutexLock l(&cache_mutex_);
      ConcurrentMapAccessor cacheAssessor;
      cache_.insert(cacheAssessor, key);
      cacheAssessor->second = value;
    }

    void AddMulti(std::unordered_map<std::string, std::pair<uint64_t, std::string>>& cache_addition, std::map<uint64_t, std::set<uint64_t>>* drop_keys) {
      //MutexLock l(&cache_mutex_);
      //fprintf(stderr, "Add cache begin, cache size: %ld, addition size: %ld\n", cache_.size(), cache_addition.size());
      for (auto& cache : cache_addition) {
        ConcurrentMapAccessor cacheAssessor;
        const auto isFound = cache_.find(cacheAssessor, cache.first);
        // if key already exists, add the key to drop_keys
        if (isFound == true) {
          TitanBlobIndex index;
          Slice copy = cache.second.second;
          index.DecodeFrom(&copy);
          TitanBlobIndex original_index;
          Slice original_copy = cacheAssessor->second.second;
          original_index.DecodeFrom(&original_copy);
          
          // record drop keys in set
          if (cacheAssessor->second.first > cache.second.first || original_index.file_number > index.file_number) {
            //保证file_number是递增的
            //fprintf(stderr, "in AddMulti, original father file number: %ld, new father file number: %ld ", it->second.first, cache.second.first);
            //fprintf(stderr, "original redirect file number: %ld, new redirect file number: %ld, can't add\n", original_index.file_number, index.file_number);
            continue;
          }
          //将旧的标记为drop
          (*drop_keys)[index.file_number].insert(index.blob_handle.order);
        }
        //cache_[cache.first] = cache.second;
        cache_.insert(cacheAssessor, cache.first);
        cacheAssessor->second = cache.second;
      }
      //fprintf(stderr, "Add cache done, cache size: %ld\n", cache_.size());
    }

    void AddMulti(std::unordered_map<std::string, std::pair<uint64_t, std::string>>& cache_addition) {
      //MutexLock l(&cache_mutex_);
      for (auto& cache : cache_addition) {
        ConcurrentMapAccessor cacheAssessor;
        cache_.insert(cacheAssessor, cache.first);
        cacheAssessor->second = cache.second;
      }
    }

    void DeleteMulti(std::unordered_map<std::string, std::string>& cache_deletion) {
      //MutexLock l(&cache_mutex_);
      //fprintf(stderr, "Delete cache begin, cache size: %ld, deletion size: %ld\n", cache_.size(), cache_deletion.size());
      for (auto& cache : cache_deletion) {
        TitanBlobIndex delete_blob_index;
        Slice delete_slice = cache.second;
        delete_blob_index.DecodeFrom(&delete_slice);
        ConcurrentMapAccessor cacheAssessor;
        const auto isFound = cache_.find(cacheAssessor, cache.first);
        if (isFound == true) {
          TitanBlobIndex blob_index;
          Slice original_slice = cacheAssessor->second.second;
          blob_index.DecodeFrom(&original_slice);
          // make sure we only delete the shadow cache that is older or equal than the new one
          if (blob_index.file_number <= delete_blob_index.file_number) {
            cache_.erase(cacheAssessor);
          } else {
            //fprintf(stderr, "in DelteMulti, original redirect number: %ld is bigger than delete redirect number: %ld, can't delete\n", blob_index.file_number, delete_blob_index.file_number);
          }
        }
        //else it means the key is already deleted
      }
      //fprintf(stderr, "Delete cache done, cache size: %ld\n", cache_.size());
    }

    void Delete(const std::string& key, std::string& value) {
      //MutexLock l(&cache_mutex_);
      TitanBlobIndex delete_blob_index;
      Slice delete_slice = value;
      delete_blob_index.DecodeFrom(&delete_slice);
      ConcurrentMapAccessor cacheAssessor;
      const auto isFound = cache_.find(cacheAssessor, key);
      if (isFound == true) {
        TitanBlobIndex blob_index;
        Slice original_slice = cacheAssessor->second.second;
        blob_index.DecodeFrom(&original_slice);
        // make sure we only delete the shadow cache that is older or equal than the new one
        if (blob_index.file_number <= delete_blob_index.file_number) {
          cache_.erase(cacheAssessor);
        }
      }
      //else it means the key is already deleted
    }

    bool KeyExist(const std::string& key) const {
      //MutexLock l(&cache_mutex_);
      if (cache_.size() == 0) {
        return false;
      }
      ConcurrentMapAccessor cacheAssessor;
      const auto isFound = cache_.find(cacheAssessor, key);
      return isFound;
    }

    bool KeyExist(const std::string& key, std::string& value) const {
      //MutexLock l(&cache_mutex_);
      if (cache_.size() == 0) {
        return false;
      }
      ConcurrentMapAccessor cacheAssessor;
      const auto isFound = cache_.find(cacheAssessor, key);
      if (isFound == true) {
        value = cacheAssessor->second.second;
        return true;
      }
      return false;
    }

    bool KeyExist(const std::string& key, uint64_t* file_number, std::string& value) const {
      //MutexLock l(&cache_mutex_);
      if (cache_.size() == 0) {
        return false;
      }
      ConcurrentMapAccessor cacheAssessor;
      const auto isFound = cache_.find(cacheAssessor, key);
      if (isFound == true) {
        *file_number = cacheAssessor->second.first;
        value = cacheAssessor->second.second;
        return true;
      }
      return false;
    }

    bool KeyExistInCopy(const std::string& key, uint64_t* file_number, std::string& value, std::unordered_map<std::string, std::pair<uint64_t, std::string>>& cache_copy) const {
      if (cache_copy.size() == 0) {
        return false;
      }
      auto it = cache_copy.find(key);
      if (it != cache_copy.end()) {
        *file_number = it->second.first;
        value = it->second.second;
        return true;
      }
      return false;
    }

    void GetShadowCacheCopy(std::unordered_map<std::string, std::pair<uint64_t, std::string>>& cache_copy) {
      //MutexLock l(&cache_mutex_);
      for (auto& entry : cache_) {
        cache_copy[entry.first] = entry.second;
      }
    }

    port::Mutex* GetMutex() { return &cache_mutex_; }

  private:
    mutable port::Mutex cache_mutex_; 
    //std::map<std::string, std::pair<SequenceNumber, std::string>, ShadowCacheCompare> cache_;
    ConcurrentMap cache_;//userkey->(father number, value)
    
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
  RedirectMap* GetRedirectEntriesMap() { return &redirect_map_; }

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
  RedirectMap redirect_map_;
  // List of guards per level which are persisted to disk and already committed to a MANIFEST
  //std::vector<GuardMetaData*> guards_[LSM_MAX_LEVEL];
  std::unordered_map<uint64_t, FileLocation> file_locations_;
  int base_level_ = 1;
  int max_level_;
  
};


}  // namespace rocksdb