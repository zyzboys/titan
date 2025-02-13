#!/usr/bin/env bash
rm -rf /users/peiqi714/test/valsize_log/*
rm -rf /users/peiqi714/test/db/*
thread_count=1
echo "start final exp valsize"
for val_size in 1024 2048 4096 8192 16384
do
    entry_count=$((50000000/(val_size / 1024)))
    echo "entry count: $entry_count"

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

done
echo "end final exp valsize"

rm -rf /users/peiqi714/test/base_log/*
rm -rf /users/peiqi714/test/db/*
thread_count=1
echo "start final exp base"
for val_size in 1024
do
    entry_count=50000000
    read_count=5000000
    echo "entry count: $entry_count"
    echo "read count: $read_count"

    # titan
    rm -rf /users/peiqi714/test/db/*
    ./titandb_bench --benchmarks=fillrandom,stats,overwrite,stats,readrandom,stats,seekrandom,stats  --db=/users/peiqi714/test/db/db --threads=$thread_count \
                    --statistics=true  --num=$entry_count --reads=$read_count --seek_nexts=10 --zipfian=1 --zipf_const=0.9 \
                    --key_size=16 --value_size=$val_size --compression_type=none \
                    --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 \
                    --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 \
                    --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 \
                    --titan_max_gc_batch_size=268435456 > /users/peiqi714/test/base_log/titan_base
    echo "titan_base space:" >> /users/peiqi714/test/base_log/space_util_base
    du -k --max-depth=0 /users/peiqi714/test/db/db >> /users/peiqi714/test/base_log/space_util_base
    cp -f /users/peiqi714/test/db/db/LOG /users/peiqi714/test/base_log/titan_base_LOG
done
echo "end final exp base"

rm -rf /users/peiqi714/test/ycsb_log/*
rm -rf /users/peiqi714/test/db/*
thread_count=1
echo "start ycsb exp"
for wkld in ycsbwkldd ycsbwkldf ycsbwklde ycsbwklda ycsbwkldb ycsbwkldc
do
    entry_count=50000000
    val_size=1024
    echo "entry count: $entry_count"
    echo "value size: $val_size"

    # titan
    rm -rf /users/peiqi714/test/db/*
    ./titandb_bench --benchmarks=ycsbfilldb,stats,ycsbupdate,stats,${wkld},stats  --db=/users/peiqi714/test/db/db --threads=$thread_count \
                    --statistics=true  --num=$entry_count --ycsb_num=$entry_count \
                    --key_size=16 --value_size=$val_size --compression_type=none \
                    --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 \
                    --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 \
                    --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 \
                    --titan_max_gc_batch_size=268435456 > /users/peiqi714/test/ycsb_log/titan_${wkld}
    echo "titan_${wkld} space:" >> /users/peiqi714/test/ycsb_log/space_util_ycsb
    du -k --max-depth=0 /users/peiqi714/test/db/db >> /users/peiqi714/test/ycsb_log/space_util_ycsb
    cp -f /users/peiqi714/test/db/db/LOG /users/peiqi714/test/ycsb_log/titan_${wkld}_LOG

    echo "end ${wkld}"

done
echo "end ycsb exp"