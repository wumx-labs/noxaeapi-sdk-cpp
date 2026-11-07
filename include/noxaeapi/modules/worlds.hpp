#pragma once

#include "../http.hpp"
#include "../types.hpp"

namespace noxaeapi::modules {

class WorldsModule {
private:
    const HttpClient& http_;
public:
    explicit WorldsModule(const HttpClient& http) : http_(http) {}

    std::vector<World> get_all() const {
        auto j = http_.get("/worlds");
        std::vector<World> res;
        for (const auto& item : j.as_array()) res.push_back(World::from_json(item));
        return res;
    }

    World get(const std::string& name) const {
        return World::from_json(http_.get("/worlds/" + name));
    }

    json::Value save(const std::string& name) const {
        return http_.post("/worlds/" + name + "/save");
    }

    json::Value set_time(const std::string& name, int64_t time) const {
        json::Value body;
        body["time"] = time;
        return http_.post("/worlds/" + name + "/time", body);
    }

    json::Value set_weather(const std::string& name, const std::string& weather, int duration = 6000) const {
        json::Value body;
        body["weather"] = weather;
        body["duration"] = duration;
        return http_.post("/worlds/" + name + "/weather", body);
    }
};

} // namespace noxaeapi::modules
