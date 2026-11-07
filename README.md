# NoxAeApi C++ SDK

Official header-only C++17 SDK for the [NoxAeApi](https://github.com/NoxlyDev/NOXAPI-HUB) Minecraft REST API suite and Velocity Network Hub.

## Features

- **Header-Only C++17/20**: Just `#include <noxaeapi/noxaeapi.hpp>`
- Built-in zero-dependency JSON parser & serializer
- Optional `libcurl` integration for high-performance HTTP transport
- Complete module coverage:
  - Players, Server, Economy, Leaderboards, LuckPerms, NoxAuth
  - Plugins, Worlds, Skills, Identity, Integrations (RankForge, etc.), Misc
- Velocity Hub Node routing support via `client.hub_node("server-1")`

## CMake Usage

```cmake
# Using FetchContent
include(FetchContent)
FetchContent_Declare(
    noxaeapi-sdk-cpp
    GIT_REPOSITORY https://github.com/NoxlyDev/NOXAPI-HUB.git
    GIT_TAG        v0.7.0
    SOURCE_SUBDIR  Library/noxaeapi-sdk-cpp
)
FetchContent_MakeAvailable(noxaeapi-sdk-cpp)

target_link_libraries(your_project PRIVATE noxaeapi::sdk)
```

## Quick Start

```cpp
#include <iostream>
#include <noxaeapi/noxaeapi.hpp>

int main() {
    noxaeapi::Client client("http://localhost:8080", "your-api-key");

    // Fetch online players
    auto players = client.players.get_online();
    for (const auto& player : players) {
        std::cout << "Online: " << player.display_name << " (" << player.uuid << ")\n";
    }

    // Velocity Hub Node routing
    auto hub = client.hub_node("survival-1");
    auto srv = hub.server.get_info();
    std::cout << "Survival TPS: " << srv.tps << "\n";

    return 0;
}
```

## License

MIT © Wumx Labs & NoxAeApi Contributors
