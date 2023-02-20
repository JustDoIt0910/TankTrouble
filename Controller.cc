//
// Created by zr on 23-2-8.
//

#include "defs.h"
#include "Controller.h"
#include "util/Math.h"
#include "util/Id.h"
#include "event/ControlEvent.h"
#include "Tank.h"
#include "Shell.h"
#include "AgentSmith.h"
#include <thread>
#include <cassert>
#include <iostream>

namespace TankTrouble
{
    static util::Vec topBorderCenter(static_cast<double>(WINDOW_WIDTH) / 2, 2.0);
    static util::Vec leftBorderCenter(2.0, static_cast<double>(WINDOW_HEIGHT) / 2);
    static util::Vec bottomBorderCenter(static_cast<double>(WINDOW_WIDTH) / 2, static_cast<double>(WINDOW_HEIGHT) - 2 - 1);
    static util::Vec rightBorderCenter(static_cast<double>(WINDOW_WIDTH) - 2 - 1, static_cast<double>(WINDOW_HEIGHT) / 2);

    Controller::Controller():
        started(false),
        controlLoop(nullptr),
        snapshot(new ObjectList),
        tankNum(0)
    {
        initBlocks(45);
        std::unique_ptr<Object> tank(new Tank(util::Vec(100, 100), 90.0, RED));
        objects[tank->id()] = std::move(tank);
        tankNum++;
    }

    Controller::~Controller()
    {
        controlLoop->quit();
        if(controlThread.joinable())
            controlThread.join();
    }

    void Controller::start()
    {
        controlThread = std::thread(&Controller::run, this);
        std::unique_lock<std::mutex> lk(mu);
        cv.wait(lk, [this] () -> bool {return this->started;});
    }

    void Controller::run()
    {
        ev::reactor::EventLoop loop;
        controlLoop = &loop;
        {
            std::unique_lock<std::mutex> lk(mu);
            started = true;
            cv.notify_all();
        }
        auto* event = new ControlEvent;
        loop.addEventListener(event, [this](ev::Event* event){this->controlEventHandler(event);});
        loop.runEvery(0.02, [this]{this->moveAll();});
        loop.loop();
        controlLoop = nullptr;
    }

    void Controller::dispatchEvent(ev::Event *event) {controlLoop->dispatchEvent(event);}

    Controller::ObjectListPtr Controller::getObjects()
    {
        std::lock_guard<std::mutex> lg(mu);
        return snapshot;
    }

    Controller::BlockList* Controller::getBlocks() {return &blocks;}

    void Controller::controlEventHandler(ev::Event *event)
    {
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

    void Controller::fire(Tank *tank)
    {
        if(tank->remainShells() == 0)
            return;
        std::unique_ptr<Object> shell(tank->makeShell());
        objects[shell->id()] = std::move(shell);
    }

    void Controller::moveAll()
    {
        ev::Timestamp before = ev::Timestamp::now();
        deletedObjs.clear();
        for(auto& entry: objects)
        {
            std::unique_ptr<Object>& obj = entry.second;
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
                    if(id != shell->tankId() || shell->ttl() < Shell::INITIAL_TTL)
                    {
                        deletedObjs.push_back(id);
                        deletedObjs.push_back(shell->id());
                        if(id == 1)
                        {
                            auto* event = new ControlEvent;
                            controlLoop->removeEventListener(event);
                        }
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
                int id = checkTankBlockCollision(tank, cur, next);
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
        ev::Timestamp after = ev::Timestamp::now();
        //std::cout << after - before << std::endl;
    }

    int Controller::checkShellCollision(const Object::PosInfo& curPos, const Object::PosInfo& nextPos)
    {
        int collisionBlock = checkShellBlockCollision(curPos, nextPos);
        if(collisionBlock) return collisionBlock;
        return checkShellTankCollision(curPos, nextPos);
    }

    int Controller::checkShellBlockCollision(const Object::PosInfo& curPos, const Object::PosInfo& nextPos)
    {
        if(nextPos.pos.x() < Shell::RADIUS + Block::BLOCK_WIDTH || nextPos.pos.x() > WINDOW_WIDTH - 1 - Block::BLOCK_WIDTH)
            return VERTICAL_BORDER_ID;
        if(nextPos.pos.y() < Shell::RADIUS + Block::BLOCK_WIDTH || nextPos.pos.y() > WINDOW_HEIGHT - 1 - Block::BLOCK_WIDTH)
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
        for(int blockId: possibleCollisionBlocks[static_cast<int>(grid.x())][static_cast<int>(grid.y())][dir])
        {
            Block block = blocks[blockId];
            util::Vec v1 = block.isHorizon() ? util::Vec(1, 0) : util::Vec(0, 1);
            util::Vec v2 = block.isHorizon() ? util::Vec(0, 1) : util::Vec(1, 0);
            if(util::checkRectCircleCollision(v1, v2, block.center(), nextPos.pos,
                                              block.width(), block.height(), Shell::RADIUS))
            {
                return blockId;
            }
        }
        return 0;
    }

    int Controller::checkShellTankCollision(const Object::PosInfo& curPos, const Object::PosInfo& nextPos)
    {
        for(int id = MIN_TANK_ID; id <= MAX_TANK_ID; id++)
        {
            if(objects.find(id) == objects.end())
                continue;
            Tank* tank = dynamic_cast<Tank*>(objects[id].get());
            auto axis = util::getUnitVectors(tank->getCurrentPosition().angle);
            if(util::checkRectCircleCollision(axis.first, axis.second, tank->getCurrentPosition().pos, nextPos.pos,
                                              Tank::TANK_WIDTH, Tank::TANK_HEIGHT, Shell::RADIUS))
            {
                std::cout << "boom" << std::endl;
                return id;
            }
        }
        return 0;
    }

    int Controller::checkTankBlockCollision(Tank* tank, const Object::PosInfo& curPos, const Object::PosInfo& nextPos)
    {
        util::Vec grid(MAP_REAL_TO_GRID(curPos.pos.x(), curPos.pos.y()));
        for (int i = 0; i < 7; ++i)
        {
            for(int blockId: possibleCollisionBlocks[static_cast<int>(grid.x())][static_cast<int>(grid.y())][i])
            {
                Block block = blocks[blockId];
                double blockAngle = block.isHorizon() ? 0.0 : 90.0;
                if(util::checkRectRectCollision(blockAngle, block.center(), block.width(), block.height(),
                                                nextPos.angle, nextPos.pos, Tank::TANK_WIDTH, Tank::TANK_HEIGHT))
                    return blockId;
            }
        }
        if(util::checkRectRectCollision(90.0, leftBorderCenter, 4.0, WINDOW_HEIGHT, nextPos.angle, nextPos.pos, Tank::TANK_WIDTH, Tank::TANK_HEIGHT) ||
                util::checkRectRectCollision(90.0, rightBorderCenter, 4.0, WINDOW_HEIGHT, nextPos.angle, nextPos.pos, Tank::TANK_WIDTH, Tank::TANK_HEIGHT))
            return VERTICAL_BORDER_ID;
        if(util::checkRectRectCollision(0.0, topBorderCenter, 4.0, WINDOW_WIDTH, nextPos.angle, nextPos.pos, Tank::TANK_WIDTH, Tank::TANK_HEIGHT) ||
           util::checkRectRectCollision(0.0, bottomBorderCenter, 4.0, WINDOW_WIDTH, nextPos.angle, nextPos.pos, Tank::TANK_WIDTH, Tank::TANK_HEIGHT))
            return VERTICAL_BORDER_ID;
        return 0;
    }

    Object::PosInfo Controller::getBouncedPosition(const Object::PosInfo& cur, const Object::PosInfo& next, int blockId)
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

    void Controller::initBlocks(int num)
    {
        auto startEnds = util::getRandomBlocks(num);
        for(const auto& p: startEnds)
        {
            util::Vec startVec(MAP_GRID_TO_REAL(p.first.x(), p.first.y()));
            util::Vec endVec(MAP_GRID_TO_REAL(p.second.x(), p.second.y()));
            Block block(util::Id::getBlockId(), startVec, endVec);
            assert(blocks.find(block.id()) == blocks.end());
            blocks[block.id()] = block;
            //将block添加到其相邻六个格子对应的碰撞可能列表中
            int gx = static_cast<int>(p.first.x()) - 1;
            int gy = static_cast<int>(p.first.y()) - 1;
            if(block.isHorizon())
            {
                if(gx >= 0)
                {
                    //左上
                    possibleCollisionBlocks[gx][gy][DOWNWARDS].push_back(block.id());
                    possibleCollisionBlocks[gx][gy][DOWNWARDS_RIGHT].push_back(block.id());
                    possibleCollisionBlocks[gx][gy][RIGHT].push_back(block.id());
                    //左下
                    gy += 1;
                    possibleCollisionBlocks[gx][gy][UPWARDS].push_back(block.id());
                    possibleCollisionBlocks[gx][gy][UPWARDS_RIGHT].push_back(block.id());
                    possibleCollisionBlocks[gx][gy][RIGHT].push_back(block.id());
                    gy -= 1;
                }
                //上中
                gx += 1;
                possibleCollisionBlocks[gx][gy][DOWNWARDS].push_back(block.id());
                possibleCollisionBlocks[gx][gy][DOWNWARDS_RIGHT].push_back(block.id());
                possibleCollisionBlocks[gx][gy][DOWNWARDS_LEFT].push_back(block.id());
                //下中
                gy += 1;
                possibleCollisionBlocks[gx][gy][UPWARDS].push_back(block.id());
                possibleCollisionBlocks[gx][gy][UPWARDS_RIGHT].push_back(block.id());
                possibleCollisionBlocks[gx][gy][UPWARDS_LEFT].push_back(block.id());
                gy -= 1; gx += 1;
                if(gx < HORIZON_GRID_NUMBER)
                {
                    //右上
                    possibleCollisionBlocks[gx][gy][DOWNWARDS].push_back(block.id());
                    possibleCollisionBlocks[gx][gy][DOWNWARDS_LEFT].push_back(block.id());
                    possibleCollisionBlocks[gx][gy][LEFT].push_back(block.id());
                    //右下
                    gy += 1;
                    possibleCollisionBlocks[gx][gy][UPWARDS].push_back(block.id());
                    possibleCollisionBlocks[gx][gy][UPWARDS_LEFT].push_back(block.id());
                    possibleCollisionBlocks[gx][gy][LEFT].push_back(block.id());
                }
            }
            else
            {
                if(gy >= 0)
                {
                    //左上
                    possibleCollisionBlocks[gx][gy][DOWNWARDS].push_back(block.id());
                    possibleCollisionBlocks[gx][gy][DOWNWARDS_RIGHT].push_back(block.id());
                    possibleCollisionBlocks[gx][gy][RIGHT].push_back(block.id());
                    //右上
                    gx += 1;
                    possibleCollisionBlocks[gx][gy][DOWNWARDS].push_back(block.id());
                    possibleCollisionBlocks[gx][gy][DOWNWARDS_LEFT].push_back(block.id());
                    possibleCollisionBlocks[gx][gy][LEFT].push_back(block.id());
                    gx -= 1;
                }
                //左中
                gy += 1;
                possibleCollisionBlocks[gx][gy][RIGHT].push_back(block.id());
                possibleCollisionBlocks[gx][gy][UPWARDS_RIGHT].push_back(block.id());
                possibleCollisionBlocks[gx][gy][DOWNWARDS_RIGHT].push_back(block.id());
                //右中
                gx += 1;
                possibleCollisionBlocks[gx][gy][LEFT].push_back(block.id());
                possibleCollisionBlocks[gx][gy][UPWARDS_LEFT].push_back(block.id());
                possibleCollisionBlocks[gx][gy][DOWNWARDS_LEFT].push_back(block.id());
                gx -= 1; gy += 1;
                if(gy < VERTICAL_GRID_NUMBER)
                {
                    //左下
                    possibleCollisionBlocks[gx][gy][UPWARDS].push_back(block.id());
                    possibleCollisionBlocks[gx][gy][RIGHT].push_back(block.id());
                    possibleCollisionBlocks[gx][gy][UPWARDS_RIGHT].push_back(block.id());
                    //右下
                    gx += 1;
                    possibleCollisionBlocks[gx][gy][UPWARDS].push_back(block.id());
                    possibleCollisionBlocks[gx][gy][LEFT].push_back(block.id());
                    possibleCollisionBlocks[gx][gy][UPWARDS_LEFT].push_back(block.id());
                }
            }
        }
    }
}