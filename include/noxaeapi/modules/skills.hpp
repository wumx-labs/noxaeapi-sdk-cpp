#pragma once

#include "../http.hpp"
#include "../types.hpp"

namespace noxaeapi::modules {

class SkillsModule {
private:
    const HttpClient& http_;
public:
    explicit SkillsModule(const HttpClient& http) : http_(http) {}

    SkillInfo get_player_skills(const std::string& uuid) const {
        return SkillInfo::from_json(http_.get("/skills/players/" + uuid));
    }

    json::Value add_exp(const std::string& uuid, const std::string& skill, double exp) const {
        json::Value body;
        body["exp"] = exp;
        return http_.post("/skills/players/" + uuid + "/" + skill + "/add-exp", body);
    }

    json::Value set_level(const std::string& uuid, const std::string& skill, int level) const {
        json::Value body;
        body["level"] = level;
        return http_.post("/skills/players/" + uuid + "/" + skill + "/set-level", body);
    }
};

} // namespace noxaeapi::modules
