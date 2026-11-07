#pragma once

#include "../http.hpp"
#include "rankforge.hpp"

namespace noxaeapi::modules {

class IntegrationsModule {
private:
    const HttpClient& http_;
public:
    RankForgeModule rankforge;

    explicit IntegrationsModule(const HttpClient& http)
        : http_(http), rankforge(http) {}

    json::Value get(const std::string& plugin, const std::string& endpoint, const std::map<std::string, std::string>& query = {}) const {
        return http_.get("/integrations/" + plugin + "/" + endpoint, query);
    }

    json::Value post(const std::string& plugin, const std::string& endpoint, const json::Value& body = json::Value()) const {
        return http_.post("/integrations/" + plugin + "/" + endpoint, body);
    }
};

} // namespace noxaeapi::modules
