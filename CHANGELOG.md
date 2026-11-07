# Changelog

All notable changes to the `noxaeapi-sdk-cpp` will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.7.0] - 2026-11-07

### Added
- Initial official release of the C++17 Header-Only SDK for NoxAeApi ecosystem.
- Zero-dependency built-in JSON parser & serializer.
- Complete support for all NoxAeApi core modules:
  - `players`: Online/offline players, inventories, ender chests, health/food, teleportation, kicks, bans, whitelist.
  - `server`: Health metrics, TPS, commands, broadcasts, server properties.
  - `economy`: Vault economy, multi-currency support, top balances.
  - `leaderboard`: Multiple leaderboard sources and rankings.
  - `luckperms`: Full LuckPerms permissions, groups, nodes management.
  - `noxauth`: In-game authentication, registration, password verification.
  - `plugins`: Plugin state query, enable, disable.
  - `worlds`: World management, weather, time, gamerules.
  - `skills`: Player skill levels, experience, power levels.
  - `identity`: Velocity cross-server identity resolution and server listing.
  - `rankforge`: PvP stats, seasonal tiers, points, top rankings.
  - `integrations`: Direct access to 16+ supported Spigot/Paper plugins.
  - `misc`: Ping, health check, runtime status.
- Velocity network node client via `client.hub_node(node_id)`.
