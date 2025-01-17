#!/usr/bin/env bash
rm -rf /users/peiqi714/test/valsize_log/*
rm -rf /users/peiqi714/test/db/*
thread_count=1
echo "start final exp valsize"
for val_size in 2048 4096 8192 16384
do
    entry_count=$((50000000/(val_size / 1024)))
    echo "entry count: $entry_count"

    # rocksdb
    rm -rf /users/peiqi714/test/db/*
    ../rocksdb_6_29/build/db_bench --benchmarks=fillrandom,stats,overwrite,stats,overwrite,stats,overwrite,stats  --db=/users/peiqi714/test/db/db --threads=$thread_count \
                    --statistics=true  --num=$entry_count  --zipfian=1 --zipf_const=0.9 \
                    --key_size=16 --value_size=$val_size --compression_type=none --compression_ratio=1 > /users/peiqi714/test/valsize_log/rocksdb_valsize${val_size}
    echo "rocksdb_valsize${val_size} space:" >> /users/peiqi714/test/valsize_log/space_util_valsize
    du -k --max-depth=0 /users/peiqi714/test/db/db >> /users/peiqi714/test/valsize_log/space_util_valsize
    cp -f /users/peiqi714/test/db/db/LOG /users/peiqi714/test/valsize_log/rocksdb_valsize${val_size}_LOG

    # blobdb
    rm -rf /users/peiqi714/test/db/*
    ../rocksdb_6_29/build/db_bench --benchmarks=fillrandom,stats,overwrite,stats,overwrite,stats,overwrite,stats  --db=/users/peiqi714/test/db/db --threads=$thread_count \
                    --statistics=true  --num=$entry_count --zipfian=1 --zipf_const=0.9 \
                    --key_size=16 --value_size=$val_size --compression_type=none --compression_ratio=1 \
                    --enable_blob_files=true --enable_blob_garbage_collection=true \
                    --blob_garbage_collection_force_threshold=0.3 --target_file_size_base=4194304 \
                    --max_bytes_for_level_base=16777216 --blob_file_size=67108864 > /users/peiqi714/test/valsize_log/blobdb_valsize${val_size}
    echo "blobdb_valsize${val_size} space:" >> /users/peiqi714/test/valsize_log/space_util_valsize
    du -k --max-depth=0 /users/peiqi714/test/db/db >> /users/peiqi714/test/valsize_log/space_util_valsize
    cp -f /users/peiqi714/test/db/db/LOG /users/peiqi714/test/valsize_log/blobdb_valsize${val_size}_LOG


    # diffkv
    rm -rf /users/peiqi714/test/db/*
    ./titandb_bench --benchmarks=fillrandom,stats,overwrite,stats,overwrite,stats,overwrite,stats  --db=/users/peiqi714/test/db/db --threads=$thread_count \
                    --statistics=true  --num=$entry_count --zipfian=1 --zipf_const=0.9 \
                    --key_size=16 --value_size=$val_size --compression_type=none \
                    --level_compaction_dynamic_level_bytes=true --titan_level_merge=true --titan_disable_background_gc=true \
                    --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 \
                    --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 \
                    --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 \
                    --titan_max_gc_batch_size=268435456 > /users/peiqi714/test/valsize_log/diffkv_valsize${val_size}
    echo "diffkv_valsize${val_size} space:" >> /users/peiqi714/test/valsize_log/space_util_valsize
    du -k --max-depth=0 /users/peiqi714/test/db/db >> /users/peiqi714/test/valsize_log/space_util_valsize
    cp -f /users/peiqi714/test/db/db/LOG /users/peiqi714/test/valsize_log/diffkv_valsize${val_size}_LOG

    # titan
    rm -rf /users/peiqi714/test/db/*
    ./titandb_bench --benchmarks=fillrandom,stats,overwrite,stats,overwrite,stats,overwrite,stats  --db=/users/peiqi714/test/db/db --threads=$thread_count \
                    --statistics=true  --num=$entry_count --zipfian=1 --zipf_const=0.9 \
                    --key_size=16 --value_size=$val_size --compression_type=none \
                    --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 \
                    --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 \
                    --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 \
                    --titan_max_gc_batch_size=268435456 > /users/peiqi714/test/valsize_log/titan_valsize${val_size}
    echo "titan_valsize${val_size} space:" >> /users/peiqi714/test/valsize_log/space_util_valsize
    du -k --max-depth=0 /users/peiqi714/test/db/db >> /users/peiqi714/test/valsize_log/space_util_valsize
    cp -f /users/peiqi714/test/db/db/LOG /users/peiqi714/test/valsize_log/titan_valsize${val_size}_LOG

    # shadow
    rm -rf /users/peiqi714/test/db/*
    ./titandb_bench --benchmarks=fillrandom,stats,overwrite,stats,overwrite,stats,overwrite,stats  --db=/users/peiqi714/test/db/db --threads=$thread_count \
                    --statistics=true  --num=$entry_count --zipfian=1 --zipf_const=0.9 \
                    --key_size=16 --value_size=$val_size --compression_type=none \
                    --titan_drop_key_bitset=1 --titan_shadow_cache=1 \
                    --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 \
                    --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 \
                    --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 \
                    --titan_max_gc_batch_size=268435456 > /users/peiqi714/test/valsize_log/shadow_valsize${val_size}
    echo "shadow_valsize${val_size} space:" >> /users/peiqi714/test/valsize_log/space_util_valsize
    du -k --max-depth=0 /users/peiqi714/test/db/db >> /users/peiqi714/test/valsize_log/space_util_valsize
    cp -f /users/peiqi714/test/db/db/LOG /users/peiqi714/test/valsize_log/shadow_valsize${val_size}_LOG

done
echo "end final exp valsize"