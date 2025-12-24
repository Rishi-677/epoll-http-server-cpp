#pragma once
#include <string>
#include <unordered_map>

using namespace std;

struct HttpResponse {
    int status = 200;
    string statusText = "OK";
    unordered_map<string, string> headers;
    string body;

    string toString() {
        string res = "HTTP/1.1 " + to_string(status) + " " + statusText + "\r\n";
        headers["Content-Length"] = to_string(body.size());

        for (auto& h : headers) {
            res += h.first + ": " + h.second + "\r\n";
        }
        res += "\r\n";
        res += body;
        return res;
    }
};
