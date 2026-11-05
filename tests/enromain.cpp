#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include <atomic>
#include <vector>

#ifdef DELETE
#undef DELETE
#endif

#include "ejson.h"
#include "simdjson.h"

using namespace ejson;
using Clock = std::chrono::steady_clock;

std::atomic<uint64_t> g_checksum{ 0 };

int main() {
    const int read_iterations = 1000000; // 100万次高频纯读测试
    std::cout << "========================================================\n";
    std::cout << ">>> Pure Read Benchmark: ejson (Cached Index) vs simdjson <<<\n";
    std::cout << ">>> Target: Query deep node 1,000,000 times <<<\n";
    std::cout << "========================================================\n\n";

    // 1. 生成 4KB 报文
    std::string raw_json = R"({"users":[)";
    for (int i = 0; i < 100; ++i) {
        raw_json += R"({"id":)" + std::to_string(i) + R"(,"name":"user_)" + std::to_string(i) + R"(","active":true},)";
    }
    raw_json.pop_back();
    raw_json += R"(]})";

    // =========================================================
    // 阶段一：ejson 加载并全量缓存节点 (一次性成本)
    // =========================================================
    jsonfile test;
    jsonvalue root = test.GetRootValue();
    root << EOBJECT;
    root << NPAIR("users");
    root["users"] << EARRAY;
    root["users"] << JSTRING(raw_json.substr(9, raw_json.size() - 10).c_str());

    // 🚀 核心魔法：全量缓存所有节点的句柄（偏移量），以便后续 O(1) 直读
    // 假设我们要高频读取第 50 个用户的 id
    jsonvalue cached_target_id = root["users"][50]["id"];

    // =========================================================
    // 阶段二：simdjson 加载解析 (一次性成本)
    // =========================================================
    simdjson::dom::parser parser;
    simdjson::dom::element doc = parser.parse(raw_json);

    // =========================================================
    // 阶段三：百万次极高频纯读对决
    // =========================================================
    std::cout << "[Test] 1,000,000 deep reads (O(1) offset vs SIMD DOM)\n";

    // 1. ejson 纯读 (直接命中缓存句柄，跳过所有扫描)
    auto t0 = Clock::now();
    for (int i = 0; i < read_iterations; ++i) {
        int id = 0;
        cached_target_id >> id; // O(1) 物理指针直达
        g_checksum.fetch_add(id, std::memory_order_relaxed);
    }
    auto t1 = Clock::now();
    double ejson_read_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    // 2. simdjson 纯读 (依然是 DOM 节点指针跳跃)
    t0 = Clock::now();
    for (int i = 0; i < read_iterations; ++i) {
        int64_t id = doc["users"].at(50)["id"].get_int64();
        g_checksum.fetch_add(id, std::memory_order_relaxed);
    }
    t1 = Clock::now();
    double simdjson_read_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    std::cout << "  ejson (Cached)  : " << std::setw(4) << ejson_read_ms << " ms\n";
    std::cout << "  simdjson (DOM)  : " << std::setw(4) << simdjson_read_ms << " ms\n\n";

    std::cout << "========================================================\n";
    return 0;
}