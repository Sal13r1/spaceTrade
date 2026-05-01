#include "map.h"
#include <ncurses.h>
#include <cstdlib>

// Stock caps and respawn rates (ticks per nugget) indexed by ResTier
static constexpr int RES_MAX_STOCK[]   = { 0,  5, 15, 30 };
static constexpr int RES_RESPAWN_RATE[] = { 0,  3,  5, 10 };

// Fraction of total tiles filled per tier
static constexpr double T1_DENSITY = 0.040;
static constexpr double T2_DENSITY = 0.015;
static constexpr double T3_DENSITY = 0.005;

// Fraction of interior tiles turned into walls
static constexpr double WALL_DENSITY = 0.08;

Map::Map(int width, int height)
    : _width(width), _height(height), _grid(width * height) {}

Tile& Map::at(int x, int y) {
    return _grid[y * _width + x];
}

const Tile& Map::at(int x, int y) const {
    return _grid[y * _width + x];
}

bool Map::inBounds(int x, int y) const {
    return x >= 0 && x < _width && y >= 0 && y < _height;
}

bool Map::passable(int x, int y) const {
    if (!inBounds(x, y)) return false;
    return at(x, y).terrain == Terrain::PLAIN;
}

void Map::generate(unsigned int seed) {
    if (seed == 0) seed = static_cast<unsigned int>(time(nullptr));
    srand(seed);

    for (auto& t : _grid) t = Tile{};

    placeWalls();
    placeResourceNodes();
}

void Map::placeWalls() {
    // Solid border
    for (int x = 0; x < _width; ++x) {
        at(x, 0).terrain          = Terrain::WALL;
        at(x, _height - 1).terrain = Terrain::WALL;
    }
    for (int y = 0; y < _height; ++y) {
        at(0, y).terrain         = Terrain::WALL;
        at(_width - 1, y).terrain = Terrain::WALL;
    }

    // Scattered interior walls
    int interior  = (_width - 2) * (_height - 2);
    int wallCount = static_cast<int>(interior * WALL_DENSITY);
    for (int i = 0; i < wallCount; ++i) {
        int x = 1 + rand() % (_width  - 2);
        int y = 1 + rand() % (_height - 2);
        at(x, y).terrain = Terrain::WALL;
    }
}

void Map::initResourceNode(Tile& tile, ResTier tier) {
    int t                      = static_cast<int>(tier);
    tile.resource.tier         = tier;
    tile.resource.maxStock     = RES_MAX_STOCK[t];
    tile.resource.stock        = RES_MAX_STOCK[t];
    tile.resource.respawnRate  = RES_RESPAWN_RATE[t];
    tile.resource.respawnTimer = RES_RESPAWN_RATE[t];
    tile.resource.owner        = -1;
}

void Map::placeResourceNodes() {
    int total = _width * _height;

    auto place = [&](ResTier tier, double density) {
        int count = static_cast<int>(total * density);
        for (int i = 0; i < count; ++i) {
            for (int attempt = 0; attempt < 20; ++attempt) {
                int x = 1 + rand() % (_width  - 2);
                int y = 1 + rand() % (_height - 2);
                Tile& t = at(x, y);
                if (t.terrain == Terrain::PLAIN && t.resource.tier == ResTier::NONE) {
                    initResourceNode(t, tier);
                    break;
                }
            }
        }
    };

    place(ResTier::T1, T1_DENSITY);
    place(ResTier::T2, T2_DENSITY);
    place(ResTier::T3, T3_DENSITY);
}

void Map::update() {
    for (auto& tile : _grid) {
        ResourceNode& r = tile.resource;
        if (r.tier == ResTier::NONE || r.stock >= r.maxStock) continue;

        if (--r.respawnTimer <= 0) {
            ++r.stock;
            r.respawnTimer = r.respawnRate;
        }
    }
}

void Map::render() const {
    for (int y = 0; y < _height; ++y) {
        for (int x = 0; x < _width; ++x) {
            const Tile& t = at(x, y);

            chtype ch;
            switch (t.terrain) {
                case Terrain::WALL:  ch = '#'; break;
                case Terrain::WATER: ch = '~'; break;
                default:
                    if (t.building.type != BldgType::NONE) {
                        switch (t.building.type) {
                            case BldgType::BASE:        ch = 'B'; break;
                            case BldgType::DRONE_FORGE: ch = 'F'; break;
                            case BldgType::WAR_HALL:    ch = 'W'; break;
                            default:                    ch = '?'; break;
                        }
                    } else if (t.resource.tier != ResTier::NONE) {
                        switch (t.resource.tier) {
                            case ResTier::T1: ch = '*'; break;
                            case ResTier::T2: ch = '%'; break;
                            case ResTier::T3: ch = '$'; break;
                            default:          ch = '.'; break;
                        }
                    } else {
                        ch = '.';
                    }
                    break;
            }

            mvaddch(y, x, ch);
        }
    }
}
