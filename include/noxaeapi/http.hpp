#pragma once

#include <string>
#include <map>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include "types.hpp"

#if defined(NOXAEAPI_USE_CURL)
#include <curl/curl.h>
#endif

namespace noxaeapi {

class ApiException : public std::runtime_error {
public:
    int status_code = 0;
    std::string response_body;

    ApiException(const std::string& msg, int code = 0, std::string body = "")
        : std::runtime_error(msg), status_code(code), response_body(std::move(body)) {}
};

class HttpClient {
private:
    std::string base_url_;
    std::string api_key_;
    std::map<std::string, std::string> custom_headers_;

#if defined(NOXAEAPI_USE_CURL)
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }
#endif

public:
    HttpClient(std::string base_url, std::string api_key)
        : base_url_(std::move(base_url)), api_key_(std::move(api_key)) {
        while (!base_url_.empty() && base_url_.back() == '/') {
            base_url_.pop_back();
        }
    }

    HttpClient with_header(const std::string& name, const std::string& val) const {
        HttpClient copy = *this;
        copy.custom_headers_[name] = val;
        return copy;
    }

    std::string request(const std::string& method, const std::string& path,
                        const std::map<std::string, std::string>& query = {},
                        const std::string& body = "") const {
        std::string url = base_url_ + (path.empty() || path[0] == '/' ? path : "/" + path);
        if (!query.empty()) {
            url += "?";
            bool first = true;
            for (const auto& kv : query) {
                if (!first) url += "&";
                url += kv.first + "=" + kv.second;
                first = false;
            }
        }

#if defined(NOXAEAPI_USE_CURL)
        CURL* curl = curl_easy_init();
        if (!curl) throw ApiException("Failed to initialize libcurl");

        std::string response_string;
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("X-API-Key: " + api_key_).c_str());
        headers = curl_slist_append(headers, "Accept: application/json");
        headers = curl_slist_append(headers, "Content-Type: application/json");

        for (const auto& h : custom_headers_) {
            headers = curl_slist_append(headers, (h.first + ": " + h.second).c_str());
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);

        if (method == "POST") {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            if (!body.empty()) curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        } else if (method == "PUT") {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
            if (!body.empty()) curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        } else if (method == "DELETE") {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        }

        CURLcode res = curl_easy_perform(curl);
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            throw ApiException(curl_easy_strerror(res), 0);
        }

        if (http_code >= 400) {
            throw ApiException("HTTP " + std::to_string(http_code) + " error", static_cast<int>(http_code), response_string);
        }

        return response_string;
#else
        // Mock fallback if compiled without libcurl
        return "{}";
#endif
    }

    json::Value get(const std::string& path, const std::map<std::string, std::string>& query = {}) const {
        return json::parse(request("GET", path, query));
    }

    json::Value post(const std::string& path, const json::Value& body = json::Value(), const std::map<std::string, std::string>& query = {}) const {
        return json::parse(request("POST", path, query, body.is_null() ? "" : body.dump()));
    }

    json::Value put(const std::string& path, const json::Value& body = json::Value(), const std::map<std::string, std::string>& query = {}) const {
        return json::parse(request("PUT", path, query, body.is_null() ? "" : body.dump()));
    }

    json::Value del(const std::string& path, const std::map<std::string, std::string>& query = {}) const {
        return json::parse(request("DELETE", path, query));
    }
};

} // namespace noxaeapi
