target_size_bytes=$((50 * 1024 * 1024 * 1024))
echo "start exp1"
for val_size in 512 1024 2048 4096
do
    entry_count=$((target_size_bytes / (val_size + 16)))
    echo "entry count: $entry_count"

done

for skewness in 0 0.3 0.6 0.9
do  
    entry_count=$((target_size_bytes / (1024 + 16)))
    echo "entry count: $entry_count"

done