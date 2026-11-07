#pragma once

#include "../http.hpp"
#include "../types.hpp"

namespace noxaeapi::modules {

class LeaderboardModule {
private:
    const HttpClient& http_;
public:
    explicit LeaderboardModule(const HttpClient& http) : http_(http) {}

    std::vector<LeaderboardSourceInfo> get_sources() const {
        auto j = http_.get("/leaderboard/sources");
        std::vector<LeaderboardSourceInfo> res;
        for (const auto& item : j.as_array()) res.push_back(LeaderboardSourceInfo::from_json(item));
        return res;
    }

    json::Value get(const std::string& source, int limit = 10, int offset = 0) const {
        return http_.get("/leaderboard/" + source, {{"limit", std::to_string(limit)}, {"offset", std::to_string(offset)}});
    }
};

} // namespace noxaeapi::modules
