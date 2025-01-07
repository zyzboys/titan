#!/usr/bin/env bash
rm -rf /home/zhangyuzhan/test/log/*
rm -rf /home/zhangyuzhan/test/db/*
thread_count=1
echo "start final exp"
for val_size in 1024
do
    entry_count=10000000
    read_count=1000000
    echo "entry count: $entry_count"
    echo "read count: $read_count"

    # rocksdb
    rm -rf /home/zhangyuzhan/test/db/*
    ../rocksdb_6_29/rocksdb/db_bench --benchmarks=fillrandom,stats,overwrite,stats,readrandom,stats,seekrandom,stats  --db=/home/zhangyuzhan/test/db/db --threads=$thread_count \
                    --statistics=true  --num=$entry_count --reads=$read_count --seek_nexts=10 --zipfian=1 --zipf_const=0.9 \
                    --key_size=16 --value_size=$val_size --compression_type=none --compression_ratio=1 > /home/zhangyuzhan/test/log/rocksdb_base
    echo "rocksdb_base space:" >> /home/zhangyuzhan/test/log/space_util_base
    du -k --max-depth=0 /home/zhangyuzhan/test/db/db >> /home/zhangyuzhan/test/log/space_util_base
    cp -f /home/zhangyuzhan/test/db/db/LOG /home/zhangyuzhan/test/log/rocksdb_base_LOG

    # blobdb
    rm -rf /home/zhangyuzhan/test/db/*
    ../rocksdb_6_29/rocksdb/db_bench --benchmarks=fillrandom,stats,overwrite,stats,readrandom,stats,seekrandom,stats  --db=/home/zhangyuzhan/test/db/db --threads=$thread_count \
                    --statistics=true  --num=$entry_count --reads=$read_count --seek_nexts=10 --zipfian=1 --zipf_const=0.9 \
                    --key_size=16 --value_size=$val_size --compression_type=none --compression_ratio=1 \
                    --enable_blob_files=true --enable_blob_garbage_collection=true \
                    --blob_garbage_collection_force_threshold=0.3 --target_file_size_base=4194304 \
                    --max_bytes_for_level_base=16777216 --blob_file_size=67108864 > /home/zhangyuzhan/test/log/blobdb_base
    echo "blobdb_base space:" >> /home/zhangyuzhan/test/log/space_util_base
    du -k --max-depth=0 /home/zhangyuzhan/test/db/db >> /home/zhangyuzhan/test/log/space_util_base
    cp -f /home/zhangyuzhan/test/db/db/LOG /home/zhangyuzhan/test/log/blobdb_base_LOG


    # diffkv
    rm -rf /home/zhangyuzhan/test/db/*
    ./titandb_bench --benchmarks=fillrandom,stats,overwrite,stats,readrandom,stats,seekrandom,stats  --db=/home/zhangyuzhan/test/db/db --threads=$thread_count \
                    --statistics=true  --num=$entry_count --reads=$read_count --seek_nexts=10 --zipfian=1 --zipf_const=0.9 \
                    --key_size=16 --value_size=$val_size --compression_type=none \
                    --level_compaction_dynamic_level_bytes=true --titan_level_merge=true --titan_disable_background_gc=true \
                    --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 \
                    --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 \
                    --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 \
                    --titan_max_gc_batch_size=268435456 > /home/zhangyuzhan/test/log/diffkv_base
    echo "diffkv_base space:" >> /home/zhangyuzhan/test/log/space_util_base
    du -k --max-depth=0 /home/zhangyuzhan/test/db/db >> /home/zhangyuzhan/test/log/space_util_base
    cp -f /home/zhangyuzhan/test/db/db/LOG /home/zhangyuzhan/test/log/diffkv_base_LOG

    # titan
    rm -rf /home/zhangyuzhan/test/db/*
    ./titandb_bench --benchmarks=fillrandom,stats,overwrite,stats,readrandom,stats,seekrandom,stats  --db=/home/zhangyuzhan/test/db/db --threads=$thread_count \
                    --statistics=true  --num=$entry_count --reads=$read_count --seek_nexts=10 --zipfian=1 --zipf_const=0.9 \
                    --key_size=16 --value_size=$val_size --compression_type=none \
                    --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 \
                    --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 \
                    --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 \
                    --titan_max_gc_batch_size=268435456 > /home/zhangyuzhan/test/log/titan_base
    echo "titan_base space:" >> /home/zhangyuzhan/test/log/space_util_base
    du -k --max-depth=0 /home/zhangyuzhan/test/db/db >> /home/zhangyuzhan/test/log/space_util_base
    cp -f /home/zhangyuzhan/test/db/db/LOG /home/zhangyuzhan/test/log/titan_base_LOG

    # shadow
    rm -rf /home/zhangyuzhan/test/db/*
    ./titandb_bench --benchmarks=fillrandom,stats,overwrite,stats,readrandom,stats,seekrandom,stats  --db=/home/zhangyuzhan/test/db/db --threads=$thread_count \
                    --statistics=true  --num=$entry_count --reads=$read_count --seek_nexts=10 --zipfian=1 --zipf_const=0.9 \
                    --key_size=16 --value_size=$val_size --compression_type=none \
                    --titan_drop_key_bitset=1 --titan_shadow_cache=1 \
                    --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 \
                    --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 \
                    --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 \
                    --titan_max_gc_batch_size=268435456 > /home/zhangyuzhan/test/log/shadow_base
    echo "shadow_base space:" >> /home/zhangyuzhan/test/log/space_util_base
    du -k --max-depth=0 /home/zhangyuzhan/test/db/db >> /home/zhangyuzhan/test/log/space_util_base
    cp -f /home/zhangyuzhan/test/db/db/LOG /home/zhangyuzhan/test/log/shadow_base_LOG

done
echo "end final exp"