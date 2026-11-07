#pragma once

#include "../http.hpp"

namespace noxaeapi::modules {

class MiscModule {
private:
    const HttpClient& http_;
public:
    explicit MiscModule(const HttpClient& http) : http_(http) {}

    json::Value ping() const {
        return http_.get("/ping");
    }

    json::Value health() const {
        return http_.get("/health");
    }
};

} // namespace noxaeapi::modules
