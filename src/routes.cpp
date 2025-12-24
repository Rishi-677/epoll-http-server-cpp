#include "routes.h"
#include <chrono>

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
}
