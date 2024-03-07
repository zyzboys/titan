#pragma once

#include "titan/options.h"
#include "titan_stats.h"
#include "file/filename.h"
#include "db/version_edit.h"

namespace rocksdb {
namespace titandb {


// ShadowSet is the set of all the shadows generated during Titan GC.
// 
class ShadowSet {
public:
  explicit ShadowSet(const TitanDBOptions& options, TitanStats* stats);

  uint64_t NewFileNumber() { return next_file_number_.fetch_add(1); }

  std::string NewFileName(uint64_t file_number) {
    return ShadowFileName(dirname_, file_number);
  }

  std::vector<FileMetaData>& GetShadows() {
    return shadows_;
  }

 private:
  std::atomic<uint64_t> next_file_number_{1};
  std::string dirname_;
  TitanStats* stats_;
  std::vector<FileMetaData> shadows_;
  
};

}  // namespace titandb
}  // namespace rocksdb