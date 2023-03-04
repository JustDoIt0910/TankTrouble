//
// Created by zr on 23-2-8.
//

#include "defs.h"
#include "LocalController.h"
#include "util/Math.h"
#include "util/Id.h"
#include "event/ControlEvent.h"
#include "event/StrategyUpdateEvent.h"
#include "Shell.h"
#include "smithAI/AgentSmith.h"
#include <thread>
#include <cassert>

namespace TankTrouble
{
    static util::Vec topBorderCenter(static_cast<double>(GAME_VIEW_WIDTH) / 2, 2.0);
    static util::Vec leftBorderCenter(2.0, static_cast<double>(GAME_VIEW_HEIGHT) / 2);
    static util::Vec bottomBorderCenter(static_cast<double>(GAME_VIEW_WIDTH) / 2, 
                                        static_cast<double>(GAME_VIEW_HEIGHT) - 2 - 1);
    static util::Vec rightBorderCenter(static_cast<double>(GAME_VIEW_WIDTH) - 2 - 1, 
                                       static_cast<double>(GAME_VIEW_HEIGHT) / 2);

    LocalController::LocalController():
        Controller(),
        tankNum(0),
        globalSteps(0),
        smith(new AgentSmith(this)) {initAll();}

    LocalController::~LocalController()
    {
        controlLoop->quit();
        if(controlThread.joinable())
            controlThread.join();
    }

    void LocalController::start()
    {
        controlThread = std::thread(&LocalController::run, this);
        std::unique_lock<std::mutex> lk(mu);
        cv.wait(lk, [this] () -> bool {return this->started;});
    }

    void LocalController::restart(double delay)
    {
        controlLoop->runAfter(delay, [this]() -> void {
            tankNum = 0;
            globalSteps = 0;
            objects.clear();
            blocks.clear();
            deletedObjs.clear();
            for(int i = 0; i < HORIZON_GRID_NUMBER; i++)
                for(int j = 0; j < VERTICAL_GRID_NUMBER; j++)
                {
                    tankPossibleCollisionBlocks[i][j].clear();
                    for(int k = 0; k < 8; k++)
                        shellPossibleCollisionBlocks[i][j][k].clear();
                }
            smithDodgeStrategy.reset(nullptr);
            smithContactStrategy.reset(nullptr);
            smithAttackStrategy.reset(nullptr);
            util::Id::reset();
            initAll();
        });
    }

    void LocalController::initAll()
    {
        initBlocks();
        std::vector<Object::PosInfo> pos = getRandomPositions(2);
        std::unique_ptr<Object> tank(new Tank(pos[0].pos, pos[0].angle, RED));
        objects[tank->id()] = std::move(tank);
        std::unique_ptr<Object> smithTank(new Tank(pos[1].pos, pos[1].angle, GREY));
        objects[smithTank->id()] = std::move(smithTank);
        tankNum += 2;
        danger = 0;
        smith->initAStar(&blocks);
    }

    void LocalController::run()
    {
        ev::reactor::EventLoop loop;
        controlLoop = &loop;
        {
            std::unique_lock<std::mutex> lk(mu);
            started = true;
            cv.notify_all();
        }

        auto* controlEvent = new ControlEvent;
        auto* strategyUpdateEvent = new StrategyUpdateEvent;
        loop.addEventListener(controlEvent,
                              [this](ev::Event* event){this->controlEventHandler(event);});
        loop.addEventListener(strategyUpdateEvent,
                              [this](ev::Event* event){this->strategyUpdateHandler(event);});

        loop.runEvery(0.01, [this]{this->moveAll();});

        loop.runEvery(0.2, [this] () -> void {
            Object::PosInfo smithPos;
            if(!getSmithPosition(smithPos)) return;
            AgentSmith::PredictingShellList shells = smith->getIncomingShells(smithPos);
            smith->ballisticsPredict(shells, globalSteps);
        });

        loop.runEvery(0.1, [this]() -> void {
            Object::PosInfo smithPos;
            if(!getSmithPosition(smithPos)) return;
            smith->getDodgeStrategy(smithPos, globalSteps);
        });

        loop.runEvery(1, [this] () -> void {
            Object::PosInfo smithPos, myPos;
            if(!getSmithPosition(smithPos) || !getMyPosition(myPos)) return;
            smith->attack(smithPos, myPos, globalSteps);
        });

        loop.loop();
        controlLoop = nullptr;
    }

    bool LocalController::getSmithPosition(Object::PosInfo& pos)
    {
        if(objects.find(2) == objects.end()) return false;
        Tank* smithTank = dynamic_cast<Tank*>(objects[2].get());
        pos = smithTank->getCurrentPosition();
        return true;
    }

    bool LocalController::getMyPosition(Object::PosInfo& pos)
    {
        if(objects.find(1) == objects.end()) return false;
        Tank* me = dynamic_cast<Tank*>(objects[1].get());
        pos = me->getCurrentPosition();
        return true;
    }

    std::vector<Object::PosInfo> LocalController::getRandomPositions(int num)
    {
        std::vector<Object::PosInfo> pos;
        std::unordered_set<std::pair<int, int>, PairHash> s;
        while(s.size() < num)
        {
            int x = util::getRandomNumber(0, HORIZON_GRID_NUMBER - 1);
            int y = util::getRandomNumber(0, VERTICAL_GRID_NUMBER - 1);
            s.insert(std::make_pair(x, y));
        }
        for(const auto& p: s)
        {
            int i = util::getRandomNumber(0, 3);
            pos.emplace_back(util::Vec(MAP_GRID_TO_REAL(p.first, p.second)), i * 90.0);
        }
        return pos;
    }

    void LocalController::dispatchEvent(ev::Event *event) {controlLoop->dispatchEvent(event);}

    LocalController::ObjectListPtr LocalController::getObjects()
    {
        std::lock_guard<std::mutex> lg(mu);
        return snapshot;
    }

    LocalController::BlockList* LocalController::getBlocks() {return &blocks;}

    void LocalController::controlEventHandler(ev::Event *event)
    {
        if(objects.find(1) == objects.end()) return;
        auto* ce = dynamic_cast<ControlEvent*>(event);
        Tank* me = dynamic_cast<Tank*>(objects[1].get());
        switch (ce->operation())
        {
            case ControlEvent::Forward: me->forward(true); break;
            case ControlEvent::Backward: me->backward(true); break;
            case ControlEvent::RotateCW: me->rotateCW(true); break;
            case ControlEvent::RotateCCW: me->rotateCCW(true); break;
            case ControlEvent::StopForward: me->forward(false); break;
            case ControlEvent::StopBackward: me->backward(false); break;
            case ControlEvent::StopRotateCW: me->rotateCW(false); break;
            case ControlEvent::StopRotateCCW: me->rotateCCW(false); break;
            case ControlEvent::Fire: fire(me); break;
        }
    }

    void LocalController::strategyUpdateHandler(ev::Event* event)
    {
        auto* updateEvent = dynamic_cast<StrategyUpdateEvent*>(event);
        Strategy* strategy = updateEvent->getStrategy();
        if(strategy->type() == Strategy::Dodge)
            smithDodgeStrategy.reset(dynamic_cast<DodgeStrategy*>(strategy));
        else if(strategy->type() == Strategy::Contact)
            smithContactStrategy.reset(dynamic_cast<ContactStrategy*>(strategy));
        else smithAttackStrategy.reset(dynamic_cast<AttackStrategy*>(strategy));
    }

    void LocalController::fire(Tank *tank)
    {
        if(tank->remainShells() == 0)
            return;
        std::unique_ptr<Object> shell(tank->makeShell());
        objects[shell->id()] = std::move(shell);

        if(objects.find(2) == objects.end()) return;
        Tank* smithTank = dynamic_cast<Tank*>(objects[2].get());
        AgentSmith::PredictingShellList shells = smith->getIncomingShells(smithTank->getCurrentPosition());
        smith->ballisticsPredict(shells, globalSteps);
    }

    static int danger = 0;

    void LocalController::moveAll()
    {
        globalSteps++;
        deletedObjs.clear();
        bool attacking = false;
        for(auto& entry: objects)
        {
            std::unique_ptr<Object>& obj = entry.second;
            if(obj->id() == 2)
            {
                auto* smithTank = dynamic_cast<Tank*>(obj.get());
                if(smithDodgeStrategy)
                {
                    if(smithDodgeStrategy->update(this, smithTank, globalSteps))
                    {
                        if(smithAttackStrategy) smithAttackStrategy->cancelAttack();
                        danger = 50;
                    }
                    else danger = danger == 0 ? danger : danger - 1;
                }
                if(smithAttackStrategy && !danger)
                {
                    if(smithAttackStrategy->update(this, smithTank, globalSteps))
                        attacking = true;
                }
                if(smithContactStrategy && !danger && !attacking)
                    smithContactStrategy->update(this, smithTank, globalSteps);
            }
            Object::PosInfo next = obj->getNextPosition(0, 0);
            Object::PosInfo cur = obj->getCurrentPosition();
            bool countdown = false;
            if(obj->type() == OBJ_SHELL)
            {
                auto* shell = dynamic_cast<Shell*>(obj.get());
                int id = checkShellCollision(cur, next);
                if(id < 0 || id > MAX_TANK_ID)
                {
                    obj->resetNextPosition(getBouncedPosition(cur, next, id));
                    countdown = true;
                }
                else if(id)
                {
                    if((id != shell->tankId() || shell->ttl() < Shell::INITIAL_TTL))
                    {
                        deletedObjs.push_back(id);
                        deletedObjs.push_back(shell->id());
                        restart(1.0);
                    }
                }
                if(countdown && shell->countDown() <= 0)
                {
                    deletedObjs.push_back(shell->id());
                    if(objects.find(shell->tankId()) != objects.end())
                        dynamic_cast<Tank*>(objects[shell->tankId()].get())->getRemainShell();
                }
            }
            else
            {
                auto* tank = dynamic_cast<Tank*>(obj.get());
                int id = checkTankBlockCollision(cur, next);
                if(id)
                    obj->resetNextPosition(cur);
            }
            obj->moveToNextPosition();
        }
        for(int id: deletedObjs)
            objects.erase(id);
        std::lock_guard<std::mutex> lg(mu);
        if(!snapshot.unique())
            snapshot.reset(new ObjectList);
        assert(snapshot.unique());
        snapshot->clear();
        for(auto& entry: objects)
        {
            std::unique_ptr<Object>& obj = entry.second;
            if(obj->type() == OBJ_TANK)
                (*snapshot)[obj->id()] = std::unique_ptr<Object>(
                        new Tank(*dynamic_cast<Tank*>(obj.get())));
            else
                (*snapshot)[obj->id()] = std::unique_ptr<Object>(
                        new Shell(*dynamic_cast<Shell*>(obj.get())));
        }
    }

    int LocalController::checkShellCollision(const Object::PosInfo& curPos, const Object::PosInfo& nextPos)
    {
        int collisionBlock = checkShellBlockCollision(curPos, nextPos);
        if(collisionBlock) return collisionBlock;
        return checkShellTankCollision(curPos, nextPos);
    }

    int LocalController::checkShellBlockCollision(const Object::PosInfo& curPos, const Object::PosInfo& nextPos)
    {
        if(nextPos.pos.x() < Shell::RADIUS + Block::BLOCK_WIDTH || nextPos.pos.x() > GAME_VIEW_WIDTH - 1 - Block::BLOCK_WIDTH)
            return VERTICAL_BORDER_ID;
        if(nextPos.pos.y() < Shell::RADIUS + Block::BLOCK_WIDTH || nextPos.pos.y() > GAME_VIEW_HEIGHT - 1 - Block::BLOCK_WIDTH)
            return HORIZON_BORDER_ID;

        util::Vec grid(MAP_REAL_TO_GRID(curPos.pos.x(), curPos.pos.y()));
        static double degreeRange[] = {0.0, 90.0, 180.0, 270.0, 360.0};
        static int directions[] = {RIGHT, UPWARDS_RIGHT, UPWARDS, UPWARDS_LEFT,
                                   LEFT, DOWNWARDS_LEFT, DOWNWARDS, DOWNWARDS_RIGHT};
        int dir = 0;
        for(int i = 0; i < 4; i++)
        {
            if(curPos.angle == degreeRange[i])
            {
                dir = directions[2 * i];
                break;
            }
            else if(curPos.angle > degreeRange[i] && curPos.angle < degreeRange[i + 1])
            {
                dir = directions[2 * i + 1];
                break;
            }
        }
        for(int blockId: shellPossibleCollisionBlocks[static_cast<int>(grid.x())][static_cast<int>(grid.y())][dir])
        {
            Block block = blocks[blockId];
            util::Vec v1 = block.isHorizon() ? util::Vec(1, 0) : util::Vec(0, 1);
            util::Vec v2 = block.isHorizon() ? util::Vec(0, 1) : util::Vec(1, 0);
            if(util::checkRectCircleCollision(v1, v2, block.center(), nextPos.pos,
                                              block.width(), block.height(), Shell::RADIUS))
                return blockId;
        }
        return 0;
    }

    int LocalController::checkShellTankCollision(const Object::PosInfo& curPos, const Object::PosInfo& nextPos)
    {
        for(int id = MIN_TANK_ID; id <= MAX_TANK_ID; id++)
        {
            if(objects.find(id) == objects.end())
                continue;
            Tank* tank = dynamic_cast<Tank*>(objects[id].get());
            auto axis = util::getUnitVectors(tank->getCurrentPosition().angle);
            if(util::checkRectCircleCollision(axis.first, axis.second, tank->getCurrentPosition().pos, nextPos.pos,
                                              Tank::TANK_WIDTH - 2, Tank::TANK_HEIGHT - 2, Shell::RADIUS))
                return id;
        }
        return 0;
    }

    int LocalController::checkTankBlockCollision(const Object::PosInfo& curPos, const Object::PosInfo& nextPos)
    {
        util::Vec grid(MAP_REAL_TO_GRID(curPos.pos.x(), curPos.pos.y()));
        for(int blockId: tankPossibleCollisionBlocks[static_cast<int>(grid.x())][static_cast<int>(grid.y())])
        {
            Block block = blocks[blockId];
            double blockAngle = block.isHorizon() ? 0.0 : 90.0;
            if(util::checkRectRectCollision(blockAngle, block.center(), block.width(), block.height(),
                                            nextPos.angle, nextPos.pos, Tank::TANK_WIDTH, Tank::TANK_HEIGHT))
                return blockId;
        }
        if(util::checkRectRectCollision(90.0, leftBorderCenter, 4.0, GAME_VIEW_HEIGHT, nextPos.angle, nextPos.pos, Tank::TANK_WIDTH, Tank::TANK_HEIGHT) ||
                util::checkRectRectCollision(90.0, rightBorderCenter, 4.0, GAME_VIEW_HEIGHT, nextPos.angle, nextPos.pos, Tank::TANK_WIDTH, Tank::TANK_HEIGHT))
            return VERTICAL_BORDER_ID;
        if(util::checkRectRectCollision(0.0, topBorderCenter, 4.0, GAME_VIEW_WIDTH, nextPos.angle, nextPos.pos, Tank::TANK_WIDTH, Tank::TANK_HEIGHT) ||
           util::checkRectRectCollision(0.0, bottomBorderCenter, 4.0, GAME_VIEW_WIDTH, nextPos.angle, nextPos.pos, Tank::TANK_WIDTH, Tank::TANK_HEIGHT))
            return VERTICAL_BORDER_ID;
        return 0;
    }

    Object::PosInfo LocalController::getBouncedPosition(const Object::PosInfo& cur, const Object::PosInfo& next, int blockId)
    {
        Object::PosInfo bounced = next;
        if(blockId < 0)
        {
            bounced.angle = (blockId == VERTICAL_BORDER_ID) ? util::angleFlipY(next.angle) : util::angleFlipX(next.angle);
            return bounced;
        }
        Block block = blocks[blockId];
        for(int i = 0; i < 4; i++)
        {
            std::pair<util::Vec, util::Vec> b = block.border(i);
            if(!util::intersectionOfSegments(cur.pos, next.pos, b.first, b.second, &bounced.pos))
                continue;
            if(i < 2)
                bounced.angle = util::angleFlipX(cur.angle);
            else
                bounced.angle = util::angleFlipY(cur.angle);
        }
        return bounced;
    }

    void LocalController::initBlocks()
    {
        maze.generate();
        auto blockPositions = maze.getBlockPositions();
        for(const auto& b: blockPositions)
        {
            Block block(util::Id::getBlockId(), b.first, b.second);
            assert(blocks.find(block.id()) == blocks.end());
            blocks[block.id()] = block;
            //将block添加到其相邻六个格子对应的碰撞可能列表中
            int gx = (block.start().x() - GRID_SIZE / 2 > 0) ?
                    MAP_REAL_X_TO_GRID_X(block.start().x() - GRID_SIZE / 2) : -1;
            int gy = (block.start().y() - GRID_SIZE / 2 > 0) ?
                     MAP_REAL_Y_TO_GRID_Y(block.start().y() - GRID_SIZE / 2) : -1;;
            if(block.isHorizon())
            {
                if(gx >= 0)
                {
                    //左上
                    shellPossibleCollisionBlocks[gx][gy][DOWNWARDS].push_back(block.id());
                    shellPossibleCollisionBlocks[gx][gy][DOWNWARDS_RIGHT].push_back(block.id());
                    shellPossibleCollisionBlocks[gx][gy][RIGHT].push_back(block.id());
                    tankPossibleCollisionBlocks[gx][gy].push_back(block.id());
                    //左下
                    gy += 1;
                    shellPossibleCollisionBlocks[gx][gy][UPWARDS].push_back(block.id());
                    shellPossibleCollisionBlocks[gx][gy][UPWARDS_RIGHT].push_back(block.id());
                    shellPossibleCollisionBlocks[gx][gy][RIGHT].push_back(block.id());
                    tankPossibleCollisionBlocks[gx][gy].push_back(block.id());
                    gy -= 1;
                }
                //上中
                gx += 1;
                shellPossibleCollisionBlocks[gx][gy][DOWNWARDS].push_back(block.id());
                shellPossibleCollisionBlocks[gx][gy][DOWNWARDS_RIGHT].push_back(block.id());
                shellPossibleCollisionBlocks[gx][gy][DOWNWARDS_LEFT].push_back(block.id());
                tankPossibleCollisionBlocks[gx][gy].push_back(block.id());
                //下中
                gy += 1;
                shellPossibleCollisionBlocks[gx][gy][UPWARDS].push_back(block.id());
                shellPossibleCollisionBlocks[gx][gy][UPWARDS_RIGHT].push_back(block.id());
                shellPossibleCollisionBlocks[gx][gy][UPWARDS_LEFT].push_back(block.id());
                tankPossibleCollisionBlocks[gx][gy].push_back(block.id());
                gy -= 1; gx += 1;
                if(gx < HORIZON_GRID_NUMBER)
                {
                    //右上
                    shellPossibleCollisionBlocks[gx][gy][DOWNWARDS].push_back(block.id());
                    shellPossibleCollisionBlocks[gx][gy][DOWNWARDS_LEFT].push_back(block.id());
                    shellPossibleCollisionBlocks[gx][gy][LEFT].push_back(block.id());
                    tankPossibleCollisionBlocks[gx][gy].push_back(block.id());
                    //右下
                    gy += 1;
                    shellPossibleCollisionBlocks[gx][gy][UPWARDS].push_back(block.id());
                    shellPossibleCollisionBlocks[gx][gy][UPWARDS_LEFT].push_back(block.id());
                    shellPossibleCollisionBlocks[gx][gy][LEFT].push_back(block.id());
                    tankPossibleCollisionBlocks[gx][gy].push_back(block.id());
                }
            }
            else
            {
                if(gy >= 0)
                {
                    //左上
                    shellPossibleCollisionBlocks[gx][gy][DOWNWARDS].push_back(block.id());
                    shellPossibleCollisionBlocks[gx][gy][DOWNWARDS_RIGHT].push_back(block.id());
                    shellPossibleCollisionBlocks[gx][gy][RIGHT].push_back(block.id());
                    tankPossibleCollisionBlocks[gx][gy].push_back(block.id());
                    //右上
                    gx += 1;
                    shellPossibleCollisionBlocks[gx][gy][DOWNWARDS].push_back(block.id());
                    shellPossibleCollisionBlocks[gx][gy][DOWNWARDS_LEFT].push_back(block.id());
                    shellPossibleCollisionBlocks[gx][gy][LEFT].push_back(block.id());
                    tankPossibleCollisionBlocks[gx][gy].push_back(block.id());
                    gx -= 1;
                }
                //左中
                gy += 1;
                shellPossibleCollisionBlocks[gx][gy][RIGHT].push_back(block.id());
                shellPossibleCollisionBlocks[gx][gy][UPWARDS_RIGHT].push_back(block.id());
                shellPossibleCollisionBlocks[gx][gy][DOWNWARDS_RIGHT].push_back(block.id());
                tankPossibleCollisionBlocks[gx][gy].push_back(block.id());
                //右中
                gx += 1;
                shellPossibleCollisionBlocks[gx][gy][LEFT].push_back(block.id());
                shellPossibleCollisionBlocks[gx][gy][UPWARDS_LEFT].push_back(block.id());
                shellPossibleCollisionBlocks[gx][gy][DOWNWARDS_LEFT].push_back(block.id());
                tankPossibleCollisionBlocks[gx][gy].push_back(block.id());
                gx -= 1; gy += 1;
                if(gy < VERTICAL_GRID_NUMBER)
                {
                    //左下
                    shellPossibleCollisionBlocks[gx][gy][UPWARDS].push_back(block.id());
                    shellPossibleCollisionBlocks[gx][gy][RIGHT].push_back(block.id());
                    shellPossibleCollisionBlocks[gx][gy][UPWARDS_RIGHT].push_back(block.id());
                    tankPossibleCollisionBlocks[gx][gy].push_back(block.id());
                    //右下
                    gx += 1;
                    shellPossibleCollisionBlocks[gx][gy][UPWARDS].push_back(block.id());
                    shellPossibleCollisionBlocks[gx][gy][LEFT].push_back(block.id());
                    shellPossibleCollisionBlocks[gx][gy][UPWARDS_LEFT].push_back(block.id());
                    tankPossibleCollisionBlocks[gx][gy].push_back(block.id());
                }
            }
        }
    }
}
