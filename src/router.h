#pragma once
#include "http_request.h"
#include "http_response.h"
#include <unordered_map>
#include <functional>

using namespace std;

class Router {
public:
    using Handler = function<void(const HttpRequest&, HttpResponse&)>;

    void get(const string& path, Handler handler) {
        routes["GET:" + path] = handler;
    }

    bool route(const HttpRequest& req, HttpResponse& res) {
        string key = req.method + ":" + req.path;
        if (routes.count(key)) {
            routes[key](req, res);
            return true;
        }
        return false;
    }

private:
    unordered_map<string, Handler> routes;
};
