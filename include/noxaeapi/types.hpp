#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <optional>
#include <cstdint>
#include <memory>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <utility>
#include <stdexcept>
#include <cctype>

namespace noxaeapi {

// ============================================================================
// Lightweight Zero-Dependency JSON Implementation
// ============================================================================
namespace json {

enum class Type {
    Null,
    Boolean,
    Number,
    String,
    Array,
    Object
};

class Value {
public:
    Type type = Type::Null;
    bool bool_val = false;
    double num_val = 0.0;
    std::string str_val;
    std::vector<Value> arr_val;
    std::map<std::string, Value> obj_val;

    Value() : type(Type::Null) {}
    Value(std::nullptr_t) : type(Type::Null) {}
    Value(bool b) : type(Type::Boolean), bool_val(b) {}
    Value(int v) : type(Type::Number), num_val(v) {}
    Value(unsigned int v) : type(Type::Number), num_val(v) {}
    Value(int64_t v) : type(Type::Number), num_val(static_cast<double>(v)) {}
    Value(uint64_t v) : type(Type::Number), num_val(static_cast<double>(v)) {}
    Value(double v) : type(Type::Number), num_val(v) {}
    Value(const char* s) : type(Type::String), str_val(s ? s : "") {}
    Value(std::string s) : type(Type::String), str_val(std::move(s)) {}
    Value(std::vector<Value> arr) : type(Type::Array), arr_val(std::move(arr)) {}
    Value(std::map<std::string, Value> obj) : type(Type::Object), obj_val(std::move(obj)) {}

    bool is_null() const noexcept { return type == Type::Null; }
    bool is_bool() const noexcept { return type == Type::Boolean; }
    bool is_number() const noexcept { return type == Type::Number; }
    bool is_string() const noexcept { return type == Type::String; }
    bool is_array() const noexcept { return type == Type::Array; }
    bool is_object() const noexcept { return type == Type::Object; }

    bool as_bool(bool def = false) const {
        return is_bool() ? bool_val : def;
    }

    int64_t as_int64(int64_t def = 0) const {
        return is_number() ? static_cast<int64_t>(num_val) : def;
    }

    int as_int(int def = 0) const {
        return is_number() ? static_cast<int>(num_val) : def;
    }

    double as_double(double def = 0.0) const {
        return is_number() ? num_val : def;
    }

    std::string as_string(const std::string& def = "") const {
        return is_string() ? str_val : def;
    }

    const std::vector<Value>& as_array() const {
        static const std::vector<Value> empty_arr;
        return is_array() ? arr_val : empty_arr;
    }

    const std::map<std::string, Value>& as_object() const {
        static const std::map<std::string, Value> empty_obj;
        return is_object() ? obj_val : empty_obj;
    }

    bool contains(const std::string& key) const {
        if (!is_object()) return false;
        return obj_val.find(key) != obj_val.end();
    }

    const Value& get(const std::string& key) const {
        static const Value null_val;
        if (!is_object()) return null_val;
        auto it = obj_val.find(key);
        if (it != obj_val.end()) return it->second;
        return null_val;
    }

    const Value& operator[](const std::string& key) const {
        return get(key);
    }

    Value& operator[](const std::string& key) {
        if (type != Type::Object) {
            type = Type::Object;
            obj_val.clear();
        }
        return obj_val[key];
    }

    const Value& operator[](size_t index) const {
        static const Value null_val;
        if (!is_array() || index >= arr_val.size()) return null_val;
        return arr_val[index];
    }

    Value& operator[](size_t index) {
        if (type != Type::Array) {
            type = Type::Array;
            arr_val.clear();
        }
        if (index >= arr_val.size()) {
            arr_val.resize(index + 1);
        }
        return arr_val[index];
    }

    void push_back(Value v) {
        if (type != Type::Array) {
            type = Type::Array;
            arr_val.clear();
        }
        arr_val.push_back(std::move(v));
    }

    std::string dump() const {
        std::ostringstream ss;
        serialize(ss);
        return ss.str();
    }

private:
    void serialize(std::ostringstream& ss) const {
        switch (type) {
            case Type::Null: ss << "null"; break;
            case Type::Boolean: ss << (bool_val ? "true" : "false"); break;
            case Type::Number: {
                if (num_val == static_cast<int64_t>(num_val)) {
                    ss << static_cast<int64_t>(num_val);
                } else {
                    ss << std::setprecision(14) << num_val;
                }
                break;
            }
            case Type::String: {
                ss << '"';
                for (char c : str_val) {
                    if (c == '"') ss << "\\\"";
                    else if (c == '\\') ss << "\\\\";
                    else if (c == '\b') ss << "\\b";
                    else if (c == '\f') ss << "\\f";
                    else if (c == '\n') ss << "\\n";
                    else if (c == '\r') ss << "\\r";
                    else if (c == '\t') ss << "\\t";
                    else ss << c;
                }
                ss << '"';
                break;
            }
            case Type::Array: {
                ss << '[';
                for (size_t i = 0; i < arr_val.size(); ++i) {
                    if (i > 0) ss << ',';
                    arr_val[i].serialize(ss);
                }
                ss << ']';
                break;
            }
            case Type::Object: {
                ss << '{';
                size_t i = 0;
                for (const auto& kv : obj_val) {
                    if (i++ > 0) ss << ',';
                    ss << '"' << kv.first << "\":";
                    kv.second.serialize(ss);
                }
                ss << '}';
                break;
            }
        }
    }
};

class Parser {
public:
    static Value parse(const std::string& str) {
        size_t idx = 0;
        skip_ws(str, idx);
        if (idx >= str.size()) return Value();
        return parse_val(str, idx);
    }

private:
    static void skip_ws(const std::string& s, size_t& i) {
        while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r')) {
            ++i;
        }
    }

    static Value parse_val(const std::string& s, size_t& i) {
        skip_ws(s, i);
        if (i >= s.size()) return Value();
        char c = s[i];
        if (c == 'n') return parse_null(s, i);
        if (c == 't' || c == 'f') return parse_bool(s, i);
        if (c == '"') return parse_string(s, i);
        if (c == '[') return parse_array(s, i);
        if (c == '{') return parse_object(s, i);
        if (c == '-' || (c >= '0' && c <= '9')) return parse_number(s, i);
        return Value();
    }

    static Value parse_null(const std::string& s, size_t& i) {
        if (s.substr(i, 4) == "null") { i += 4; return Value(); }
        return Value();
    }

    static Value parse_bool(const std::string& s, size_t& i) {
        if (s.substr(i, 4) == "true") { i += 4; return Value(true); }
        if (s.substr(i, 5) == "false") { i += 5; return Value(false); }
        return Value(false);
    }

    static Value parse_string(const std::string& s, size_t& i) {
        ++i; // skip '"'
        std::string res;
        while (i < s.size()) {
            char c = s[i++];
            if (c == '"') break;
            if (c == '\\' && i < s.size()) {
                char esc = s[i++];
                if (esc == '"') res.push_back('"');
                else if (esc == '\\') res.push_back('\\');
                else if (esc == '/') res.push_back('/');
                else if (esc == 'b') res.push_back('\b');
                else if (esc == 'f') res.push_back('\f');
                else if (esc == 'n') res.push_back('\n');
                else if (esc == 'r') res.push_back('\r');
                else if (esc == 't') res.push_back('\t');
                else res.push_back(esc);
            } else {
                res.push_back(c);
            }
        }
        return Value(res);
    }

    static Value parse_number(const std::string& s, size_t& i) {
        size_t start = i;
        if (s[i] == '-') ++i;
        while (i < s.size() && ((s[i] >= '0' && s[i] <= '9') || s[i] == '.' || s[i] == 'e' || s[i] == 'E' || s[i] == '+' || s[i] == '-')) {
            ++i;
        }
        try {
            double v = std::stod(s.substr(start, i - start));
            return Value(v);
        } catch (...) {
            return Value(0.0);
        }
    }

    static Value parse_array(const std::string& s, size_t& i) {
        ++i; // skip '['
        std::vector<Value> arr;
        skip_ws(s, i);
        if (i < s.size() && s[i] == ']') {
            ++i;
            return Value(arr);
        }
        while (i < s.size()) {
            arr.push_back(parse_val(s, i));
            skip_ws(s, i);
            if (i < s.size() && s[i] == ',') {
                ++i;
                skip_ws(s, i);
            } else if (i < s.size() && s[i] == ']') {
                ++i;
                break;
            } else {
                break;
            }
        }
        return Value(arr);
    }

    static Value parse_object(const std::string& s, size_t& i) {
        ++i; // skip '{'
        std::map<std::string, Value> obj;
        skip_ws(s, i);
        if (i < s.size() && s[i] == '}') {
            ++i;
            return Value(obj);
        }
        while (i < s.size()) {
            skip_ws(s, i);
            if (i >= s.size() || s[i] != '"') break;
            Value key = parse_string(s, i);
            skip_ws(s, i);
            if (i < s.size() && s[i] == ':') ++i;
            Value val = parse_val(s, i);
            obj[key.as_string()] = std::move(val);
            skip_ws(s, i);
            if (i < s.size() && s[i] == ',') {
                ++i;
                skip_ws(s, i);
            } else if (i < s.size() && s[i] == '}') {
                ++i;
                break;
            } else {
                break;
            }
        }
        return Value(obj);
    }
};

inline Value parse(const std::string& str) {
    return Parser::parse(str);
}

} // namespace json

// ============================================================================
// Core Model Structures
// ============================================================================

struct Location {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    std::optional<double> yaw;
    std::optional<double> pitch;
    std::optional<std::string> world;
};

struct OnlinePlayer {
    std::string uuid;
    std::string display_name;
    std::optional<std::string> address;
    std::optional<uint32_t> port;
    double exhaustion = 0.0;
    double exp = 0.0;
    int64_t exp_level = 0;
    bool whitelisted = false;
    bool banned = false;
    bool op = false;
    std::optional<double> balance;
    std::optional<std::vector<double>> location;
    std::optional<std::string> dimension;
    double health = 20.0;
    int64_t hunger = 20;
    double saturation = 5.0;
    std::string gamemode;
    int64_t last_played = 0;
    std::optional<bool> authenticated;
    std::optional<bool> registered;

    static OnlinePlayer from_json(const json::Value& j) {
        OnlinePlayer p;
        p.uuid = j["uuid"].as_string();
        p.display_name = j["displayName"].as_string();
        if (j.contains("address") && !j["address"].is_null()) p.address = j["address"].as_string();
        if (j.contains("port") && !j["port"].is_null()) p.port = static_cast<uint32_t>(j["port"].as_int());
        p.exhaustion = j["exhaustion"].as_double();
        p.exp = j["exp"].as_double();
        p.exp_level = j["expLevel"].as_int64();
        p.whitelisted = j["whitelisted"].as_bool();
        p.banned = j["banned"].as_bool();
        p.op = j["op"].as_bool();
        if (j.contains("balance") && !j["balance"].is_null()) p.balance = j["balance"].as_double();
        if (j.contains("location") && j["location"].is_array()) {
            std::vector<double> loc;
            for (const auto& el : j["location"].as_array()) loc.push_back(el.as_double());
            p.location = loc;
        }
        if (j.contains("dimension") && !j["dimension"].is_null()) p.dimension = j["dimension"].as_string();
        p.health = j["health"].as_double();
        p.hunger = j["hunger"].as_int64();
        p.saturation = j["saturation"].as_double();
        p.gamemode = j["gamemode"].as_string();
        p.last_played = j["lastPlayed"].as_int64();
        if (j.contains("authenticated") && !j["authenticated"].is_null()) p.authenticated = j["authenticated"].as_bool();
        if (j.contains("registered") && !j["registered"].is_null()) p.registered = j["registered"].as_bool();
        return p;
    }
};

struct OfflinePlayer {
    std::string uuid;
    std::string display_name;
    bool whitelisted = false;
    bool banned = false;
    bool op = false;
    std::optional<double> balance;
    int64_t last_played = 0;

    static OfflinePlayer from_json(const json::Value& j) {
        OfflinePlayer p;
        p.uuid = j["uuid"].as_string();
        p.display_name = j["displayName"].as_string();
        p.whitelisted = j["whitelisted"].as_bool();
        p.banned = j["banned"].as_bool();
        p.op = j["op"].as_bool();
        if (j.contains("balance") && !j["balance"].is_null()) p.balance = j["balance"].as_double();
        p.last_played = j["lastPlayed"].as_int64();
        return p;
    }
};

struct PlayerResolveResult {
    std::string name;
    std::string uuid;

    static PlayerResolveResult from_json(const json::Value& j) {
        PlayerResolveResult r;
        r.name = j["name"].as_string();
        r.uuid = j["uuid"].as_string();
        return r;
    }
};

struct InventoryItem {
    std::string id;
    int64_t count = 1;
    int64_t slot = 0;

    static InventoryItem from_json(const json::Value& j) {
        InventoryItem item;
        item.id = j["id"].as_string();
        item.count = j["count"].as_int64(1);
        item.slot = j["slot"].as_int64(0);
        return item;
    }
};

struct PlayerStats {
    std::string uuid;
    std::string name;
    int64_t kills = 0;
    int64_t deaths = 0;
    int64_t playtime = 0;
    int64_t blocks_placed = 0;
    int64_t blocks_broken = 0;

    static PlayerStats from_json(const json::Value& j) {
        PlayerStats s;
        s.uuid = j["uuid"].as_string();
        s.name = j["name"].as_string();
        s.kills = j["kills"].as_int64();
        s.deaths = j["deaths"].as_int64();
        s.playtime = j["playtime"].as_int64();
        s.blocks_placed = j["blocksPlaced"].as_int64();
        s.blocks_broken = j["blocksBroken"].as_int64();
        return s;
    }
};

struct ServerHealth {
    int64_t cpus = 0;
    int64_t uptime = 0;
    int64_t total_memory = 0;
    int64_t max_memory = 0;
    int64_t free_memory = 0;

    static ServerHealth from_json(const json::Value& j) {
        ServerHealth h;
        h.cpus = j["cpus"].as_int64();
        h.uptime = j["uptime"].as_int64();
        h.total_memory = j["totalMemory"].as_int64();
        h.max_memory = j["maxMemory"].as_int64();
        h.free_memory = j["freeMemory"].as_int64();
        return h;
    }
};

struct ServerBan {
    std::string target;
    std::optional<std::string> source;
    std::optional<std::string> reason;
    std::optional<std::string> expiration;

    static ServerBan from_json(const json::Value& j) {
        ServerBan b;
        b.target = j["target"].as_string();
        if (j.contains("source") && !j["source"].is_null()) b.source = j["source"].as_string();
        if (j.contains("reason") && !j["reason"].is_null()) b.reason = j["reason"].as_string();
        if (j.contains("expiration") && !j["expiration"].is_null()) b.expiration = j["expiration"].as_string();
        return b;
    }
};

struct WhitelistEntry {
    std::string uuid;
    std::string name;

    static WhitelistEntry from_json(const json::Value& j) {
        WhitelistEntry w;
        w.uuid = j["uuid"].as_string();
        w.name = j["name"].as_string();
        return w;
    }
};

struct ServerInfo {
    std::string name;
    std::string motd;
    std::string version;
    std::string bukkit_version;
    std::string tps;
    ServerHealth health;
    std::vector<ServerBan> banned_ips;
    std::vector<ServerBan> banned_players;
    std::vector<WhitelistEntry> whitelisted_players;
    int64_t max_players = 0;
    int64_t online_players = 0;

    static ServerInfo from_json(const json::Value& j) {
        ServerInfo info;
        info.name = j["name"].as_string();
        info.motd = j["motd"].as_string();
        info.version = j["version"].as_string();
        info.bukkit_version = j["bukkitVersion"].as_string();
        info.tps = j["tps"].as_string();
        info.health = ServerHealth::from_json(j["health"]);
        if (j.contains("bannedIps") && j["bannedIps"].is_array()) {
            for (const auto& el : j["bannedIps"].as_array()) info.banned_ips.push_back(ServerBan::from_json(el));
        }
        if (j.contains("bannedPlayers") && j["bannedPlayers"].is_array()) {
            for (const auto& el : j["bannedPlayers"].as_array()) info.banned_players.push_back(ServerBan::from_json(el));
        }
        if (j.contains("whitelistedPlayers") && j["whitelistedPlayers"].is_array()) {
            for (const auto& el : j["whitelistedPlayers"].as_array()) info.whitelisted_players.push_back(WhitelistEntry::from_json(el));
        }
        info.max_players = j["maxPlayers"].as_int64();
        info.online_players = j["onlinePlayers"].as_int64();
        return info;
    }
};

struct World {
    std::string name;
    std::string uuid;
    int64_t time = 0;
    bool storm = false;
    bool thundering = false;
    bool generate_structures = true;
    bool allow_animals = true;
    bool allow_monsters = true;
    std::string difficulty;
    std::string environment;
    std::string seed;

    static World from_json(const json::Value& j) {
        World w;
        w.name = j["name"].as_string();
        w.uuid = j["uuid"].as_string();
        w.time = j["time"].as_int64();
        w.storm = j["storm"].as_bool();
        w.thundering = j["thundering"].as_bool();
        w.generate_structures = j["generateStructures"].as_bool();
        w.allow_animals = j["allowAnimals"].as_bool();
        w.allow_monsters = j["allowMonsters"].as_bool();
        w.difficulty = j["difficulty"].as_string();
        w.environment = j["environment"].as_string();
        w.seed = j["seed"].as_string();
        return w;
    }
};

struct Plugin {
    std::string name;
    bool enabled = false;
    std::string version;
    std::optional<std::string> website;
    std::vector<std::string> authors;
    std::vector<std::string> depends;
    std::vector<std::string> soft_depends;
    std::optional<std::string> api_version;
    std::optional<std::string> description;

    static Plugin from_json(const json::Value& j) {
        Plugin p;
        p.name = j["name"].as_string();
        p.enabled = j["enabled"].as_bool();
        p.version = j["version"].as_string();
        if (j.contains("website") && !j["website"].is_null()) p.website = j["website"].as_string();
        if (j.contains("authors") && j["authors"].is_array()) {
            for (const auto& a : j["authors"].as_array()) p.authors.push_back(a.as_string());
        }
        if (j.contains("depends") && j["depends"].is_array()) {
            for (const auto& d : j["depends"].as_array()) p.depends.push_back(d.as_string());
        }
        if (j.contains("softDepends") && j["softDepends"].is_array()) {
            for (const auto& sd : j["softDepends"].as_array()) p.soft_depends.push_back(sd.as_string());
        }
        if (j.contains("apiVersion") && !j["apiVersion"].is_null()) p.api_version = j["apiVersion"].as_string();
        if (j.contains("description") && !j["description"].is_null()) p.description = j["description"].as_string();
        return p;
    }
};

struct EconomyInfo {
    bool available = false;
    std::string name;
    std::string currency_name_singular;
    std::string currency_name_plural;
    int fractional_digits = 2;

    static EconomyInfo from_json(const json::Value& j) {
        EconomyInfo e;
        e.available = j["available"].as_bool();
        e.name = j["name"].as_string();
        e.currency_name_singular = j["currencyNameSingular"].as_string();
        e.currency_name_plural = j["currencyNamePlural"].as_string();
        e.fractional_digits = j["fractionalDigits"].as_int(2);
        return e;
    }
};

struct PlayerBalance {
    std::string uuid;
    double balance = 0.0;

    static PlayerBalance from_json(const json::Value& j) {
        PlayerBalance b;
        b.uuid = j["uuid"].as_string();
        b.balance = j["balance"].as_double();
        return b;
    }
};

struct TopBalanceEntry {
    std::string uuid;
    double balance = 0.0;

    static TopBalanceEntry from_json(const json::Value& j) {
        TopBalanceEntry e;
        e.uuid = j["uuid"].as_string();
        e.balance = j["balance"].as_double();
        return e;
    }
};

struct LeaderboardSourceInfo {
    std::string id;
    std::string display_name;
    bool available = false;

    static LeaderboardSourceInfo from_json(const json::Value& j) {
        LeaderboardSourceInfo l;
        l.id = j["id"].as_string();
        l.display_name = j["displayName"].as_string();
        l.available = j["available"].as_bool();
        return l;
    }
};

struct CurrencyBalance {
    std::string uuid;
    std::optional<std::string> name;
    std::string currency;
    double balance = 0.0;

    static CurrencyBalance from_json(const json::Value& j) {
        CurrencyBalance b;
        b.uuid = j["uuid"].as_string();
        if (j.contains("name") && !j["name"].is_null()) b.name = j["name"].as_string();
        b.currency = j["currency"].as_string();
        b.balance = j["balance"].as_double();
        return b;
    }
};

struct CurrencyTopEntry {
    std::string uuid;
    std::string name;
    double balance = 0.0;

    static CurrencyTopEntry from_json(const json::Value& j) {
        CurrencyTopEntry e;
        e.uuid = j["uuid"].as_string();
        e.name = j["name"].as_string();
        e.balance = j["balance"].as_double();
        return e;
    }
};

struct GroupInfo {
    std::string name;
    std::vector<std::string> permissions;

    static GroupInfo from_json(const json::Value& j) {
        GroupInfo g;
        g.name = j["name"].as_string();
        if (j.contains("permissions") && j["permissions"].is_array()) {
            for (const auto& p : j["permissions"].as_array()) g.permissions.push_back(p.as_string());
        }
        return g;
    }
};

struct PermissionNode {
    std::string permission;
    bool value = true;
    int64_t expiry = 0;
    std::optional<std::string> server;
    std::optional<std::string> world;

    static PermissionNode from_json(const json::Value& j) {
        PermissionNode n;
        n.permission = j["permission"].as_string();
        n.value = j["value"].as_bool(true);
        n.expiry = j["expiry"].as_int64();
        if (j.contains("server") && !j["server"].is_null()) n.server = j["server"].as_string();
        if (j.contains("world") && !j["world"].is_null()) n.world = j["world"].as_string();
        return n;
    }
};

struct Advancement {
    std::string key;
    std::vector<std::string> criteria;

    static Advancement from_json(const json::Value& j) {
        Advancement a;
        a.key = j["key"].as_string();
        if (j.contains("criteria") && j["criteria"].is_array()) {
            for (const auto& c : j["criteria"].as_array()) a.criteria.push_back(c.as_string());
        }
        return a;
    }
};

struct NoxAuthPlayerInfo {
    std::string uuid;
    std::string name;
    bool registered = false;
    bool authenticated = false;
    std::optional<std::string> last_ip;
    std::optional<int64_t> last_login_time;
    std::optional<std::string> country_code;
    std::optional<std::string> country_name;

    static NoxAuthPlayerInfo from_json(const json::Value& j) {
        NoxAuthPlayerInfo info;
        info.uuid = j["uuid"].as_string();
        info.name = j["name"].as_string();
        info.registered = j["registered"].as_bool();
        info.authenticated = j["authenticated"].as_bool();
        if (j.contains("lastIp") && !j["lastIp"].is_null()) info.last_ip = j["lastIp"].as_string();
        if (j.contains("lastLoginTime") && !j["lastLoginTime"].is_null()) info.last_login_time = j["lastLoginTime"].as_int64();
        if (j.contains("countryCode") && !j["countryCode"].is_null()) info.country_code = j["countryCode"].as_string();
        if (j.contains("countryName") && !j["countryName"].is_null()) info.country_name = j["countryName"].as_string();
        return info;
    }
};

struct PasswordCheckResult {
    std::string name;
    bool valid = false;

    static PasswordCheckResult from_json(const json::Value& j) {
        PasswordCheckResult r;
        r.name = j["name"].as_string();
        r.valid = j["valid"].as_bool();
        return r;
    }
};

struct SkillInfo {
    std::string uuid;
    std::map<std::string, int64_t> skills;
    int64_t power_level = 0;

    static SkillInfo from_json(const json::Value& j) {
        SkillInfo s;
        s.uuid = j["uuid"].as_string();
        s.power_level = j["powerLevel"].as_int64();
        if (j.contains("skills") && j["skills"].is_object()) {
            for (const auto& kv : j["skills"].as_object()) {
                s.skills[kv.first] = kv.second.as_int64();
            }
        }
        return s;
    }
};

// ── Identity ────────────────────────────────────────────────────────────────
struct IdentityServerInfo {
    std::string id;
    std::string label;

    static IdentityServerInfo from_json(const json::Value& j) {
        IdentityServerInfo s;
        s.id = j["id"].as_string();
        s.label = j["label"].as_string();
        return s;
    }
};

struct IdentityInfo {
    std::string username;
    std::string identity_uuid;
    std::optional<std::string> local_uuid;
    IdentityServerInfo server;
    bool online_mode = false;
    bool has_played_before = false;

    static IdentityInfo from_json(const json::Value& j) {
        IdentityInfo info;
        info.username = j["username"].as_string();
        info.identity_uuid = j["identityUuid"].as_string();
        if (j.contains("localUuid") && !j["localUuid"].is_null()) info.local_uuid = j["localUuid"].as_string();
        info.server = IdentityServerInfo::from_json(j["server"]);
        info.online_mode = j["onlineMode"].as_bool();
        info.has_played_before = j["hasPlayedBefore"].as_bool();
        return info;
    }
};

// ── RankForge ───────────────────────────────────────────────────────────────
struct RankForgeTier {
    std::string name;
    int64_t min_points = 0;
    std::optional<int64_t> max_points;

    static RankForgeTier from_json(const json::Value& j) {
        RankForgeTier t;
        t.name = j["name"].as_string();
        t.min_points = j["minPoints"].as_int64();
        if (j.contains("maxPoints") && !j["maxPoints"].is_null()) {
            t.max_points = j["maxPoints"].as_int64();
        }
        return t;
    }
};

struct RankForgePlayerStats {
    std::string uuid;
    int64_t points = 0;
    int64_t kills = 0;
    int64_t deaths = 0;
    double kd = 0.0;
    int64_t daily_kills = 0;
    std::optional<RankForgeTier> current_tier;
    std::optional<RankForgeTier> next_tier;

    static RankForgePlayerStats from_json(const json::Value& j) {
        RankForgePlayerStats s;
        s.uuid = j["uuid"].as_string();
        s.points = j["points"].as_int64();
        s.kills = j["kills"].as_int64();
        s.deaths = j["deaths"].as_int64();
        s.kd = j["kd"].as_double();
        s.daily_kills = j["dailyKills"].as_int64();
        if (j.contains("currentTier") && !j["currentTier"].is_null()) {
            s.current_tier = RankForgeTier::from_json(j["currentTier"]);
        }
        if (j.contains("nextTier") && !j["nextTier"].is_null()) {
            s.next_tier = RankForgeTier::from_json(j["nextTier"]);
        }
        return s;
    }
};

struct RankForgePointsResponse {
    std::string uuid;
    int64_t points = 0;

    static RankForgePointsResponse from_json(const json::Value& j) {
        RankForgePointsResponse r;
        r.uuid = j["uuid"].as_string();
        r.points = j["points"].as_int64();
        return r;
    }
};

struct RankForgePointsTransactionResult {
    std::string uuid;
    int64_t points = 0;
    bool success = false;

    static RankForgePointsTransactionResult from_json(const json::Value& j) {
        RankForgePointsTransactionResult r;
        r.uuid = j["uuid"].as_string();
        r.points = j["points"].as_int64();
        r.success = j["success"].as_bool();
        return r;
    }
};

struct RankForgeTopEntry {
    int64_t rank = 0;
    std::string uuid;
    std::optional<std::string> name;
    int64_t points = 0;
    std::optional<RankForgeTier> tier;

    static RankForgeTopEntry from_json(const json::Value& j) {
        RankForgeTopEntry e;
        e.rank = j["rank"].as_int64();
        e.uuid = j["uuid"].as_string();
        if (j.contains("name") && !j["name"].is_null()) e.name = j["name"].as_string();
        e.points = j["points"].as_int64();
        if (j.contains("tier") && !j["tier"].is_null()) e.tier = RankForgeTier::from_json(j["tier"]);
        return e;
    }
};

struct RankForgeSaveResponse {
    bool success = false;

    static RankForgeSaveResponse from_json(const json::Value& j) {
        RankForgeSaveResponse r;
        r.success = j["success"].as_bool();
        return r;
    }
};

// ── Integrations ────────────────────────────────────────────────────────────

struct AuthMePlayerInfo {
    std::string name;
    std::string uuid;
    bool registered = false;
    bool authenticated = false;
    std::string last_ip;
    int64_t last_login_time = 0;

    static AuthMePlayerInfo from_json(const json::Value& j) {
        AuthMePlayerInfo info;
        info.name = j["name"].as_string();
        info.uuid = j["uuid"].as_string();
        info.registered = j["registered"].as_bool();
        info.authenticated = j["authenticated"].as_bool();
        info.last_ip = j["lastIp"].as_string();
        info.last_login_time = j["lastLoginTime"].as_int64();
        return info;
    }
};

struct FastLoginPlayerInfo {
    std::string name;
    std::string uuid;
    bool premium = false;

    static FastLoginPlayerInfo from_json(const json::Value& j) {
        FastLoginPlayerInfo info;
        info.name = j["name"].as_string();
        info.uuid = j["uuid"].as_string();
        info.premium = j["premium"].as_bool();
        return info;
    }
};

struct NLoginPlayerInfo {
    std::string name;
    std::string uuid;
    bool registered = false;
    bool authenticated = false;

    static NLoginPlayerInfo from_json(const json::Value& j) {
        NLoginPlayerInfo info;
        info.name = j["name"].as_string();
        info.uuid = j["uuid"].as_string();
        info.registered = j["registered"].as_bool();
        info.authenticated = j["authenticated"].as_bool();
        return info;
    }
};

struct LoginSecurityPlayerInfo {
    std::string name;
    std::string uuid;
    bool registered = false;
    bool authenticated = false;

    static LoginSecurityPlayerInfo from_json(const json::Value& j) {
        LoginSecurityPlayerInfo info;
        info.name = j["name"].as_string();
        info.uuid = j["uuid"].as_string();
        info.registered = j["registered"].as_bool();
        info.authenticated = j["authenticated"].as_bool();
        return info;
    }
};

struct BetterTeamsSummary {
    std::string name;
    std::string tag;
    std::string description;
    double score = 0.0;
    double money = 0.0;
    int64_t level = 0;
    int64_t member_count = 0;

    static BetterTeamsSummary from_json(const json::Value& j) {
        BetterTeamsSummary s;
        s.name = j["name"].as_string();
        s.tag = j["tag"].as_string();
        s.description = j["description"].as_string();
        s.score = j["score"].as_double();
        s.money = j["money"].as_double();
        s.level = j["level"].as_int64();
        s.member_count = j["memberCount"].as_int64();
        return s;
    }
};

struct BetterTeamsDetail {
    std::string name;
    std::string tag;
    std::string description;
    double score = 0.0;
    double money = 0.0;
    int64_t level = 0;
    int64_t member_count = 0;
    std::vector<std::string> members;

    static BetterTeamsDetail from_json(const json::Value& j) {
        BetterTeamsDetail d;
        d.name = j["name"].as_string();
        d.tag = j["tag"].as_string();
        d.description = j["description"].as_string();
        d.score = j["score"].as_double();
        d.money = j["money"].as_double();
        d.level = j["level"].as_int64();
        d.member_count = j["memberCount"].as_int64();
        if (j.contains("members") && j["members"].is_array()) {
            for (const auto& m : j["members"].as_array()) d.members.push_back(m.as_string());
        }
        return d;
    }
};

struct BetterTeamsListResponse {
    int64_t page = 1;
    int64_t limit = 10;
    int64_t total = 0;
    std::vector<BetterTeamsSummary> teams;

    static BetterTeamsListResponse from_json(const json::Value& j) {
        BetterTeamsListResponse r;
        r.page = j["page"].as_int64(1);
        r.limit = j["limit"].as_int64(10);
        r.total = j["total"].as_int64(0);
        if (j.contains("teams") && j["teams"].is_array()) {
            for (const auto& t : j["teams"].as_array()) r.teams.push_back(BetterTeamsSummary::from_json(t));
        }
        return r;
    }
};

struct EssentialsHomeLocation {
    std::string name;
    std::string world;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    std::optional<double> yaw;
    std::optional<double> pitch;

    static EssentialsHomeLocation from_json(const json::Value& j) {
        EssentialsHomeLocation h;
        h.name = j["name"].as_string();
        h.world = j["world"].as_string();
        h.x = j["x"].as_double();
        h.y = j["y"].as_double();
        h.z = j["z"].as_double();
        if (j.contains("yaw") && !j["yaw"].is_null()) h.yaw = j["yaw"].as_double();
        if (j.contains("pitch") && !j["pitch"].is_null()) h.pitch = j["pitch"].as_double();
        return h;
    }
};

struct EssentialsPlayerHomesResponse {
    std::string player;
    std::vector<EssentialsHomeLocation> homes;

    static EssentialsPlayerHomesResponse from_json(const json::Value& j) {
        EssentialsPlayerHomesResponse r;
        r.player = j["player"].as_string();
        if (j.contains("homes") && j["homes"].is_array()) {
            for (const auto& h : j["homes"].as_array()) r.homes.push_back(EssentialsHomeLocation::from_json(h));
        }
        return r;
    }
};

struct EssentialsWarpsResponse {
    int64_t total = 0;
    std::vector<std::string> warps;

    static EssentialsWarpsResponse from_json(const json::Value& j) {
        EssentialsWarpsResponse r;
        r.total = j["total"].as_int64();
        if (j.contains("warps") && j["warps"].is_array()) {
            for (const auto& w : j["warps"].as_array()) r.warps.push_back(w.as_string());
        }
        return r;
    }
};

struct EssentialsPlayerInfo {
    std::string player;
    std::optional<std::string> nickname;
    bool afk = false;
    bool muted = false;
    bool jailed = false;
    bool god_mode = false;

    static EssentialsPlayerInfo from_json(const json::Value& j) {
        EssentialsPlayerInfo info;
        info.player = j["player"].as_string();
        if (j.contains("nickname") && !j["nickname"].is_null()) info.nickname = j["nickname"].as_string();
        info.afk = j["afk"].as_bool();
        info.muted = j["muted"].as_bool();
        info.jailed = j["jailed"].as_bool();
        info.god_mode = j["godMode"].as_bool();
        return info;
    }
};

struct GriefPreventionBoundary {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    static GriefPreventionBoundary from_json(const json::Value& j) {
        GriefPreventionBoundary b;
        b.x = j["x"].as_double();
        b.y = j["y"].as_double();
        b.z = j["z"].as_double();
        return b;
    }
};

struct GriefPreventionClaim {
    std::optional<std::string> id;
    std::optional<std::string> owner;
    std::optional<std::string> owner_uuid;
    std::optional<std::string> world;
    std::optional<GriefPreventionBoundary> greater_boundary;
    std::optional<GriefPreventionBoundary> lesser_boundary;
    std::optional<double> area;

    static GriefPreventionClaim from_json(const json::Value& j) {
        GriefPreventionClaim c;
        if (j.contains("id") && !j["id"].is_null()) c.id = j["id"].as_string();
        if (j.contains("owner") && !j["owner"].is_null()) c.owner = j["owner"].as_string();
        if (j.contains("ownerUuid") && !j["ownerUuid"].is_null()) c.owner_uuid = j["ownerUuid"].as_string();
        if (j.contains("world") && !j["world"].is_null()) c.world = j["world"].as_string();
        if (j.contains("greaterBoundary") && !j["greaterBoundary"].is_null()) {
            c.greater_boundary = GriefPreventionBoundary::from_json(j["greaterBoundary"]);
        }
        if (j.contains("lesserBoundary") && !j["lesserBoundary"].is_null()) {
            c.lesser_boundary = GriefPreventionBoundary::from_json(j["lesserBoundary"]);
        }
        if (j.contains("area") && !j["area"].is_null()) c.area = j["area"].as_double();
        return c;
    }
};

struct GriefPreventionClaimsResponse {
    std::string uuid;
    int64_t total = 0;
    std::vector<GriefPreventionClaim> claims;

    static GriefPreventionClaimsResponse from_json(const json::Value& j) {
        GriefPreventionClaimsResponse r;
        r.uuid = j["uuid"].as_string();
        r.total = j["total"].as_int64();
        if (j.contains("claims") && j["claims"].is_array()) {
            for (const auto& c : j["claims"].as_array()) r.claims.push_back(GriefPreventionClaim::from_json(c));
        }
        return r;
    }
};

struct DiscordSRVLinkedResponse {
    std::string uuid;
    std::string discord_id;

    static DiscordSRVLinkedResponse from_json(const json::Value& j) {
        DiscordSRVLinkedResponse r;
        r.uuid = j["uuid"].as_string();
        r.discord_id = j["discordId"].as_string();
        return r;
    }
};

struct DiscordSRVBroadcastResponse {
    bool success = false;
    std::string channel;
    std::string message;

    static DiscordSRVBroadcastResponse from_json(const json::Value& j) {
        DiscordSRVBroadcastResponse r;
        r.success = j["success"].as_bool();
        r.channel = j["channel"].as_string();
        r.message = j["message"].as_string();
        return r;
    }
};

struct PlayerPointsBalanceResponse {
    std::string uuid;
    int64_t points = 0;

    static PlayerPointsBalanceResponse from_json(const json::Value& j) {
        PlayerPointsBalanceResponse r;
        r.uuid = j["uuid"].as_string();
        r.points = j["points"].as_int64();
        return r;
    }
};

struct PlayerPointsTransactionResult {
    std::string uuid;
    int64_t amount = 0;
    bool success = false;
    int64_t current_balance = 0;

    static PlayerPointsTransactionResult from_json(const json::Value& j) {
        PlayerPointsTransactionResult r;
        r.uuid = j["uuid"].as_string();
        r.amount = j["amount"].as_int64();
        r.success = j["success"].as_bool();
        r.current_balance = j["currentBalance"].as_int64();
        return r;
    }
};

struct WorldGuardRegionPoint {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    static WorldGuardRegionPoint from_json(const json::Value& j) {
        WorldGuardRegionPoint p;
        p.x = j["x"].as_double();
        p.y = j["y"].as_double();
        p.z = j["z"].as_double();
        return p;
    }
};

struct WorldGuardRegion {
    std::string id;
    std::optional<std::string> region_type;
    std::optional<int64_t> priority;
    std::vector<std::string> owners;
    std::vector<std::string> members;
    std::map<std::string, std::string> flags;
    std::optional<WorldGuardRegionPoint> min;
    std::optional<WorldGuardRegionPoint> max;

    static WorldGuardRegion from_json(const json::Value& j) {
        WorldGuardRegion r;
        r.id = j["id"].as_string();
        if (j.contains("type") && !j["type"].is_null()) r.region_type = j["type"].as_string();
        if (j.contains("priority") && !j["priority"].is_null()) r.priority = j["priority"].as_int64();
        if (j.contains("owners") && j["owners"].is_array()) {
            for (const auto& o : j["owners"].as_array()) r.owners.push_back(o.as_string());
        }
        if (j.contains("members") && j["members"].is_array()) {
            for (const auto& m : j["members"].as_array()) r.members.push_back(m.as_string());
        }
        if (j.contains("flags") && j["flags"].is_object()) {
            for (const auto& kv : j["flags"].as_object()) r.flags[kv.first] = kv.second.dump();
        }
        if (j.contains("min") && !j["min"].is_null()) r.min = WorldGuardRegionPoint::from_json(j["min"]);
        if (j.contains("max") && !j["max"].is_null()) r.max = WorldGuardRegionPoint::from_json(j["max"]);
        return r;
    }
};

struct PlanPlayerSummary {
    std::string uuid;

    static PlanPlayerSummary from_json(const json::Value& j) {
        PlanPlayerSummary s;
        s.uuid = j["uuid"].as_string();
        return s;
    }
};

struct GeyserPlayerInfo {
    bool bedrock = false;
    std::optional<std::string> xuid;
    std::optional<std::string> device_os;
    std::optional<std::string> version;

    static GeyserPlayerInfo from_json(const json::Value& j) {
        GeyserPlayerInfo info;
        info.bedrock = j["bedrock"].as_bool();
        if (j.contains("xuid") && !j["xuid"].is_null()) info.xuid = j["xuid"].as_string();
        if (j.contains("deviceOs") && !j["deviceOs"].is_null()) info.device_os = j["deviceOs"].as_string();
        if (j.contains("version") && !j["version"].is_null()) info.version = j["version"].as_string();
        return info;
    }
};

struct MythicMobsListResponse {
    int64_t total = 0;
    std::vector<std::string> mobs;

    static MythicMobsListResponse from_json(const json::Value& j) {
        MythicMobsListResponse r;
        r.total = j["total"].as_int64();
        if (j.contains("mobs") && j["mobs"].is_array()) {
            for (const auto& m : j["mobs"].as_array()) r.mobs.push_back(m.as_string());
        }
        return r;
    }
};

struct MythicMobSpawnOptions {
    std::string mob_type;
    std::string world;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    std::optional<int64_t> level;

    std::string to_json_string() const {
        json::Value j;
        j["type"] = mob_type;
        j["world"] = world;
        j["x"] = x;
        j["y"] = y;
        j["z"] = z;
        if (level.has_value()) j["level"] = *level;
        return j.dump();
    }
};

struct MythicMobSpawnResponse {
    bool success = false;
    std::string mob_type;
    std::string world;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    int64_t level = 1;

    static MythicMobSpawnResponse from_json(const json::Value& j) {
        MythicMobSpawnResponse r;
        r.success = j["success"].as_bool();
        r.mob_type = j["type"].as_string();
        r.world = j["world"].as_string();
        r.x = j["x"].as_double();
        r.y = j["y"].as_double();
        r.z = j["z"].as_double();
        r.level = j["level"].as_int64(1);
        return r;
    }
};

struct TabNametagResponse {
    std::string uuid;
    std::string prefix;
    std::string suffix;
    std::optional<bool> success;

    static TabNametagResponse from_json(const json::Value& j) {
        TabNametagResponse r;
        r.uuid = j["uuid"].as_string();
        r.prefix = j["prefix"].as_string();
        r.suffix = j["suffix"].as_string();
        if (j.contains("success") && !j["success"].is_null()) r.success = j["success"].as_bool();
        return r;
    }
};

struct CoreProtectLookupOptions {
    std::optional<int64_t> time;
    std::optional<std::string> user;
    std::optional<int64_t> limit;
};

struct CoreProtectLookupEntry {
    std::optional<int64_t> time;
    std::optional<std::string> user;
    std::optional<std::string> action;
    std::optional<std::string> entry_type;
    std::optional<std::string> world;
    std::optional<double> x;
    std::optional<double> y;
    std::optional<double> z;

    static CoreProtectLookupEntry from_json(const json::Value& j) {
        CoreProtectLookupEntry e;
        if (j.contains("time") && !j["time"].is_null()) e.time = j["time"].as_int64();
        if (j.contains("user") && !j["user"].is_null()) e.user = j["user"].as_string();
        if (j.contains("action") && !j["action"].is_null()) e.action = j["action"].as_string();
        if (j.contains("type") && !j["type"].is_null()) e.entry_type = j["type"].as_string();
        if (j.contains("world") && !j["world"].is_null()) e.world = j["world"].as_string();
        if (j.contains("x") && !j["x"].is_null()) e.x = j["x"].as_double();
        if (j.contains("y") && !j["y"].is_null()) e.y = j["y"].as_double();
        if (j.contains("z") && !j["z"].is_null()) e.z = j["z"].as_double();
        return e;
    }
};

struct CoreProtectLookupResponse {
    int64_t total = 0;
    int64_t time_window_seconds = 0;
    std::string user_filter;
    std::vector<CoreProtectLookupEntry> results;

    static CoreProtectLookupResponse from_json(const json::Value& j) {
        CoreProtectLookupResponse r;
        r.total = j["total"].as_int64();
        r.time_window_seconds = j["timeWindowSeconds"].as_int64();
        r.user_filter = j["userFilter"].as_string();
        if (j.contains("results") && j["results"].is_array()) {
            for (const auto& item : j["results"].as_array()) {
                r.results.push_back(CoreProtectLookupEntry::from_json(item));
            }
        }
        return r;
    }
};

struct ShopGuiSummary {
    std::string id;
    std::optional<std::string> name;

    static ShopGuiSummary from_json(const json::Value& j) {
        ShopGuiSummary s;
        s.id = j["id"].as_string();
        if (j.contains("name") && !j["name"].is_null()) s.name = j["name"].as_string();
        return s;
    }
};

struct ShopGuiListResponse {
    int64_t total = 0;
    std::vector<ShopGuiSummary> shops;

    static ShopGuiListResponse from_json(const json::Value& j) {
        ShopGuiListResponse r;
        r.total = j["total"].as_int64();
        if (j.contains("shops") && j["shops"].is_array()) {
            for (const auto& s : j["shops"].as_array()) r.shops.push_back(ShopGuiSummary::from_json(s));
        }
        return r;
    }
};

struct ShopGuiShopDetail {
    std::string id;

    static ShopGuiShopDetail from_json(const json::Value& j) {
        ShopGuiShopDetail d;
        d.id = j["id"].as_string();
        return d;
    }
};

} // namespace noxaeapi
