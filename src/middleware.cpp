#include "middleware.h"

std::vector<Middleware> middlewares;

void addMiddleware(const Middleware& mw) {
    middlewares.push_back(mw);
}

void runMiddlewares(HttpRequest& req, HttpResponse& res) {
    for (auto& mw : middlewares) {
        mw(req, res);
    }
}
