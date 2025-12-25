#include "metrics.h"

std::atomic<long> total_requests{0};
std::atomic<long> active_connections{0};
std::atomic<long long> total_latency_us{0};

void record_request(long latency_us) {
    total_requests++;
    total_latency_us += latency_us;
}
