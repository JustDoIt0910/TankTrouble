//
// Created by zr on 23-2-27.
//

#include "Maze.h"
#include "util/Math.h"
#include <cstring>

namespace TankTrouble
{
    static int dx[] = {0, -1, 0, 1};
    static int dy[] = {-1, 0, 1, 0};
    int vis[HORIZON_GRID_NUMBER][VERTICAL_GRID_NUMBER];

    Maze::Maze():
        map(MAX_GRID_ID, std::vector<int>(MAX_GRID_ID, 0))
    {
        memset(vis, 0 ,sizeof(vis));
        std::vector<std::pair<Grid, Grid>> walls;
        vis[0][0] = 1;
        walls.emplace_back(Grid(0, 0), Grid(1, 0));
        while(!walls.empty())
        {
            int n = util::getRandomNumber(0, walls.size() - 1);
            std::pair<Grid, Grid> wall = walls[n];
            walls.erase(walls.begin() + n);
            if(!vis[wall.second.x][wall.second.y])
            {
                map[wall.first.id()][wall.second.id()] = map[wall.second.id()][wall.first.id()] = 1;
                vis[wall.second.x][wall.second.y] = 1;
            }
            for(int i = 0; i < 4; i++)
            {
                int nx = wall.second.x + dx[i];
                int ny = wall.second.y + dy[i];
                if(nx < 0 || nx >= HORIZON_GRID_NUMBER || ny < 0 || ny >= VERTICAL_GRID_NUMBER)
                    continue;
                if(vis[nx][ny]) continue;
                Grid next(nx, ny);
                if(map[wall.second.id()][next.id()] == 0)
                    walls.emplace_back(wall.second, next);
            }
        }
    }

    std::vector<std::pair<util::Vec, util::Vec>> Maze::getBlockPositions()
    {
        std::vector<std::pair<util::Vec, util::Vec>> blocks;
        for(int y = 0; y < VERTICAL_GRID_NUMBER; y++)
            for(int x = 0; x < HORIZON_GRID_NUMBER - 1; x++)
                if(map[Grid(x, y).id()][Grid(x + 1, y).id()] == 0)
                    blocks.emplace_back(util::Vec((x + 1) * GRID_SIZE, y * GRID_SIZE),
                                        util::Vec((x + 1) * GRID_SIZE, (y + 1) * GRID_SIZE));
        for(int x = 0; x < HORIZON_GRID_NUMBER; x++)
            for(int y = 0; y < VERTICAL_GRID_NUMBER - 1; y++)
                if(map[Grid(x, y).id()][Grid(x, y + 1).id()] == 0)
                    blocks.emplace_back(util::Vec(x * GRID_SIZE, (y + 1) * GRID_SIZE),
                                        util::Vec((x + 1) * GRID_SIZE, (y + 1) * GRID_SIZE));
        return blocks;
    }
}