#pragma once

#include "../http.hpp"
#include "../types.hpp"

namespace noxaeapi::modules {

class PlayersModule {
private:
    const HttpClient& http_;
public:
    explicit PlayersModule(const HttpClient& http) : http_(http) {}

    std::vector<OnlinePlayer> get_online() const {
        auto j = http_.get("/players");
        std::vector<OnlinePlayer> res;
        for (const auto& item : j.as_array()) res.push_back(OnlinePlayer::from_json(item));
        return res;
    }

    OnlinePlayer get(const std::string& uuid) const {
        return OnlinePlayer::from_json(http_.get("/players/" + uuid));
    }

    OfflinePlayer get_offline(const std::string& uuid) const {
        return OfflinePlayer::from_json(http_.get("/players/" + uuid + "/offline"));
    }

    json::Value get_inventory(const std::string& uuid) const {
        return http_.get("/players/" + uuid + "/inventory");
    }

    json::Value get_ender_chest(const std::string& uuid) const {
        return http_.get("/players/" + uuid + "/enderchest");
    }

    PlayerStats get_stats(const std::string& uuid) const {
        return PlayerStats::from_json(http_.get("/players/" + uuid + "/stats"));
    }

    json::Value kick(const std::string& uuid, const std::string& reason = "Kicked") const {
        json::Value body;
        body["reason"] = reason;
        return http_.post("/players/" + uuid + "/kick", body);
    }

    json::Value ban(const std::string& uuid, const std::string& reason = "Banned") const {
        json::Value body;
        body["reason"] = reason;
        return http_.post("/players/" + uuid + "/ban", body);
    }

    json::Value unban(const std::string& uuid) const {
        return http_.post("/players/" + uuid + "/unban");
    }

    json::Value set_whitelist(const std::string& uuid, bool whitelisted) const {
        json::Value body;
        body["whitelisted"] = whitelisted;
        return http_.post("/players/" + uuid + "/whitelist", body);
    }

    json::Value teleport(const std::string& uuid, double x, double y, double z) const {
        json::Value body;
        body["x"] = x; body["y"] = y; body["z"] = z;
        return http_.post("/players/" + uuid + "/teleport", body);
    }

    json::Value send_message(const std::string& uuid, const std::string& msg) const {
        json::Value body;
        body["message"] = msg;
        return http_.post("/players/" + uuid + "/message", body);
    }
};

} // namespace noxaeapi::modules
