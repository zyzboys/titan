#include <iostream>
#include <cassert>
#include "rocksdb/db.h"

int main(int argc, char *argv[])
{
    if(argc != 3) {
        std::cout << "specify property and db path" << std::endl;
        return 0;
    }
        
    rocksdb::DB *db;
    rocksdb::Options options;
    options.create_if_missing = true;
    rocksdb::Status status =
        rocksdb::DB::Open(options, argv[1], &db);
    // rocksdb::Status status =
    //     rocksdb::DB::Open(options, "/home/master/finch/tmp/rocksdb/rocksdb", &db);
    assert(status.ok());
  
    std::string p = argv[2];  // 获取该属性值
    // std::string p = "rocksdb.stats";  // 获取该属性值
    std::string v;  // 存储待获取属性值
  
    db->GetProperty(p, &v);  // 执行 GetProperty() 方法
    std::cout << v << std::endl;  // 输出结果

    db->Close();
    delete db;

    return 0;
}