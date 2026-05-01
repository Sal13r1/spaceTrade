#pragma once
#include <vector>
#include <cstdint>
#include <ctime>

constexpr int MAX_FACTIONS = 4;
constexpr int MAP_WIDTH    = 120;
constexpr int MAP_HEIGHT   = 40;

enum class Terrain  : uint8_t { PLAIN, WALL, WATER };
enum class ResTier  : uint8_t { NONE, T1, T2, T3 };
enum class BldgType : uint8_t { NONE, BASE, DRONE_FORGE, WAR_HALL };

struct ResourceNode {
    ResTier tier         = ResTier::NONE;
    int     stock        = 0;
    int     maxStock     = 0;
    int     respawnRate  = 0;  // ticks between nugget spawns
    int     respawnTimer = 0;
    int     owner        = -1; // faction index, -1 = neutral
};

struct Building {
    BldgType type  = BldgType::NONE;
    int      owner = -1;
    int      tier  = 1;
};

struct Tile {
    Terrain      terrain  = Terrain::PLAIN;
    ResourceNode resource;
    Building     building;
    bool         explored[MAX_FACTIONS] = {};
};

class Map {
public:
    Map(int width = MAP_WIDTH, int height = MAP_HEIGHT);

    void generate(unsigned int seed = 0);
    void update();   // advances resource respawn timers
    void render() const;

    Tile&       at(int x, int y);
    const Tile& at(int x, int y) const;

    bool inBounds(int x, int y) const;
    bool passable(int x, int y) const;

    int width()  const { return _width; }
    int height() const { return _height; }

private:
    int               _width;
    int               _height;
    std::vector<Tile> _grid;

    void placeWalls();
    void placeResourceNodes();
    void initResourceNode(Tile& tile, ResTier tier);
};
