#pragma once

#include "http.hpp"
#include "hub_node.hpp"
#include "modules/players.hpp"
#include "modules/server.hpp"
#include "modules/economy.hpp"
#include "modules/leaderboard.hpp"
#include "modules/luckperms.hpp"
#include "modules/noxauth.hpp"
#include "modules/plugins.hpp"
#include "modules/worlds.hpp"
#include "modules/skills.hpp"
#include "modules/identity.hpp"
#include "modules/integrations.hpp"
#include "modules/misc.hpp"

namespace noxaeapi {

class Client {
private:
    HttpClient http_;

public:
    modules::PlayersModule players;
    modules::ServerModule server;
    modules::EconomyModule economy;
    modules::LeaderboardModule leaderboard;
    modules::LuckPermsModule luckperms;
    modules::NoxAuthModule noxauth;
    modules::PluginsModule plugins;
    modules::WorldsModule worlds;
    modules::SkillsModule skills;
    modules::IdentityModule identity;
    modules::IntegrationsModule integrations;
    modules::RankForgeModule& rankforge;
    modules::MiscModule misc;

    Client(const std::string& base_url, const std::string& api_key)
        : http_(base_url, api_key),
          players(http_),
          server(http_),
          economy(http_),
          leaderboard(http_),
          luckperms(http_),
          noxauth(http_),
          plugins(http_),
          worlds(http_),
          skills(http_),
          identity(http_),
          integrations(http_),
          rankforge(integrations.rankforge),
          misc(http_) {}

    HubNodeClient hub_node(const std::string& node_id) const {
        return HubNodeClient(http_, node_id);
    }

    const HttpClient& get_http_client() const {
        return http_;
    }
};

} // namespace noxaeapi
