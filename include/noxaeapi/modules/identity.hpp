#pragma once

#include "../http.hpp"
#include "../types.hpp"

namespace noxaeapi::modules {

class IdentityModule {
private:
    const HttpClient& http_;
public:
    explicit IdentityModule(const HttpClient& http) : http_(http) {}

    PlayerResolveResult resolve(const std::string& name) const {
        return PlayerResolveResult::from_json(http_.get("/identity/resolve", {{"name", name}}));
    }

    std::vector<std::string> get_network_servers() const {
        auto j = http_.get("/identity/network/servers");
        std::vector<std::string> res;
        for (const auto& item : j.as_array()) res.push_back(item.as_string());
        return res;
    }

    json::Value get_network_player(const std::string& uuid) const {
        return http_.get("/identity/network/players/" + uuid);
    }
};

} // namespace noxaeapi::modules
