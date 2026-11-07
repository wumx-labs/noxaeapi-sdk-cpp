#pragma once

#include "../http.hpp"
#include "../types.hpp"

namespace noxaeapi::modules {

class ServerModule {
private:
    const HttpClient& http_;
public:
    explicit ServerModule(const HttpClient& http) : http_(http) {}

    ServerInfo get_info() const {
        return ServerInfo::from_json(http_.get("/server"));
    }

    ServerHealth get_health() const {
        return ServerHealth::from_json(http_.get("/server/health"));
    }

    std::string get_tps() const {
        auto j = http_.get("/server/tps");
        return j.is_string() ? j.as_string() : j["tps"].as_string();
    }

    std::vector<Plugin> get_plugins() const {
        auto j = http_.get("/server/plugins");
        std::vector<Plugin> res;
        for (const auto& item : j.as_array()) res.push_back(Plugin::from_json(item));
        return res;
    }

    std::vector<World> get_worlds() const {
        auto j = http_.get("/server/worlds");
        std::vector<World> res;
        for (const auto& item : j.as_array()) res.push_back(World::from_json(item));
        return res;
    }

    json::Value execute_command(const std::string& command) const {
        json::Value body;
        body["command"] = command;
        return http_.post("/server/command", body);
    }

    json::Value broadcast(const std::string& message) const {
        json::Value body;
        body["message"] = message;
        return http_.post("/server/broadcast", body);
    }
};

} // namespace noxaeapi::modules
