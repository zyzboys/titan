#!/usr/bin/env bash
# uniform
# titan
rm -rf /home/data/zhangyuzhan/test/db/uniform/*
rm -rf /home/data/zhangyuzhan/test/log/uniform/space_utilization
cd /home/master/finch/src/titan/build/
./titandb_bench --benchmarks=fillrandom,stats,overwrite,stats  --db=/home/data/zhangyuzhan/test/db/uniform/titan --statistics=true --num=50000000 --key_size=16 --value_size=1024 --compression_type=none --compression_ratio=1 --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 --titan_max_gc_batch_size=268435456 > /home/data/zhangyuzhan/test/log/uniform/titan/titan_fillupdate50G
echo "titan space utilization: " >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
du -k --max-depth=0 /home/data/zhangyuzhan/test/db/uniform/titan >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
cp -f /home/data/zhangyuzhan/test/db/uniform/titan/LOG /home/data/zhangyuzhan/test/log/uniform/titan/LOG
rm -rf /home/data/zhangyuzhan/test/db/uniform/titan

# titan_noGC
./titandb_bench --benchmarks=fillrandom,stats,overwrite,stats  --db=/home/data/zhangyuzhan/test/db/uniform/titan_noGC --statistics=true --num=50000000 --key_size=16 --value_size=1024 --titan_disable_background_gc=true --compression_type=none --compression_ratio=1 --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 --titan_max_gc_batch_size=268435456 > /home/data/zhangyuzhan/test/log/uniform/titan_noGC/titan_noGC_fillupdate50G
echo "titan_noGC space utilization: " >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
du -k --max-depth=0 /home/data/zhangyuzhan/test/db/uniform/titan_noGC >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
cp -f /home/data/zhangyuzhan/test/db/uniform/titan_noGC/LOG /home/data/zhangyuzhan/test/log/uniform/titan_noGC/LOG
rm -rf /home/data/zhangyuzhan/test/db/uniform/titan_noGC

# titan_level
./titandb_bench --benchmarks=fillrandom,stats,overwrite,stats  --db=/home/data/zhangyuzhan/test/db/uniform/titan_level --statistics=true --num=50000000 --key_size=16 --value_size=1024 --level_compaction_dynamic_level_bytes=true --titan_level_merge=true --titan_disable_background_gc=true --compression_type=none --compression_ratio=1 --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 --titan_max_gc_batch_size=268435456 > /home/data/zhangyuzhan/test/log/uniform/titan_level/titan_level_fillupdate50G
echo "titan_level space utilization: " >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
du -k --max-depth=0 /home/data/zhangyuzhan/test/db/uniform/titan_level >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
cp -f /home/data/zhangyuzhan/test/db/uniform/titan_level/LOG /home/data/zhangyuzhan/test/log/uniform/titan_level/LOG
rm -rf /home/data/zhangyuzhan/test/db/uniform/titan_level

# rocksdb
cd /home/master/finch/src/rocksdb/build/
./db_bench --benchmarks=fillrandom,stats,overwrite,stats --db=/home/data/zhangyuzhan/test/db/uniform/rocksdb --statistics=true  --num=50000000 --key_size=16 --value_size=1024 --compression_type=none --compression_ratio=1 > /home/data/zhangyuzhan/test/log/uniform/rocksdb/rocksdb_fillupdate50G
echo "rocksdb space utilization: " >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
du -k --max-depth=0 /home/data/zhangyuzhan/test/db/uniform/rocksdb >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
cp -f /home/data/zhangyuzhan/test/db/uniform/rocksdb/LOG /home/data/zhangyuzhan/test/log/uniform/rocksdb/LOG
rm -rf /home/data/zhangyuzhan/test/db/uniform/rocksdb

# blobdb
./db_bench --benchmarks=fillrandom,stats,overwrite,stats --db=/home/data/zhangyuzhan/test/db/uniform/blobdb --statistics=true  --num=50000000 --key_size=16 --value_size=1024 --compression_type=none --compression_ratio=1 --enable_blob_files=true --enable_blob_garbage_collection=true --blob_garbage_collection_force_threshold=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --blob_file_size=67108864 >> /home/data/zhangyuzhan/test/log/uniform/blobdb/blobdb_fillupdate50G
echo "blobdb space utilization: " >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
du -k --max-depth=0 /home/data/zhangyuzhan/test/db/uniform/blobdb >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
cp -f /home/data/zhangyuzhan/test/db/uniform/blobdb/LOG /home/data/zhangyuzhan/test/log/uniform/blobdb/LOG
rm -rf /home/data/zhangyuzhan/test/db/uniform/blobdb

# blobdb_noGC
./db_bench --benchmarks=fillrandom,stats,overwrite,stats --db=/home/data/zhangyuzhan/test/db/uniform/blobdb_noGC --statistics=true  --num=50000000 --key_size=16 --value_size=1024 --compression_type=none --compression_ratio=1 --enable_blob_files=true --enable_blob_garbage_collection=false --blob_garbage_collection_force_threshold=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --blob_file_size=67108864 >> /home/data/zhangyuzhan/test/log/uniform/blobdb_noGC/blobdb_noGC_fillupdate50G
echo "blobdb_noGC space utilization: " >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
du -k --max-depth=0 /home/data/zhangyuzhan/test/db/uniform/blobdb_noGC >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
cp -f /home/data/zhangyuzhan/test/db/uniform/blobdb_noGC/LOG /home/data/zhangyuzhan/test/log/uniform/blobdb_noGC/LOG
rm -rf /home/data/zhangyuzhan/test/db/uniform/blobdb_noGC



# zipfian
rm -rf /home/data/zhangyuzhan/test/db/zipfian/*
rm -rf /home/data/zhangyuzhan/test/log/zipfian/space_utilization
# titan
cd /home/master/finch/src/titan/build/
./titandb_bench --benchmarks=fillrandom,stats,overwrite,stats  --db=/home/data/zhangyuzhan/test/db/zipfian/titan --statistics=true --zipfian=1 --zipfian_const=0.9 --num=50000000 --key_size=16 --value_size=1024 --compression_type=none --compression_ratio=1 --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 --titan_max_gc_batch_size=268435456 > /home/data/zhangyuzhan/test/log/zipfian/titan/titan_fillupdate50G
echo "titan space utilization: " >> /home/data/zhangyuzhan/test/log/zipfian/space_utilization
du -k --max-depth=0 /home/data/zhangyuzhan/test/db/zipfian/titan >> /home/data/zhangyuzhan/test/log/zipfian/space_utilization
cp -f /home/data/zhangyuzhan/test/db/zipfian/titan/LOG /home/data/zhangyuzhan/test/log/zipfian/titan/LOG
rm -rf /home/data/zhangyuzhan/test/db/zipfian/titan

# titan_noGC
./titandb_bench --benchmarks=fillrandom,stats,overwrite,stats  --db=/home/data/zhangyuzhan/test/db/zipfian/titan_noGC --statistics=true --zipfian=1 --zipfian_const=0.9 --num=50000000 --key_size=16 --value_size=1024 --titan_disable_background_gc=true --compression_type=none --compression_ratio=1 --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 --titan_max_gc_batch_size=268435456 > /home/data/zhangyuzhan/test/log/zipfian/titan_noGC/titan_noGC_fillupdate50G
echo "titan_noGC space utilization: " >> /home/data/zhangyuzhan/test/log/zipfian/space_utilization
du -k --max-depth=0 /home/data/zhangyuzhan/test/db/zipfian/titan_noGC >> /home/data/zhangyuzhan/test/log/zipfian/space_utilization
cp -f /home/data/zhangyuzhan/test/db/zipfian/titan_noGC/LOG /home/data/zhangyuzhan/test/log/zipfian/titan_noGC/LOG
rm -rf /home/data/zhangyuzhan/test/db/zipfian/titan_noGC

# titan_level
./titandb_bench --benchmarks=fillrandom,stats,overwrite,stats  --db=/home/data/zhangyuzhan/test/db/zipfian/titan_level --statistics=true --zipfian=1 --zipfian_const=0.9 --num=50000000 --key_size=16 --value_size=1024 --level_compaction_dynamic_level_bytes=true --titan_level_merge=true --titan_disable_background_gc=true --compression_type=none --compression_ratio=1 --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 --titan_max_gc_batch_size=268435456 > /home/data/zhangyuzhan/test/log/zipfian/titan_level/titan_level_fillupdate50G
echo "titan_level space utilization: " >> /home/data/zhangyuzhan/test/log/zipfian/space_utilization
du -k --max-depth=0 /home/data/zhangyuzhan/test/db/zipfian/titan_level >> /home/data/zhangyuzhan/test/log/zipfian/space_utilization
cp -f /home/data/zhangyuzhan/test/db/zipfian/titan_level/LOG /home/data/zhangyuzhan/test/log/zipfian/titan_level/LOG
rm -rf /home/data/zhangyuzhan/test/db/zipfian/titan_level

# rocksdb
cd /home/master/finch/src/rocksdb/build/
./db_bench --benchmarks=fillrandom,stats,overwrite,stats --db=/home/data/zhangyuzhan/test/db/zipfian/rocksdb --statistics=true  --zipfian=1 --zipfian_const=0.9 --num=50000000 --key_size=16 --value_size=1024 --compression_type=none --compression_ratio=1 > /home/data/zhangyuzhan/test/log/zipfian/rocksdb/rocksdb_fillupdate50G
echo "rocksdb space utilization: " >> /home/data/zhangyuzhan/test/log/zipfian/space_utilization
du -k --max-depth=0 /home/data/zhangyuzhan/test/db/zipfian/rocksdb >> /home/data/zhangyuzhan/test/log/zipfian/space_utilization
cp -f /home/data/zhangyuzhan/test/db/zipfian/rocksdb/LOG /home/data/zhangyuzhan/test/log/zipfian/rocksdb/LOG
rm -rf /home/data/zhangyuzhan/test/db/zipfian/rocksdb

# blobdb
./db_bench --benchmarks=fillrandom,stats,overwrite,stats --db=/home/data/zhangyuzhan/test/db/zipfian/blobdb --statistics=true  --zipfian=1 --zipfian_const=0.9 --num=50000000 --key_size=16 --value_size=1024 --compression_type=none --compression_ratio=1 --enable_blob_files=true --enable_blob_garbage_collection=true --blob_garbage_collection_force_threshold=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --blob_file_size=67108864 >> /home/data/zhangyuzhan/test/log/zipfian/blobdb/blobdb_fillupdate50G
echo "blobdb space utilization: " >> /home/data/zhangyuzhan/test/log/zipfian/space_utilization
du -k --max-depth=0 /home/data/zhangyuzhan/test/db/zipfian/blobdb >> /home/data/zhangyuzhan/test/log/zipfian/space_utilization
cp -f /home/data/zhangyuzhan/test/db/zipfian/blobdb/LOG /home/data/zhangyuzhan/test/log/zipfian/blobdb/LOG
rm -rf /home/data/zhangyuzhan/test/db/zipfian/blobdb

# blobdb_noGC
./db_bench --benchmarks=fillrandom,stats,overwrite,stats --db=/home/data/zhangyuzhan/test/db/zipfian/blobdb_noGC --statistics=true  --zipfian=1 --zipfian_const=0.9 --num=50000000 --key_size=16 --value_size=1024 --compression_type=none --compression_ratio=1 --enable_blob_files=true --enable_blob_garbage_collection=false --blob_garbage_collection_force_threshold=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --blob_file_size=67108864 >> /home/data/zhangyuzhan/test/log/zipfian/blobdb_noGC/blobdb_noGC_fillupdate50G
echo "blobdb_noGC space utilization: " >> /home/data/zhangyuzhan/test/log/zipfian/space_utilization
du -k --max-depth=0 /home/data/zhangyuzhan/test/db/zipfian/blobdb_noGC >> /home/data/zhangyuzhan/test/log/zipfian/space_utilization
cp -f /home/data/zhangyuzhan/test/db/zipfian/blobdb_noGC/LOG /home/data/zhangyuzhan/test/log/zipfian/blobdb_noGC/LOG
rm -rf /home/data/zhangyuzhan/test/db/zipfian/blobdb_noGC