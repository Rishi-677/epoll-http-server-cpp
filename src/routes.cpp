#include "routes.h"
#include <chrono>
#include "metrics.h"

using namespace std;

void registerRoutes(Router &router)
{

    router.get("/", [](const HttpRequest &, HttpResponse &res)
               {
        res.headers["Content-Type"] = "text/plain";
        res.body = "Welcome to the C++ HTTP Server!"; });

    router.get("/hello", [](const HttpRequest &, HttpResponse &res)
               {
        res.headers["Content-Type"] = "text/plain";
        res.body = "Hello from /hello route"; });

    router.get("/time", [](const HttpRequest &, HttpResponse &res)
               {
        res.headers["Content-Type"] = "text/plain";

        auto now = chrono::system_clock::now();
        auto t = chrono::system_clock::to_time_t(now);

        res.body = string("Current time: ") + ctime(&t); });

    router.get("/metrics", [](const HttpRequest &, HttpResponse &res)
               {
    res.headers["Content-Type"] = "text/plain";

    long total = total_requests.load();

    long active = active_connections.load();
    long long total_lat = total_latency_us.load();

    long avg = (total > 0) ? (total_lat / total) : 0;

    res.body =
        "total_requests: " + std::to_string(total) + "\n" +
        "active_connections: " + std::to_string(active) + "\n" +
        "avg_latency_us: " + std::to_string(avg) + "\n"; });
}
