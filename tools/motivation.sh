#!/usr/bin/env bash
rm -rf ./space_util
rm -rf /users/peiqi714/test/*
target_size_bytes=$((50 * 1024 * 1024 * 1024))
thread_count=8
echo "start exp1"
for val_size in 256 512 1024 2048 4096
do
    entry_count=$((target_size_bytes / (val_size + 16) / thread_count * thread_count))
    ycsb_entry_count=$((entry_count * 2))
    echo "entry count: $entry_count"
    echo "ycsb entry count: $ycsb_entry_count"
    
    # titan no gc
    rm -rf /users/peiqi714/test/*
    ./titandb_bench --benchmarks=ycsbfilldb,stats,ycsbwklda,stats  --db=/users/peiqi714/test/db --threads=8 \
                    --statistics=true --ycsb_zipf_const=0.99 --ycsb_readwritepercent=0 --num=$entry_count --ycsb_num=$ycsb_entry_count \
                    --key_size=16 --value_size=$val_size --compression_type=none --titan_disable_background_gc=true \
                    --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 \
                    --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 \
                    --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 \
                    --titan_max_gc_batch_size=268435456 > TitanNoGC_0.9_$val_size
    echo "TitanNoGC_0.9_$val_size space:" >> space_util
    du -k --max-depth=0 /users/peiqi714/test/db >> space_util

    # diffkv
    rm -rf /users/peiqi714/test/*
    ./titandb_bench --benchmarks=ycsbfilldb,stats,ycsbwklda,stats  --db=/users/peiqi714/test/db --threads=8 \
                    --statistics=true --ycsb_zipf_const=0.99 --ycsb_readwritepercent=0 --num=$entry_count --ycsb_num=$ycsb_entry_count \
                    --key_size=16 --value_size=$val_size --compression_type=none \
                    --level_compaction_dynamic_level_bytes=true --titan_level_merge=true --titan_disable_background_gc=true \
                    --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 \
                    --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 \
                    --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 \
                    --titan_max_gc_batch_size=268435456 > diffkv_0.9_$val_size
    echo "diffkv_0.9_$val_size space:" >> space_util
    du -k --max-depth=0 /users/peiqi714/test/db >> space_util

    # titan
    rm -rf /users/peiqi714/test/*
    ./titandb_bench --benchmarks=ycsbfilldb,stats,ycsbwklda,stats  --db=/users/peiqi714/test/db --threads=8 \
                    --statistics=true --ycsb_zipf_const=0.99 --ycsb_readwritepercent=0 --num=$entry_count --ycsb_num=$ycsb_entry_count \
                    --key_size=16 --value_size=$val_size --compression_type=none \
                    --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 \
                    --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 \
                    --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 \
                    --titan_max_gc_batch_size=268435456 > titan_0.9_$val_size
    echo "titan_0.9_$val_size space:" >> space_util
    du -k --max-depth=0 /users/peiqi714/test/db >> space_util

    # bitset
    rm -rf /users/peiqi714/test/*
    ./titandb_bench --benchmarks=ycsbfilldb,stats,ycsbwklda,stats  --db=/users/peiqi714/test/db --threads=8 \
                    --statistics=true --ycsb_zipf_const=0.99 --ycsb_readwritepercent=0 --num=$entry_count --ycsb_num=$ycsb_entry_count \
                    --key_size=16 --value_size=$val_size --compression_type=none \
                    --titan_drop_key_bitset=1 \
                    --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 \
                    --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 \
                    --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 \
                    --titan_max_gc_batch_size=268435456 > bitset_0.9_$val_size
    echo "bitset_0.9_$val_size space:" >> space_util
    du -k --max-depth=0 /users/peiqi714/test/db >> space_util

    # shadow
    rm -rf /users/peiqi714/test/*
    ./titandb_bench --benchmarks=ycsbfilldb,stats,ycsbwklda,stats  --db=/users/peiqi714/test/db --threads=8 \
                    --statistics=true --ycsb_zipf_const=0.99 --ycsb_readwritepercent=0 --num=$entry_count --ycsb_num=$ycsb_entry_count \
                    --key_size=16 --value_size=$val_size --compression_type=none \
                    --titan_drop_key_bitset=1 --titan_shadow_cache=1 \
                    --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 \
                    --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 \
                    --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 \
                    --titan_max_gc_batch_size=268435456 > shadow_0.9_$val_size
    echo "shadow_0.9_$val_size space:" >> space_util
    du -k --max-depth=0 /users/peiqi714/test/db >> space_util

    # rocksdb
    rm -rf /users/peiqi714/test/*
    ../rocksdb_6_29/build/db_bench --benchmarks=ycsbfilldb,stats,ycsbwklda,stats  --db=/users/peiqi714/test/db --threads=8 \
                    --statistics=true --ycsb_zipf_const=0.99 --ycsb_readwritepercent=0 --num=$entry_count --ycsb_num=$ycsb_entry_count\
                    --key_size=16 --value_size=$val_size --compression_type=none > rocksdb_0.9_$val_size
    echo "rocksdb_0.9_$val_size space:" >> space_util
    du -k --max-depth=0 /users/peiqi714/test/db >> space_util

    # blobdb
    rm -rf /users/peiqi714/test/*
    ../rocksdb_6_29/build/db_bench --benchmarks=ycsbfilldb,stats,ycsbwklda,stats  --db=/users/peiqi714/test/db --threads=8 \
                    --statistics=true --ycsb_zipf_const=0.99 --ycsb_readwritepercent=0 --num=$entry_count --ycsb_num=$ycsb_entry_count \
                    --key_size=16 --value_size=$val_size --compression_type=none \
                    --enable_blob_files=true --enable_blob_garbage_collection=true \
                    --blob_garbage_collection_force_threshold=0.3 --target_file_size_base=4194304 \
                    --max_bytes_for_level_base=16777216 --blob_file_size=67108864 > blobdb_0.9_$val_size
    echo "blobdb_0.9_$val_size space:" >> space_util
    du -k --max-depth=0 /users/peiqi714/test/db >> space_util

done
echo "end exp1"

# for skewness in 0 0.3 0.6 0.9
# do  
#     entry_count=$((target_size_bytes / (1024 + 16)))
#     echo "entry count: $entry_count"
#     # rocksdb
#     rm -rf /users/peiqi714/test/*
#     ../rocksdb_6_29/build/db_bench --benchmarks=ycsbfilldb,stats,ycsbwklda,stats  --db=/users/peiqi714/test/db --threads=8 \
#                     --statistics=true --ycsb_zipf_const=$skewness --ycsb_readwritepercent=0 --num=$entry_count \
#                     --key_size=16 --value_size=1024 --compression_type=none > rocksdb_1024_$skewness
#     echo "rocksdb_1024_$skewness space:" >> space_util
#     du -k --max-depth=0 /users/peiqi714/test/db >> space_util

#     # blobdb
#     rm -rf /users/peiqi714/test/*
#     ../rocksdb_6_29/build/db_bench --benchmarks=ycsbfilldb,stats,ycsbwklda,stats  --db=/users/peiqi714/test/db --threads=8 \
#                     --statistics=true --ycsb_zipf_const=$skewness --ycsb_readwritepercent=0 --num=$entry_count \
#                     --key_size=16 --value_size=1024 --compression_type=none \
#                     --enable_blob_files=true --enable_blob_garbage_collection=true \
#                     --blob_garbage_collection_force_threshold=0.3 --target_file_size_base=4194304 \
#                     --max_bytes_for_level_base=16777216 --blob_file_size=67108864 > blobdb_1024_$skewness
#     echo "blobdb_1024_$skewness space:" >> space_util
#     du -k --max-depth=0 /users/peiqi714/test/db >> space_util

#     # titan
#     rm -rf /users/peiqi714/test/*
#     ./titandb_bench --benchmarks=ycsbfilldb,stats,ycsbwklda,stats  --db=/users/peiqi714/test/db --threads=8 \
#                     --statistics=true --ycsb_zipf_const=$skewness --ycsb_readwritepercent=0 --num=$entry_count \
#                     --key_size=16 --value_size=1024 --compression_type=none \
#                     --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 \
#                     --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 \
#                     --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 \
#                     --titan_max_gc_batch_size=268435456 > titan_1024_$skewness
#     echo "titan_1024_$skewness space:" >> space_util
#     du -k --max-depth=0 /users/peiqi714/test/db >> space_util

#     # titan no gc
#     rm -rf /users/peiqi714/test/*
#     ./titandb_bench --benchmarks=ycsbfilldb,stats,ycsbwklda,stats  --db=/users/peiqi714/test/db --threads=8 \
#                     --statistics=true --ycsb_zipf_const=$skewness --ycsb_readwritepercent=0 --num=$entry_count \
#                     --key_size=16 --value_size=1024 --compression_type=none --titan_disable_background_gc=true \
#                     --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 \
#                     --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 \
#                     --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 \
#                     --titan_max_gc_batch_size=268435456 > TitanNoGC_1024_$skewness
#     echo "TitanNoGC_1024_$skewness space:" >> space_util
#     du -k --max-depth=0 /users/peiqi714/test/db >> space_util

#     # diffkv
#     rm -rf /users/peiqi714/test/*
#     ./titandb_bench --benchmarks=ycsbfilldb,stats,ycsbwklda,stats  --db=/users/peiqi714/test/db --threads=8 \
#                     --statistics=true --ycsb_zipf_const=$skewness --ycsb_readwritepercent=0 --num=$entry_count \
#                     --key_size=16 --value_size=1024 --compression_type=none \
#                     --level_compaction_dynamic_level_bytes=true --titan_level_merge=true --titan_disable_background_gc=true \
#                     --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 \
#                     --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 \
#                     --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 \
#                     --titan_max_gc_batch_size=268435456 > diffkv_1024_$skewness
#     echo "diffkv_1024_$skewness space:" >> space_util
#     du -k --max-depth=0 /users/peiqi714/test/db >> space_util

# done
# echo "end exp2"