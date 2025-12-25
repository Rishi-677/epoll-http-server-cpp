#pragma once
#include <functional>
#include <vector>
#include "http_request.h"
#include "http_response.h"

using Middleware = std::function<void(HttpRequest&, HttpResponse&)>;

void addMiddleware(const Middleware& mw);
void runMiddlewares(HttpRequest& req, HttpResponse& res);