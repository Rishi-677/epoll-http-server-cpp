#include "middleware_setup.h"
#include "middleware.h"

#include <chrono>
#include <iostream>
#include <unordered_map>
#include <mutex>

using namespace std;

// -------------------- RATE LIMIT DATA --------------------
struct RateLimitInfo
{
    int count;
    chrono::steady_clock::time_point window_start;
};

static unordered_map<string, RateLimitInfo> rateLimitMap;
static mutex rateLimitMutex;

static const int MAX_REQUESTS = 5;
static const int WINDOW_SECONDS = 10;

// -------------------- REGISTER ALL MIDDLEWARES --------------------
void registerMiddlewares()
{

    // 🔹 Logging + timing middleware
    addMiddleware([](HttpRequest &req, HttpResponse &)
                  {
        auto now = chrono::high_resolution_clock::now();
        req.headers["__start_time"] =
            to_string(chrono::duration_cast<chrono::microseconds>(
                now.time_since_epoch()).count());

        cout << "[REQ] " << req.method << " " << req.path << endl; });

    // 🔹 Rate limiting middleware
    addMiddleware([](HttpRequest &req, HttpResponse &res)
                  {
        string client = req.client_ip;  // simple identifier

        auto now = chrono::steady_clock::now();

        lock_guard<mutex> lock(rateLimitMutex);
        auto& info = rateLimitMap[client];

        if (info.count == 0) {
            info.window_start = now;
            info.count = 1;
            return;
        }

        auto elapsed = chrono::duration_cast<chrono::seconds>(
            now - info.window_start).count();

        if (elapsed > WINDOW_SECONDS) {
            info.window_start = now;
            info.count = 1;
            return;
        }

        info.count++;

        if (info.count > MAX_REQUESTS) {
            res.status = 429;
            res.statusText = "Too Many Requests";
            res.headers["Content-Type"] = "text/plain";
            res.body = "Rate limit exceeded. Try again later.\n";
        } });
}