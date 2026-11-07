#pragma once

#include "../http.hpp"
#include "../types.hpp"

namespace noxaeapi::modules {

class NoxAuthModule {
private:
    const HttpClient& http_;
public:
    explicit NoxAuthModule(const HttpClient& http) : http_(http) {}

    NoxAuthPlayerInfo get_player(const std::string& uuid) const {
        return NoxAuthPlayerInfo::from_json(http_.get("/noxauth/players/" + uuid));
    }

    bool check_password(const std::string& name, const std::string& password) const {
        json::Value body;
        body["name"] = name;
        body["password"] = password;
        auto j = http_.post("/noxauth/check-password", body);
        return j.is_bool() ? j.as_bool() : j["valid"].as_bool();
    }

    json::Value set_password(const std::string& uuid, const std::string& password) const {
        json::Value body;
        body["password"] = password;
        return http_.post("/noxauth/players/" + uuid + "/password", body);
    }

    json::Value register_player(const std::string& name, const std::string& password, const std::string& email = "") const {
        json::Value body;
        body["name"] = name;
        body["password"] = password;
        if (!email.empty()) body["email"] = email;
        return http_.post("/noxauth/register", body);
    }

    json::Value unregister(const std::string& uuid) const {
        return http_.del("/noxauth/players/" + uuid);
    }
};

} // namespace noxaeapi::modules
