#include <iostream>
#include <cassert>
#include "titan/db.h"
#include "rocksdb/statistics.h"

int main(int argc, char *argv[])
{
    // if(argc != 3) {
    //     std::cout << "specify property and db path" << std::endl;
    //     return 0;
    // }
        
    // rocksdb::titandb::TitanDB *db;
    // rocksdb::titandb::TitanOptions options;
    // options.statistics = rocksdb::CreateDBStatistics<214,67>();
    // options.create_if_missing = true;
    // rocksdb::Status status =
    //     rocksdb::titandb::TitanDB::Open(options, argv[1], &db);
    // // rocksdb::Status status =
    // //     rocksdb::titandb::TitanDB::Open(options, "/home/data/zhangyuzhan/titan/log_begin/titan50G", &db);
    // assert(status.ok());
  
    // std::string p = argv[2];  // 获取该属性值
    // // std::string p = "rocksdb.titandb.num-live-blob-file-size";  // 获取该属性值
    // std::string v;  // 存储待获取属性值
  
    // db->GetProperty(p, &v);  // 执行 GetProperty() 方法
    // std::cout << v << std::endl;  // 输出结果

    // db->Close();
    // delete db;

    return 0;
}