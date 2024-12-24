rm -rf ./space_util
for input in 30000000 50000000 70000000
do
echo "input size: $input"
rm -rf /home/zhangyuzhan/test/db/*
./titandb_bench --benchmarks=fillrandom,stats,overwrite,stats  --db=/home/zhangyuzhan/test/db/db --statistics=true --zipfian=1 --zipf_const=0.9 --num=$input --key_size=16 --value_size=1024 --compression_type=none --compression_ratio=1 --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 --titan_max_gc_batch_size=268435456 > base_overwrite$input
echo "base_overwrite$input space:" >> space_util
du -k --max-depth=0 /home/zhangyuzhan/test/db/db >> space_util
rm -rf /home/zhangyuzhan/test/db/*
./titandb_bench --benchmarks=ycsbfilldb,stats,ycsbwklda,stats  --db=/home/zhangyuzhan/test/db/db --statistics=true --zipfian=1 --zipf_const=0.9 --num=$input --key_size=16 --value_size=1024 --compression_type=none --compression_ratio=1 --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 --titan_max_gc_batch_size=268435456 > base_ycsba$input
echo "base_ycsba$input space:" >> space_util
du -k --max-depth=0 /home/zhangyuzhan/test/db/db >> space_util
rm -rf /home/zhangyuzhan/test/db/*
./titandb_bench --benchmarks=fillrandom,stats,readrandom,stats  --db=/home/zhangyuzhan/test/db/db --statistics=true --zipfian=1 --zipf_const=0.9 --num=$input --key_size=16 --value_size=1024 --compression_type=none --compression_ratio=1 --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 --titan_max_gc_batch_size=268435456 > base_readrandom$input
echo "base_readrandom$input space:" >> space_util
du -k --max-depth=0 /home/zhangyuzhan/test/db/db >> space_util

rm -rf /home/zhangyuzhan/test/db/*
./titandb_bench --benchmarks=fillrandom,stats,overwrite,stats  --db=/home/zhangyuzhan/test/db/db --titan_drop_key_bitset=1 --statistics=true --zipfian=1 --zipf_const=0.9 --num=$input --key_size=16 --value_size=1024 --compression_type=none --compression_ratio=1 --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 --titan_max_gc_batch_size=268435456 > bitset_overwrite$input
echo "bitset_overwrite$input space:" >> space_util
du -k --max-depth=0 /home/zhangyuzhan/test/db/db >> space_util
rm -rf /home/zhangyuzhan/test/db/*
./titandb_bench --benchmarks=ycsbfilldb,stats,ycsbwklda,stats  --db=/home/zhangyuzhan/test/db/db --titan_drop_key_bitset=1 --statistics=true --zipfian=1 --zipf_const=0.9 --num=$input --key_size=16 --value_size=1024 --compression_type=none --compression_ratio=1 --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 --titan_max_gc_batch_size=268435456 > bitset_ycsba$input
echo "bitset_ycsba$input space:" >> space_util
du -k --max-depth=0 /home/zhangyuzhan/test/db/db >> space_util
rm -rf /home/zhangyuzhan/test/db/*
./titandb_bench --benchmarks=fillrandom,stats,readrandom,stats  --db=/home/zhangyuzhan/test/db/db --titan_drop_key_bitset=1 --statistics=true --zipfian=1 --zipf_const=0.9 --num=$input --key_size=16 --value_size=1024 --compression_type=none --compression_ratio=1 --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 --titan_max_gc_batch_size=268435456 > bitset_readrandom$input
echo "bitset_readrandom$input space:" >> space_util
du -k --max-depth=0 /home/zhangyuzhan/test/db/db >> space_util

rm -rf /home/zhangyuzhan/test/db/*
./titandb_bench --benchmarks=fillrandom,stats,overwrite,stats  --db=/home/zhangyuzhan/test/db/db --titan_drop_key_bitset=1 --titan_shadow_cache=1 --statistics=true --zipfian=1 --zipf_const=0.9 --num=$input --key_size=16 --value_size=1024 --compression_type=none --compression_ratio=1 --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 --titan_max_gc_batch_size=268435456 > shadow_overwrite$input
echo "shadow_overwrite$input space:" >> space_util
du -k --max-depth=0 /home/zhangyuzhan/test/db/db >> space_util
rm -rf /home/zhangyuzhan/test/db/*
./titandb_bench --benchmarks=ycsbfilldb,stats,ycsbwklda,stats  --db=/home/zhangyuzhan/test/db/db --titan_drop_key_bitset=1 --titan_shadow_cache=1 --statistics=true --zipfian=1 --zipf_const=0.9 --num=$input --key_size=16 --value_size=1024 --compression_type=none --compression_ratio=1 --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 --titan_max_gc_batch_size=268435456 > shadow_ycsba$input
echo "shadow_ycsba$input space:" >> space_util
du -k --max-depth=0 /home/zhangyuzhan/test/db/db >> space_util
rm -rf /home/zhangyuzhan/test/db/*
./titandb_bench --benchmarks=fillrandom,stats,readrandom,stats  --db=/home/zhangyuzhan/test/db/db --titan_drop_key_bitset=1 --titan_shadow_cache=1 --statistics=true --zipfian=1 --zipf_const=0.9 --num=$input --key_size=16 --value_size=1024 --compression_type=none --compression_ratio=1 --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 --titan_max_gc_batch_size=268435456 > shadow_readrandom$input
echo "shadow_random$input space:" >> space_util
du -k --max-depth=0 /home/zhangyuzhan/test/db/db >> space_util
done
