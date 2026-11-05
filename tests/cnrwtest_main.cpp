#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include <atomic>
#include <vector>

// 1. 先包 Windows 底层头
#define NOMINMAX
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")

// 2. 暴力清理 Windows 污染的宏
#ifdef DELETE
#undef DELETE
#endif
#ifdef ERROR
#undef ERROR
#endif
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

// 3. 再包你自己的库和 RapidJSON
#include "ejson.h"
#include "json.hpp"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace ejson;

#pragma comment(lib, "psapi.lib")

using namespace ejson;
using Clock = std::chrono::steady_clock;

std::atomic<uint64_t> g_checksum{ 0 };

double GetPhysicalMemoryMB() {
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize / 1024.0 / 1024.0;
    }
    return 0.0;
}

int main() {
    const int iter_parse = 10000;
    const int iter_build = 10000;

    std::cout << "====================================================\n";
    std::cout << ">>> JSON 库全面对比测试 (ejson vs nlohmann vs RapidJSON) <<<\n";
    std::cout << "====================================================\n\n";

    // 生成 4KB 报文
    std::string raw_json = R"({"users":[)";
    std::string batched_users = "[";
    for (int i = 0; i < 100; ++i) {
        std::string item = R"({"id":)" + std::to_string(i) + R"(,"name":"user_)" + std::to_string(i) + R"(","active":true})";
        raw_json += item + ",";
        batched_users += item;
        if (i < 99) batched_users += ",";
    }
    raw_json.pop_back();
    raw_json += R"(]})";
    batched_users += "]";

    // =========================================================
    // 测试 1：解析并查询
    // =========================================================
    std::cout << "[测试 1] 解析 4KB 报文并查询深层节点 (10000次)\n";

    // 1.1 ejson
    auto t0 = Clock::now();
    for (int i = 0; i < iter_parse; ++i) {
        jsonfile test;
        jsonvalue root = test.GetRootValue();
        root << EOBJECT;
        root << NPAIR("users");
        root["users"] << EARRAY;
        root["users"] << JSTRING(batched_users.c_str());

        int id = 0;
        root["users"][50]["id"] >> id;
        g_checksum.fetch_add(id);
    }
    auto t1 = Clock::now();
    double ejson_parse = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    // 1.2 nlohmann
    t0 = Clock::now();
    for (int i = 0; i < iter_parse; ++i) {
        auto j = nlohmann::json::parse(raw_json);
        int id = j["users"][50]["id"];
        g_checksum.fetch_add(id);
    }
    t1 = Clock::now();
    double nlohmann_parse = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    // 1.3 RapidJSON
    t0 = Clock::now();
    for (int i = 0; i < iter_parse; ++i) {
        rapidjson::Document doc;
        doc.Parse(raw_json.c_str());
        int id = doc["users"][50]["id"].GetInt();
        g_checksum.fetch_add(id);
    }
    t1 = Clock::now();
    double rapidjson_parse = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    std::cout << "  ejson 解析耗时: " << ejson_parse << " ms\n";
    std::cout << "  nlohmann 解析耗时: " << nlohmann_parse << " ms\n";
    std::cout << "  RapidJSON 解析耗时: " << rapidjson_parse << " ms\n\n";

    // =========================================================
    // 测试 2：从零构建 100 个元素
    // =========================================================
    std::cout << "[测试 2] 从零构建 100 个元素的数组 (10000次)\n";

    // 2.1 ejson
    t0 = Clock::now();
    for (int i = 0; i < iter_build; ++i) {
        jsonfile test;
        jsonvalue root = test.GetRootValue();
        root << EOBJECT;
        root << NPAIR("users");
        root["users"] << EARRAY;
        root["users"] << JSTRING(batched_users.c_str());
    }
    t1 = Clock::now();
    double ejson_build = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    // 2.2 nlohmann
    t0 = Clock::now();
    for (int i = 0; i < iter_build; ++i) {
        nlohmann::json j;
        j["users"] = nlohmann::json::array();
        for (int k = 0; k < 100; ++k) {
            nlohmann::json user;
            user["id"] = k;
            user["name"] = "user_" + std::to_string(k);
            user["active"] = true;
            j["users"].push_back(user);
        }
        std::string out = j.dump();
    }
    t1 = Clock::now();
    double nlohmann_build = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    // 2.3 RapidJSON (构建比较复杂，这里用 Writer)
    t0 = Clock::now();
    for (int i = 0; i < iter_build; ++i) {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        writer.StartObject();
        writer.Key("users");
        writer.StartArray();
        for (int k = 0; k < 100; ++k) {
            writer.StartObject();
            writer.Key("id"); writer.Int(k);
            writer.Key("name"); writer.String("user_");
            writer.Key("active"); writer.Bool(true);
            writer.EndObject();
        }
        writer.EndArray();
        writer.EndObject();
        std::string out(buffer.GetString(), buffer.GetSize());
    }
    t1 = Clock::now();
    double rapidjson_build = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    std::cout << "  ejson 构建耗时: " << ejson_build << " ms\n";
    std::cout << "  nlohmann 构建耗时: " << nlohmann_build << " ms\n";
    std::cout << "  RapidJSON 构建耗时: " << rapidjson_build << " ms\n\n";

    // =========================================================
    // 测试 3：内存开销
    // =========================================================
    std::cout << "[测试 3] 构建 1 万次后的额外物理内存占用\n";

    // ejson 内存
    auto warmup_start = Clock::now();
    while (std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - warmup_start).count() < 300) {}
    double start = GetPhysicalMemoryMB();
    for (int i = 0; i < iter_build; ++i) {
        jsonfile test;
        jsonvalue root = test.GetRootValue();
        root << EOBJECT; root << NPAIR("users"); root["users"] << EARRAY;
        root["users"] << JSTRING(batched_users.c_str());
    }
    std::cout << "  ejson 内存消耗: " << std::fixed << std::setprecision(2) << (GetPhysicalMemoryMB() - start) << " MB\n";

    // nlohmann 内存
    warmup_start = Clock::now();
    while (std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - warmup_start).count() < 300) {}
    start = GetPhysicalMemoryMB();
    for (int i = 0; i < iter_build; ++i) {
        nlohmann::json j;
        j["users"] = nlohmann::json::array();
        for (int k = 0; k < 100; ++k) { j["users"].push_back({ {"id", k}, {"name", "u"}, {"active", true} }); }
        j.dump();
    }
    std::cout << "  nlohmann 内存消耗: " << std::fixed << std::setprecision(2) << (GetPhysicalMemoryMB() - start) << " MB\n";

    std::cout << "====================================================\n";
    return 0;
}