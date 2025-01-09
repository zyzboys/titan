#!/usr/bin/env bash
rm -rf /users/peiqi714/test/ycsb_log/*
rm -rf /users/peiqi714/test/db/*
thread_count=1
echo "start ycsb exp"
for wkld in ycsbwklda ycsbwkldb ycsbwkldc ycsbwkldd ycsbwklde ycsbwkldf
do
    entry_count=50000000
    val_size=1024
    echo "entry count: $entry_count"
    ehco "value size: $val_size"

    # rocksdb
    rm -rf /users/peiqi714/test/db/*
    ../rocksdb_6_29/build/db_bench --benchmarks=ycsbfilldb,stats,${wkld},stats  --db=/users/peiqi714/test/db/db --threads=$thread_count \
                    --statistics=true  --num=$entry_count --ycsb_num=$entry_count \
                    --key_size=16 --value_size=$val_size --compression_type=none --compression_ratio=1 > /users/peiqi714/test/ycsb_log/rocksdb_${wkld}
    echo "rocksdb_${wkld} space:" >> /users/peiqi714/test/ycsb_log/space_util_ycsb
    du -k --max-depth=0 /users/peiqi714/test/db/db >> /users/peiqi714/test/ycsb_log/space_util_ycsb
    cp -f /users/peiqi714/test/db/db/LOG /users/peiqi714/test/ycsb_log/rocksdb_${wkld}_LOG

    # blobdb
    rm -rf /users/peiqi714/test/db/*
    ../rocksdb_6_29/build/db_bench --benchmarks=ycsbfilldb,stats,${wkld},stats  --db=/users/peiqi714/test/db/db --threads=$thread_count \
                    --statistics=true  --num=$entry_count --ycsb_num=$entry_count \
                    --key_size=16 --value_size=$val_size --compression_type=none --compression_ratio=1 \
                    --enable_blob_files=true --enable_blob_garbage_collection=true \
                    --blob_garbage_collection_force_threshold=0.3 --target_file_size_base=4194304 \
                    --max_bytes_for_level_base=16777216 --blob_file_size=67108864 > /users/peiqi714/test/ycsb_log/blobdb_${wkld}
    echo "blobdb_${wkld} space:" >> /users/peiqi714/test/ycsb_log/space_util_ycsb
    du -k --max-depth=0 /users/peiqi714/test/db/db >> /users/peiqi714/test/ycsb_log/space_util_ycsb
    cp -f /users/peiqi714/test/db/db/LOG /users/peiqi714/test/ycsb_log/blobdb_${wkld}_LOG


    # diffkv
    rm -rf /users/peiqi714/test/db/*
    ./titandb_bench --benchmarks=ycsbfilldb,stats,${wkld},stats  --db=/users/peiqi714/test/db/db --threads=$thread_count \
                    --statistics=true  --num=$entry_count --ycsb_num=$entry_count \
                    --key_size=16 --value_size=$val_size --compression_type=none \
                    --level_compaction_dynamic_level_bytes=true --titan_level_merge=true --titan_disable_background_gc=true \
                    --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 \
                    --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 \
                    --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 \
                    --titan_max_gc_batch_size=268435456 > /users/peiqi714/test/ycsb_log/diffkv_${wkld}
    echo "diffkv_${wkld} space:" >> /users/peiqi714/test/ycsb_log/space_util_ycsb
    du -k --max-depth=0 /users/peiqi714/test/db/db >> /users/peiqi714/test/ycsb_log/space_util_ycsb
    cp -f /users/peiqi714/test/db/db/LOG /users/peiqi714/test/ycsb_log/diffkv_${wkld}_LOG

    # titan
    rm -rf /users/peiqi714/test/db/*
    ./titandb_bench --benchmarks=ycsbfilldb,stats,${wkld},stats  --db=/users/peiqi714/test/db/db --threads=$thread_count \
                    --statistics=true  --num=$entry_count --ycsb_num=$entry_count \
                    --key_size=16 --value_size=$val_size --compression_type=none \
                    --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 \
                    --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 \
                    --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 \
                    --titan_max_gc_batch_size=268435456 > /users/peiqi714/test/ycsb_log/titan_${wkld}
    echo "titan_${wkld} space:" >> /users/peiqi714/test/ycsb_log/space_util_ycsb
    du -k --max-depth=0 /users/peiqi714/test/db/db >> /users/peiqi714/test/ycsb_log/space_util_ycsb
    cp -f /users/peiqi714/test/db/db/LOG /users/peiqi714/test/ycsb_log/titan_${wkld}_LOG

    # shadow
    rm -rf /users/peiqi714/test/db/*
    ./titandb_bench --benchmarks=ycsbfilldb,stats,${wkld},stats  --db=/users/peiqi714/test/db/db --threads=$thread_count \
                    --statistics=true  --num=$entry_count --ycsb_num=$entry_count \
                    --key_size=16 --value_size=$val_size --compression_type=none \
                    --titan_drop_key_bitset=1 --titan_shadow_cache=1 \
                    --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 \
                    --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 \
                    --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 \
                    --titan_max_gc_batch_size=268435456 > /users/peiqi714/test/ycsb_log/shadow_${wkld}
    echo "shadow_${wkld} space:" >> /users/peiqi714/test/ycsb_log/space_util_ycsb
    du -k --max-depth=0 /users/peiqi714/test/db/db >> /users/peiqi714/test/ycsb_log/space_util_ycsb
    cp -f /users/peiqi714/test/db/db/LOG /users/peiqi714/test/ycsb_log/shadow_${wkld}_LOG

    echo "end ${wkld}"

done
echo "end ycsb exp"