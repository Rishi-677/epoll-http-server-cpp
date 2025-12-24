#pragma once
#include "http_request.h"
#include <sstream>

using namespace std;

class HttpParser {
public:
    static HttpRequest parse(const string& raw) {
        HttpRequest req;
        istringstream stream(raw);
        string line;

        // Request line
        getline(stream, line);
        istringstream lineStream(line);
        lineStream >> req.method >> req.path >> req.version;

        // Headers
        while (getline(stream, line) && line != "\r") {
            size_t pos = line.find(":");
            if (pos != string::npos) {
                string key = line.substr(0, pos);
                string value = line.substr(pos + 1);
                if (!value.empty() && value[0] == ' ')
                    value.erase(0, 1);
                if (!value.empty() && value.back() == '\r')
                    value.pop_back();
                req.headers[key] = value;
            }
        }

        return req;
    }
};
