#pragma once
#include <string>
#include <unordered_map>

using namespace std;

struct HttpRequest {
    string method;
    string path;
    string version;
    unordered_map<string, string> headers;
    string body;
};
