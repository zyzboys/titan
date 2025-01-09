#!/usr/bin/env bash
rm -rf /users/peiqi714/test/log/*
rm -rf /users/peiqi714/test/db/*
thread_count=1
echo "start final exp update"
for val_size in 1024
do
    entry_count=50000000
    echo "entry count: $entry_count"

    # rocksdb
    rm -rf /users/peiqi714/test/db/*
    ../rocksdb_6_29/build/db_bench --benchmarks=fillrandom,stats,overwrite,stats,overwrite,stats,overwrite,stats  --db=/users/peiqi714/test/db/db --threads=$thread_count \
                    --statistics=true  --num=$entry_count  --zipfian=1 --zipf_const=0.9 \
                    --key_size=16 --value_size=$val_size --compression_type=none --compression_ratio=1 > /users/peiqi714/test/log/rocksdb_update
    echo "rocksdb_update space:" >> /users/peiqi714/test/log/space_util_update
    du -k --max-depth=0 /users/peiqi714/test/db/db >> /users/peiqi714/test/log/space_util_update
    cp -f /users/peiqi714/test/db/db/LOG /users/peiqi714/test/log/rocksdb_update_LOG

    # blobdb
    rm -rf /users/peiqi714/test/db/*
    ../rocksdb_6_29/build/db_bench --benchmarks=fillrandom,stats,overwrite,stats,overwrite,stats,overwrite,stats  --db=/users/peiqi714/test/db/db --threads=$thread_count \
                    --statistics=true  --num=$entry_count --zipfian=1 --zipf_const=0.9 \
                    --key_size=16 --value_size=$val_size --compression_type=none --compression_ratio=1 \
                    --enable_blob_files=true --enable_blob_garbage_collection=true \
                    --blob_garbage_collection_force_threshold=0.3 --target_file_size_base=4194304 \
                    --max_bytes_for_level_base=16777216 --blob_file_size=67108864 > /users/peiqi714/test/log/blobdb_update
    echo "blobdb_update space:" >> /users/peiqi714/test/log/space_util_update
    du -k --max-depth=0 /users/peiqi714/test/db/db >> /users/peiqi714/test/log/space_util_update
    cp -f /users/peiqi714/test/db/db/LOG /users/peiqi714/test/log/blobdb_update_LOG


    # diffkv
    rm -rf /users/peiqi714/test/db/*
    ./titandb_bench --benchmarks=fillrandom,stats,overwrite,stats,overwrite,stats,overwrite,stats  --db=/users/peiqi714/test/db/db --threads=$thread_count \
                    --statistics=true  --num=$entry_count --zipfian=1 --zipf_const=0.9 \
                    --key_size=16 --value_size=$val_size --compression_type=none \
                    --level_compaction_dynamic_level_bytes=true --titan_level_merge=true --titan_disable_background_gc=true \
                    --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 \
                    --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 \
                    --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 \
                    --titan_max_gc_batch_size=268435456 > /users/peiqi714/test/log/diffkv_update
    echo "diffkv_update space:" >> /users/peiqi714/test/log/space_util_update
    du -k --max-depth=0 /users/peiqi714/test/db/db >> /users/peiqi714/test/log/space_util_update
    cp -f /users/peiqi714/test/db/db/LOG /users/peiqi714/test/log/diffkv_update_LOG

    # titan
    rm -rf /users/peiqi714/test/db/*
    ./titandb_bench --benchmarks=fillrandom,stats,overwrite,stats,overwrite,stats,overwrite,stats  --db=/users/peiqi714/test/db/db --threads=$thread_count \
                    --statistics=true  --num=$entry_count --zipfian=1 --zipf_const=0.9 \
                    --key_size=16 --value_size=$val_size --compression_type=none \
                    --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 \
                    --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 \
                    --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 \
                    --titan_max_gc_batch_size=268435456 > /users/peiqi714/test/log/titan_update
    echo "titan_update space:" >> /users/peiqi714/test/log/space_util_update
    du -k --max-depth=0 /users/peiqi714/test/db/db >> /users/peiqi714/test/log/space_util_update
    cp -f /users/peiqi714/test/db/db/LOG /users/peiqi714/test/log/titan_update_LOG

    # shadow
    rm -rf /users/peiqi714/test/db/*
    ./titandb_bench --benchmarks=fillrandom,stats,overwrite,stats,overwrite,stats,overwrite,stats  --db=/users/peiqi714/test/db/db --threads=$thread_count \
                    --statistics=true  --num=$entry_count --zipfian=1 --zipf_const=0.9 \
                    --key_size=16 --value_size=$val_size --compression_type=none \
                    --titan_drop_key_bitset=1 --titan_shadow_cache=1 \
                    --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 \
                    --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 \
                    --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 \
                    --titan_max_gc_batch_size=268435456 > /users/peiqi714/test/log/shadow_update
    echo "shadow_update space:" >> /users/peiqi714/test/log/space_util_update
    du -k --max-depth=0 /users/peiqi714/test/db/db >> /users/peiqi714/test/log/space_util_update
    cp -f /users/peiqi714/test/db/db/LOG /users/peiqi714/test/log/shadow_update_LOG

done
echo "end final exp update"