#!/usr/bin/env bash
rm -rf ./space_util
rm -rf /users/peiqi714/test/*
target_size_bytes=$((50 * 1024 * 1024 * 1024))
thread_count=1
echo "start exp1"
for val_size in 256 512 1024 2048 4096
do
    entry_count=$((target_size_bytes / (val_size + 16) / thread_count * thread_count))
    ycsb_entry_count=$((entry_count))
    echo "entry count: $entry_count"
    echo "ycsb entry count: $ycsb_entry_count"

    # titan
    rm -rf /users/peiqi714/test/*
    ./titandb_bench --benchmarks=ycsbfilldb,stats,ycsbwklda,stats  --db=/users/peiqi714/test/db --threads=$thread_count \
                    --statistics=true --ycsb_readwritepercent=0 --num=$entry_count --ycsb_num=$ycsb_entry_count \
                    --key_size=16 --value_size=$val_size --compression_type=none \
                    --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 \
                    --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 \
                    --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 \
                    --titan_max_gc_batch_size=268435456 > titan_ycsb_$val_size
    echo "titan_ycsb_$val_size space:" >> space_util
    du -k --max-depth=0 /users/peiqi714/test/db >> space_util

    # shadow
    rm -rf /users/peiqi714/test/*
    ./titandb_bench --benchmarks=ycsbfilldb,stats,ycsbwklda,stats  --db=/users/peiqi714/test/db --threads=$thread_count \
                    --statistics=true --ycsb_readwritepercent=0 --num=$entry_count --ycsb_num=$ycsb_entry_count \
                    --key_size=16 --value_size=$val_size --compression_type=none \
                    --titan_drop_key_bitset=1 --titan_shadow_cache=1 \
                    --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 \
                    --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 \
                    --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 \
                    --titan_max_gc_batch_size=268435456 > shadow_ycsb_$val_size
    echo "shadow_ycsb_$val_size space:" >> space_util
    du -k --max-depth=0 /users/peiqi714/test/db >> space_util

done
echo "end exp1"

echo "start exp2"
for val_size in 256 512 1024 2048 4096
do
    # titan
    rm -rf /users/peiqi714/test/*
    ./titandb_bench --benchmarks=fillrandom,stats,overwrite,stats  --db=/users/peiqi714/test/db --threads=$thread_count \
                    --statistics=true --zipfian=1 --zipf_const=0.9 --num=50000000 \
                    --key_size=16 --value_size=$val_size --compression_type=none \
                    --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 \
                    --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 \
                    --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 \
                    --titan_max_gc_batch_size=268435456 > titan_overwrite_$val_size
    echo "titan_overwrite_$val_size space:" >> space_util
    du -k --max-depth=0 /users/peiqi714/test/db >> space_util

    # shadow
    rm -rf /users/peiqi714/test/*
    ./titandb_bench --benchmarks=fillrandom,stats,overwrite,stats  --db=/users/peiqi714/test/db --threads=$thread_count \
                    --statistics=true --zipfian=1 --zipf_const=0.9 --num=50000000 \
                    --key_size=16 --value_size=$val_size --compression_type=none \
                    --titan_drop_key_bitset=1 --titan_shadow_cache=1 \
                    --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 \
                    --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 \
                    --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 \
                    --titan_max_gc_batch_size=268435456 > shadow_overwrite_$val_size
    echo "shadow_overwrite_$val_size space:" >> space_util
    du -k --max-depth=0 /users/peiqi714/test/db >> space_util

done
echo "end exp2"