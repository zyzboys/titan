#include <iostream>
#include <cassert>
#include "rocksdb/db.h"
#include "rocksdb/env.h"
#include "rocksdb/trace_reader_writer.h"
using namespace rocksdb;

int main() {
    Options opt;
    Env *env = Env::Default();
    EnvOptions env_options(opt);
    std::string trace_path = "/home/master/finch/tmp/rocksdb/trace_example";
    std::unique_ptr<TraceWriter> trace_writer;
    DB* db = nullptr;
    std::string db_name = "/home/master/finch/tmp/rocksdb/rocksdb";

    NewFileTraceWriter(env, env_options, trace_path, &trace_writer);
    DB::Open(opt, db_name, &db);

    std::string k1 = "name", k2 = "age";
    std::string v1 = "gukaifeng", v2 = "24";
    TraceOptions trace_opt;
    db->StartTrace(trace_opt, std::move(trace_writer));

    Status s = db->Put(rocksdb::WriteOptions(), k1, v1);
    if (!s.ok()) {
        std::cout << "put failed" << std::endl;
    } else {
        std::cout << "put succeed" << std::endl;
    }
    
    db->EndTrace();


}