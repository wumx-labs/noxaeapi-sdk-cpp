#pragma once

#include "../http.hpp"
#include "../types.hpp"

namespace noxaeapi::modules {

class EconomyModule {
private:
    const HttpClient& http_;
public:
    explicit EconomyModule(const HttpClient& http) : http_(http) {}

    EconomyInfo get_info() const {
        return EconomyInfo::from_json(http_.get("/economy"));
    }

    double get_balance(const std::string& uuid) const {
        auto j = http_.get("/economy/balance/" + uuid);
        return j.is_number() ? j.as_double() : j["balance"].as_double();
    }

    json::Value set_balance(const std::string& uuid, double amount) const {
        json::Value body;
        body["amount"] = amount;
        return http_.post("/economy/balance/" + uuid, body);
    }

    json::Value add_balance(const std::string& uuid, double amount) const {
        json::Value body;
        body["amount"] = amount;
        return http_.post("/economy/balance/" + uuid + "/add", body);
    }

    json::Value remove_balance(const std::string& uuid, double amount) const {
        json::Value body;
        body["amount"] = amount;
        return http_.post("/economy/balance/" + uuid + "/remove", body);
    }

    std::vector<TopBalanceEntry> get_top(int limit = 10, int offset = 0) const {
        auto j = http_.get("/economy/top", {{"limit", std::to_string(limit)}, {"offset", std::to_string(offset)}});
        std::vector<TopBalanceEntry> res;
        for (const auto& item : j.as_array()) res.push_back(TopBalanceEntry::from_json(item));
        return res;
    }

    CurrencyBalance get_currency_balance(const std::string& uuid, const std::string& currency) const {
        return CurrencyBalance::from_json(http_.get("/economy/" + currency + "/balance/" + uuid));
    }

    std::vector<CurrencyTopEntry> get_currency_top(const std::string& currency, int limit = 10, int offset = 0) const {
        auto j = http_.get("/economy/" + currency + "/top", {{"limit", std::to_string(limit)}, {"offset", std::to_string(offset)}});
        std::vector<CurrencyTopEntry> res;
        for (const auto& item : j.as_array()) res.push_back(CurrencyTopEntry::from_json(item));
        return res;
    }
};

} // namespace noxaeapi::modules
