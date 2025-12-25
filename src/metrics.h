#pragma once
#include <atomic>

extern std::atomic<long> total_requests;
extern std::atomic<long> active_connections;
extern std::atomic<long long> total_latency_us;

void record_request(long latency_us);
