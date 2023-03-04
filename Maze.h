//
// Created by zr on 23-2-27.
//

#ifndef TANK_TROUBLE_MAZE_H
#define TANK_TROUBLE_MAZE_H
#include <vector>
#include <utility>
#include "util/Vec.h"
#include "defs.h"

#define MAX_GRID_ID (HORIZON_GRID_NUMBER * VERTICAL_GRID_NUMBER)

namespace TankTrouble
{
    class Maze
    {
    public:
        Maze() = default;
        void generate();
        std::vector<std::pair<util::Vec, util::Vec>> getBlockPositions();

    private:
        struct Grid
        {
            int x, y;
            Grid(int _x, int _y): x(_x), y(_y) {}
            [[nodiscard]] int id() const {return y * HORIZON_GRID_NUMBER + x;}
        };
        std::vector<std::vector<int>> map;
    };
}

#endif //TANK_TROUBLE_MAZE_H
