#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include <atomic>
#include <vector>
#define NOMINMAX
#include <windows.h>
#include <psapi.h>

#ifdef DELETE
#undef DELETE
#endif

#include "ejson.h"
#include "json.hpp"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

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

    std::cout << "========================================================\n";
    std::cout << ">>> JSON Libraries Comprehensive Benchmark <<<\n";
    std::cout << ">>> [ejson] vs [nlohmann/json] vs [RapidJSON] <<<\n";
    std::cout << "========================================================\n\n";

    // Generate a 4KB JSON payload
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
    // Test 1: Parsing & Querying
    // =========================================================
    std::cout << "[Test 1] Parse 4KB payload & query deep node (" << iter_parse << " iterations)\n";

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

    std::cout << "  ejson     : " << std::setw(4) << ejson_parse << " ms\n";
    std::cout << "  nlohmann  : " << std::setw(4) << nlohmann_parse << " ms\n";
    std::cout << "  RapidJSON : " << std::setw(4) << rapidjson_parse << " ms\n\n";

    // =========================================================
    // Test 2: Building from scratch
    // =========================================================
    std::cout << "[Test 2] Build 100-element array from scratch (" << iter_build << " iterations)\n";

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

    // 2.3 RapidJSON
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

    std::cout << "  ejson     : " << std::setw(4) << ejson_build << " ms\n";
    std::cout << "  nlohmann  : " << std::setw(4) << nlohmann_build << " ms\n";
    std::cout << "  RapidJSON : " << std::setw(4) << rapidjson_build << " ms\n\n";

    // =========================================================
    // Test 3: Memory Overhead
    // =========================================================
    std::cout << "[Test 3] Extra physical memory allocation after " << iter_build << " builds\n";

    // ejson memory
    auto warmup_start = Clock::now();
    while (std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - warmup_start).count() < 300) {}
    double start = GetPhysicalMemoryMB();
    for (int i = 0; i < iter_build; ++i) {
        jsonfile test;
        jsonvalue root = test.GetRootValue();
        root << EOBJECT; root << NPAIR("users"); root["users"] << EARRAY;
        root["users"] << JSTRING(batched_users.c_str());
    }
    std::cout << "  ejson     : " << std::fixed << std::setprecision(2) << (GetPhysicalMemoryMB() - start) << " MB\n";

    // nlohmann memory
    warmup_start = Clock::now();
    while (std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - warmup_start).count() < 300) {}
    start = GetPhysicalMemoryMB();
    for (int i = 0; i < iter_build; ++i) {
        nlohmann::json j;
        j["users"] = nlohmann::json::array();
        for (int k = 0; k < 100; ++k) { j["users"].push_back({ {"id", k}, {"name", "u"}, {"active", true} }); }
        j.dump();
    }
    std::cout << "  nlohmann  : " << std::fixed << std::setprecision(2) << (GetPhysicalMemoryMB() - start) << " MB\n";

    std::cout << "========================================================\n";
    return 0;
}