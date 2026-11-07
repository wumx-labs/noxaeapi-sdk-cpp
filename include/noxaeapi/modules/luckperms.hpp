#pragma once

#include "../http.hpp"
#include "../types.hpp"

namespace noxaeapi::modules {

class LuckPermsModule {
private:
    const HttpClient& http_;
public:
    explicit LuckPermsModule(const HttpClient& http) : http_(http) {}

    std::vector<std::string> get_groups() const {
        auto j = http_.get("/luckperms/groups");
        std::vector<std::string> res;
        for (const auto& item : j.as_array()) res.push_back(item.as_string());
        return res;
    }

    GroupInfo get_group(const std::string& name) const {
        return GroupInfo::from_json(http_.get("/luckperms/groups/" + name));
    }

    json::Value create_group(const std::string& name) const {
        json::Value body;
        body["name"] = name;
        return http_.post("/luckperms/groups", body);
    }

    json::Value delete_group(const std::string& name) const {
        return http_.del("/luckperms/groups/" + name);
    }

    std::vector<std::string> get_user_permissions(const std::string& uuid) const {
        auto j = http_.get("/luckperms/users/" + uuid + "/permissions");
        std::vector<std::string> res;
        for (const auto& item : j.as_array()) res.push_back(item.as_string());
        return res;
    }

    json::Value add_user_permission(const std::string& uuid, const std::string& perm, bool val = true, int64_t expiry = 0) const {
        json::Value body;
        body["permission"] = perm;
        body["value"] = val;
        body["expiry"] = expiry;
        return http_.post("/luckperms/users/" + uuid + "/permissions", body);
    }

    json::Value remove_user_permission(const std::string& uuid, const std::string& perm) const {
        return http_.del("/luckperms/users/" + uuid + "/permissions", {{"permission", perm}});
    }
};

} // namespace noxaeapi::modules
