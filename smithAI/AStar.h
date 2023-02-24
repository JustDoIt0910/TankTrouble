//
// Created by zr on 23-2-24.
//

#ifndef TANK_TROUBLE_A_STAR_H
#define TANK_TROUBLE_A_STAR_H
#include "defs.h"
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <vector>
#include <memory>
#define MAX_A_STAR_NODE_ID  (VERTICAL_A_STAR_GRID_NUMBER * HORIZON_A_STAR_GRID_NUMBER)
#define REACHABLE           0
#define UNREACHABLE         1

namespace TankTrouble
{
    const static int HORIZON_VERTICAL_COST = 10;
    const static int DIAGONAL_COST = 14;
    class Block;

    class AStar
    {
    public:
        typedef std::unordered_map<int, Block> BlockList;
        typedef std::deque<std::pair<int, int>> AStarResult;
        AStar();
        void init(BlockList* blocks);
        AStarResult findRoute(int sx, int sy, int ex, int ey);

    private:
        struct AStarNode
        {
            int x, y;
            int id, parentId;
            int G, H, F;
            AStarNode(int _x, int _y):
                x(_x), y(_y), id(getGridId(_x, _y)),
                parentId(-1), G(0), H(0), F(0){}
        };
        static int getGridId(int x, int y);

        int getRoute(int sx, int sy, int ex, int ey);
        void addToOpenList(std::unique_ptr<AStarNode>&& node);
        std::vector<AStarNode> getReachable(int x, int y);
        static int calcG(AStarNode* parent, AStarNode* cur);
        static int calcH(AStarNode* cur, int ex, int ey);

        std::vector<std::vector<int>> map;

        typedef std::pair<int, int> Entry;
        typedef std::priority_queue<Entry, std::vector<Entry>, std::greater<>> EntryList;
        EntryList entryList;
        std::unordered_set<int> openSet;
        std::unordered_set<int> closedSet;
        std::unordered_map<int, std::unique_ptr<AStarNode>> nodes;
    };
}

#endif //TANK_TROUBLE_A_STAR_H
