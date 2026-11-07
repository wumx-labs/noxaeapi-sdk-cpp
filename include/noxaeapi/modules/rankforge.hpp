#pragma once

#include "../http.hpp"
#include "../types.hpp"

namespace noxaeapi::modules {

class RankForgeModule {
private:
    const HttpClient& http_;
public:
    explicit RankForgeModule(const HttpClient& http) : http_(http) {}

    json::Value get_player_stats(const std::string& uuid) const {
        return http_.get("/integrations/rankforge/players/" + uuid);
    }

    int get_points(const std::string& uuid) const {
        auto j = http_.get("/integrations/rankforge/players/" + uuid + "/points");
        return j.is_number() ? j.as_int() : j["points"].as_int();
    }

    json::Value set_points(const std::string& uuid, int points) const {
        json::Value body;
        body["points"] = points;
        return http_.post("/integrations/rankforge/players/" + uuid + "/points", body);
    }

    json::Value add_points(const std::string& uuid, int points) const {
        json::Value body;
        body["points"] = points;
        return http_.post("/integrations/rankforge/players/" + uuid + "/points/add", body);
    }

    json::Value remove_points(const std::string& uuid, int points) const {
        json::Value body;
        body["points"] = points;
        return http_.post("/integrations/rankforge/players/" + uuid + "/points/remove", body);
    }

    std::vector<std::string> get_tiers() const {
        auto j = http_.get("/integrations/rankforge/tiers");
        std::vector<std::string> res;
        for (const auto& item : j.as_array()) res.push_back(item.as_string());
        return res;
    }

    json::Value get_top(int limit = 10, int offset = 0) const {
        return http_.get("/integrations/rankforge/top", {{"limit", std::to_string(limit)}, {"offset", std::to_string(offset)}});
    }

    json::Value save() const {
        return http_.post("/integrations/rankforge/save");
    }
};

} // namespace noxaeapi::modules
