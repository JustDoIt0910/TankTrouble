//
// Created by zr on 23-2-8.
//

#ifndef TANK_TROUBLE_LOCAL_CONTROLLER_H
#define TANK_TROUBLE_LOCAL_CONTROLLER_H
#include <memory>
#include <vector>
#include <unordered_map>
#include "Controller.h"
#include "Object.h"
#include "util/Vec.h"
#include "Maze.h"
#include "defs.h"
#include "smithAI/DodgeStrategy.h"
#include "smithAI/ContactStrategy.h"
#include "smithAI/AttackStrategy.h"
#include "smithAI/AStar.h"

#define UPWARDS             0
#define UPWARDS_LEFT        1
#define LEFT                2
#define DOWNWARDS_LEFT      3
#define DOWNWARDS           4
#define DOWNWARDS_RIGHT     5
#define RIGHT               6
#define UPWARDS_RIGHT       7

namespace TankTrouble
{
    class AgentSmith;

    class LocalController : public Controller {
    public:
        LocalController();
        ~LocalController() override;
        void start() override;

    private:
        void restart(double delay);
        void initAll();
        void run();
        void moveAll();
        void controlEventHandler(ev::Event* event);
        void strategyUpdateHandler(ev::Event* event);
        void fire(Tank* tank);

        int checkShellCollision(const Object::PosInfo& curPos, const Object::PosInfo& nextPos);
        int checkShellBlockCollision(const Object::PosInfo& curPos, const Object::PosInfo& nextPos);
        int checkShellTankCollision(const Object::PosInfo& curPos, const Object::PosInfo& nextPos);

        int checkTankBlockCollision(const Object::PosInfo& curPos, const Object::PosInfo& nextPos);
        Object::PosInfo getBouncedPosition(const Object::PosInfo& cur, const Object::PosInfo& next, int blockId);

        void initBlocks();
        struct PairHash
        {
            template<typename T1, typename T2>
            size_t operator()(const std::pair<T1, T2>& p) const
            {
                return std::hash<T1>()(p.first) ^ std::hash<T2>()(p.second);
            }
        };
        static std::vector<Object::PosInfo> getRandomPositions(int num);
        bool getSmithPosition(Object::PosInfo& pos);
        bool getMyPosition(Object::PosInfo& pos);

        Maze maze;
        std::vector<int> deletedObjs;
        ObjectList objects;
        std::vector<int> shellPossibleCollisionBlocks[HORIZON_GRID_NUMBER][VERTICAL_GRID_NUMBER][8];
        std::vector<int> tankPossibleCollisionBlocks[HORIZON_GRID_NUMBER][VERTICAL_GRID_NUMBER];
        int tankNum;
        uint64_t globalSteps;

        friend class AgentSmith;
        friend class DodgeStrategy;
        friend class ContactStrategy;
        friend class AttackStrategy;
        std::unique_ptr<AgentSmith> smith;
        int danger;
        std::unique_ptr<DodgeStrategy> smithDodgeStrategy;
        std::unique_ptr<ContactStrategy> smithContactStrategy;
        std::unique_ptr<AttackStrategy> smithAttackStrategy;
    };
}

#endif //TANK_TROUBLE_LOCAL_CONTROLLER_H
