#pragma once

#include "http.hpp"
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

class HubNodeClient {
private:
    HttpClient http_;
    std::string node_id_;

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

    HubNodeClient(const HttpClient& parent_http, std::string node_id)
        : node_id_(std::move(node_id)),
          http_(parent_http.with_header("X-Velocity-Node", node_id_)),
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

    const std::string& get_node_id() const { return node_id_; }
};

} // namespace noxaeapi
