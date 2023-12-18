#include "shadow_set.h"

namespace rocksdb {
namespace titandb {

ShadowSet::ShadowSet(const TitanDBOptions& options, TitanStats* stats)
    : dirname_(options.dirname),
      stats_(stats){}

}  // namespace titandb
}  // namespace rocksdb