#pragma once

#include "../http.hpp"
#include "../types.hpp"

namespace noxaeapi::modules {

class PluginsModule {
private:
    const HttpClient& http_;
public:
    explicit PluginsModule(const HttpClient& http) : http_(http) {}

    std::vector<Plugin> get_all() const {
        auto j = http_.get("/plugins");
        std::vector<Plugin> res;
        for (const auto& item : j.as_array()) res.push_back(Plugin::from_json(item));
        return res;
    }

    Plugin get(const std::string& name) const {
        return Plugin::from_json(http_.get("/plugins/" + name));
    }

    json::Value enable(const std::string& name) const {
        return http_.post("/plugins/" + name + "/enable");
    }

    json::Value disable(const std::string& name) const {
        return http_.post("/plugins/" + name + "/disable");
    }
};

} // namespace noxaeapi::modules
