#!/usr/bin/env bash
# uniform
# titan
rm -rf /home/data/zhangyuzhan/test/db/uniform/*
rm -rf /home/data/zhangyuzhan/test/log/uniform/*
cd /home/master/finch/src/titan/build/
./titandb_bench --benchmarks=fillrandom,stats  --db=/home/data/zhangyuzhan/test/db/uniform/titan --statistics=true --num=50000000 --key_size=16 --value_size=1024 --compression_type=none --compression_ratio=1 --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 --titan_max_gc_batch_size=268435456 > /home/data/zhangyuzhan/test/log/uniform/titan_fillrandom50G
echo "titan fill space utilization: " >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
du -k --max-depth=0 /home/data/zhangyuzhan/test/db/uniform/titan >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
cp -f /home/data/zhangyuzhan/test/db/uniform/titan/LOG /home/data/zhangyuzhan/test/log/uniform/titan_fillrandom50G_LOG
./titandb_bench --benchmarks=overwrite,stats  --db=/home/data/zhangyuzhan/test/db/uniform/titan --use_existing_db=1 --statistics=true --num=50000000 --key_size=16 --value_size=1024 --compression_type=none --compression_ratio=1 --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 --titan_max_gc_batch_size=268435456 > /home/data/zhangyuzhan/test/log/uniform/titan_updaterandom50G
echo "titan update space utilization: " >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
du -k --max-depth=0 /home/data/zhangyuzhan/test/db/uniform/titan >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
cp -f /home/data/zhangyuzhan/test/db/uniform/titan/LOG /home/data/zhangyuzhan/test/log/uniform/titan_updaterandom50G_LOG
rm -rf /home/data/zhangyuzhan/test/db/uniform/titan
echo "bench uniform titan done"

# titan_noGC
./titandb_bench --benchmarks=fillrandom,stats  --db=/home/data/zhangyuzhan/test/db/uniform/titan_noGC --statistics=true --num=50000000 --key_size=16 --value_size=1024 --titan_disable_background_gc=true --compression_type=none --compression_ratio=1 --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 --titan_max_gc_batch_size=268435456 > /home/data/zhangyuzhan/test/log/uniform/titan_noGC_fillrandom50G
echo "titan_noGC fill space utilization: " >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
du -k --max-depth=0 /home/data/zhangyuzhan/test/db/uniform/titan_noGC >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
cp -f /home/data/zhangyuzhan/test/db/uniform/titan_noGC/LOG /home/data/zhangyuzhan/test/log/uniform/titan_noGC_fillrandom50G_LOG
./titandb_bench --benchmarks=overwrite,stats  --db=/home/data/zhangyuzhan/test/db/uniform/titan_noGC --use_existing_db=1 --statistics=true --num=50000000 --key_size=16 --value_size=1024 --titan_disable_background_gc=true --compression_type=none --compression_ratio=1 --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 --titan_max_gc_batch_size=268435456 > /home/data/zhangyuzhan/test/log/uniform/titan_noGC_updaterandom50G
echo "titan_noGC update space utilization: " >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
du -k --max-depth=0 /home/data/zhangyuzhan/test/db/uniform/titan_noGC >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
cp -f /home/data/zhangyuzhan/test/db/uniform/titan_noGC/LOG /home/data/zhangyuzhan/test/log/uniform/titan_noGC_updaterandom50G_LOG
rm -rf /home/data/zhangyuzhan/test/db/uniform/titan_noGC
echo "bench uniform titan_noGC done"

# titan_level
./titandb_bench --benchmarks=fillrandom,stats  --db=/home/data/zhangyuzhan/test/db/uniform/titan_level --statistics=true --num=50000000 --key_size=16 --value_size=1024 --level_compaction_dynamic_level_bytes=true --titan_level_merge=true --titan_disable_background_gc=true --compression_type=none --compression_ratio=1 --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 --titan_max_gc_batch_size=268435456 > /home/data/zhangyuzhan/test/log/uniform/titan_level_fillrandom50G
echo "titan_level fill space utilization: " >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
du -k --max-depth=0 /home/data/zhangyuzhan/test/db/uniform/titan_level >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
cp -f /home/data/zhangyuzhan/test/db/uniform/titan_level/LOG /home/data/zhangyuzhan/test/log/uniform/titan_level_fillrandom50G_LOG
./titandb_bench --benchmarks=overwrite,stats  --db=/home/data/zhangyuzhan/test/db/uniform/titan_level --use_existing_db=1 --statistics=true --num=50000000 --key_size=16 --value_size=1024 --level_compaction_dynamic_level_bytes=true --titan_level_merge=true --titan_disable_background_gc=true --compression_type=none --compression_ratio=1 --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 --titan_max_gc_batch_size=268435456 > /home/data/zhangyuzhan/test/log/uniform/titan_level_updaterandom50G
echo "titan_level update space utilization: " >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
du -k --max-depth=0 /home/data/zhangyuzhan/test/db/uniform/titan_level >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
cp -f /home/data/zhangyuzhan/test/db/uniform/titan_level/LOG /home/data/zhangyuzhan/test/log/uniform/titan_level_updaterandom50G_LOG
rm -rf /home/data/zhangyuzhan/test/db/uniform/titan_level
echo "bench uniform titan_level done"

# rocksdb
cd /home/master/finch/src/rocksdb/build/
./db_bench --benchmarks=fillrandom,stats --db=/home/data/zhangyuzhan/test/db/uniform/rocksdb --statistics=true  --num=50000000 --key_size=16 --value_size=1024 --compression_type=none --compression_ratio=1 > /home/data/zhangyuzhan/test/log/uniform/rocksdb_fillrandom50G
echo "rocksdb fill space utilization: " >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
du -k --max-depth=0 /home/data/zhangyuzhan/test/db/uniform/rocksdb >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
cp -f /home/data/zhangyuzhan/test/db/uniform/rocksdb/LOG /home/data/zhangyuzhan/test/log/uniform/rocksdb_fillrandom50G_LOG
./db_bench --benchmarks=overwrite,stats --db=/home/data/zhangyuzhan/test/db/uniform/rocksdb --use_existing_db=1 --statistics=true  --num=50000000 --key_size=16 --value_size=1024 --compression_type=none --compression_ratio=1 > /home/data/zhangyuzhan/test/log/uniform/rocksdb_updaterandom50G
echo "rocksdb update space utilization: " >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
du -k --max-depth=0 /home/data/zhangyuzhan/test/db/uniform/rocksdb >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
cp -f /home/data/zhangyuzhan/test/db/uniform/rocksdb/LOG /home/data/zhangyuzhan/test/log/uniform/rocksdb_updaterandom50G_LOG
rm -rf /home/data/zhangyuzhan/test/db/uniform/rocksdb
echo "bench uniform rocksdb done"

# blobdb
./db_bench --benchmarks=fillrandom,stats --db=/home/data/zhangyuzhan/test/db/uniform/blobdb --statistics=true  --num=50000000 --key_size=16 --value_size=1024 --compression_type=none --compression_ratio=1 --enable_blob_files=true --enable_blob_garbage_collection=true --blob_garbage_collection_force_threshold=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --blob_file_size=67108864 >> /home/data/zhangyuzhan/test/log/uniform/blobdb_fillrandom50G
echo "blobdb fill space utilization: " >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
du -k --max-depth=0 /home/data/zhangyuzhan/test/db/uniform/blobdb >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
cp -f /home/data/zhangyuzhan/test/db/uniform/blobdb/LOG /home/data/zhangyuzhan/test/log/uniform/blobdb_fillrandom50G_LOG
./db_bench --benchmarks=overwrite,stats --db=/home/data/zhangyuzhan/test/db/uniform/blobdb --use_existing_db=1 --statistics=true  --num=50000000 --key_size=16 --value_size=1024 --compression_type=none --compression_ratio=1 --enable_blob_files=true --enable_blob_garbage_collection=true --blob_garbage_collection_force_threshold=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --blob_file_size=67108864 >> /home/data/zhangyuzhan/test/log/uniform/blobdb_updaterandom50G
echo "blobdb update space utilization: " >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
du -k --max-depth=0 /home/data/zhangyuzhan/test/db/uniform/blobdb >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
cp -f /home/data/zhangyuzhan/test/db/uniform/blobdb/LOG /home/data/zhangyuzhan/test/log/uniform/blobdb_updaterandom50G_LOG
rm -rf /home/data/zhangyuzhan/test/db/uniform/blobdb
echo "bench uniform blobdb done"

# blobdb_noGC
./db_bench --benchmarks=fillrandom,stats --db=/home/data/zhangyuzhan/test/db/uniform/blobdb_noGC --statistics=true  --num=50000000 --key_size=16 --value_size=1024 --compression_type=none --compression_ratio=1 --enable_blob_files=true --enable_blob_garbage_collection=false --blob_garbage_collection_force_threshold=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --blob_file_size=67108864 >> /home/data/zhangyuzhan/test/log/uniform/blobdb_noGC_fillrandom50G
echo "blobdb_noGC fill space utilization: " >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
du -k --max-depth=0 /home/data/zhangyuzhan/test/db/uniform/blobdb_noGC >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
cp -f /home/data/zhangyuzhan/test/db/uniform/blobdb_noGC/LOG /home/data/zhangyuzhan/test/log/uniform/blobdb_noGC_fillrandom50G_LOG
./db_bench --benchmarks=overwrite,stats --db=/home/data/zhangyuzhan/test/db/uniform/blobdb_noGC --use_existing_db=1 --statistics=true  --num=50000000 --key_size=16 --value_size=1024 --compression_type=none --compression_ratio=1 --enable_blob_files=true --enable_blob_garbage_collection=false --blob_garbage_collection_force_threshold=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --blob_file_size=67108864 >> /home/data/zhangyuzhan/test/log/uniform/blobdb_noGC_updaterandom50G
echo "blobdb_noGC update space utilization: " >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
du -k --max-depth=0 /home/data/zhangyuzhan/test/db/uniform/blobdb_noGC >> /home/data/zhangyuzhan/test/log/uniform/space_utilization
cp -f /home/data/zhangyuzhan/test/db/uniform/blobdb_noGC/LOG /home/data/zhangyuzhan/test/log/uniform/blobdb_noGC_updaterandom50G_LOG
rm -rf /home/data/zhangyuzhan/test/db/uniform/blobdb_noGC
echo "bench uniform blobdb_noGC done"