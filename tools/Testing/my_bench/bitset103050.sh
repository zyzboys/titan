cd /home/master/finch/src/titan/build/
./titandb_bench --benchmarks=fillrandom,stats,overwrite,stats  --db=/home/data/zhangyuzhan/test/db/bitset/bitset10 --statistics=true --zipfian=1 --zipfian_const=0.9 --num=10000000 --key_size=16 --value_size=1024 --compression_type=none --compression_ratio=1 --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 --titan_max_gc_batch_size=268435456 > /home/data/zhangyuzhan/test/log/bitset/bitset10
echo "bitset10 space utilization: " >> /home/data/zhangyuzhan/test/log/bitset/space_utilization
du -k --max-depth=0 /home/data/zhangyuzhan/test/db/bitset/bitset10 >> /home/data/zhangyuzhan/test/log/bitset/space_utilization
cp /home/data/zhangyuzhan/test/db/bitset/bitset10/LOG /home/data/zhangyuzhan/test/log/bitset/bitset10_LOG
rm -rf /home/data/zhangyuzhan/test/db/bitset/bitset10
echo "10 done"

cd /home/master/finch/src/titan/build/
./titandb_bench --benchmarks=fillrandom,stats,overwrite,stats  --db=/home/data/zhangyuzhan/test/db/bitset/bitset30 --statistics=true --zipfian=1 --zipfian_const=0.9 --num=30000000 --key_size=16 --value_size=1024 --compression_type=none --compression_ratio=1 --titan_blob_file_discardable_ratio=0.3 --target_file_size_base=4194304 --max_bytes_for_level_base=16777216 --titan_blob_file_target_size=67108864 --titan_merge_small_file_threshold=2097152 --titan_min_gc_batch_size=134217728 --titan_max_gc_batch_size=268435456 > /home/data/zhangyuzhan/test/log/bitset/bitset30
echo "bitset30 space utilization: " >> /home/data/zhangyuzhan/test/log/bitset/space_utilization
du -k --max-depth=0 /home/data/zhangyuzhan/test/db/bitset/bitset30 >> /home/data/zhangyuzhan/test/log/bitset/space_utilization
cp /home/data/zhangyuzhan/test/db/bitset/bitset30/LOG /home/data/zhangyuzhan/test/log/bitset/bitset30_LOG
rm -rf /home/data/zhangyuzhan/test/db/bitset/bitset30
echo "30 done"




