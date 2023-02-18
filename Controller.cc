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
#include <thread>
#include <cassert>
#include <iostream>
#include <stdio.h>

namespace TankTrouble
{
    Controller::Controller():
        started(false),
        controlLoop(nullptr),
        snapshot(new ObjectList)
    {
        objects.push_back(std::unique_ptr<Object>(new Tank(util::Vec(100, 100), 90.0, RED)));
        initBlocks(45);
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
int tid;
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
        tid = loop.runEvery(0.05, [this]{this->moveAll();});
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
        Tank* me = dynamic_cast<Tank*>(objects[0].get());
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
        objects.emplace_back(std::unique_ptr<Object>(tank->makeShell()));
    }

    void Controller::moveAll()
    {
        ev::Timestamp before = ev::Timestamp::now();
        for(auto& obj: objects)
        {
            Object::PosInfo next = obj->getNextPosition(0, 0);
            Object::PosInfo cur = obj->getCurrentPosition();
            if(obj->type() == OBJ_SHELL)
            {
                if(int id = checkShellCollision(cur, next))
                    if(id)
                    {
                        std::cout << "collision " << id << std::endl;
                        controlLoop->cancelTimer(tid);
                    }
            }
            obj->moveToNextPosition();
        }
        std::lock_guard<std::mutex> lg(mu);
        if(!snapshot.unique())
            snapshot.reset(new ObjectList);
        assert(snapshot.unique());
        snapshot->clear();
        for(auto& obj: objects)
        {
            if(obj->type() == OBJ_TANK)
                snapshot->emplace_back(std::unique_ptr<Object>(
                        new Tank(*dynamic_cast<Tank*>(obj.get()))));
            else
                snapshot->emplace_back(std::unique_ptr<Object>(
                        new Shell(*dynamic_cast<Shell*>(obj.get()))));
        }
        ev::Timestamp after = ev::Timestamp::now();
        //std::cout << after - before << std::endl;
    }

    int Controller::checkShellCollision(const Object::PosInfo& curPos, const Object::PosInfo& nextPos)
    {
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
        //printf("{x = %f, y = %f, gx = %d, gy = %d, dir = %d}\n",curPos.pos.x(), curPos.pos.y(), static_cast<int>(grid.x()), static_cast<int>(grid.y()), dir);
        for(int blockId: possibleCollisionBlocks[static_cast<int>(grid.x())][static_cast<int>(grid.y())][dir])
        {
            std::cout << blockId << ": ";
            Block block = blocks[blockId];
            util::Vec v1 = block.isHorizon() ? util::Vec(1, 0) : util::Vec(0, 1);
            util::Vec v2 = block.isHorizon() ? util::Vec(0, 1) : util::Vec(1, 0);
            if(util::checkRectCircleCollision(v1, v2, block.center(), nextPos.pos,
                                              block.width(), block.height(), Shell::RADIUS))
            {
                return blockId;
            }
        }
        std::cout << std::endl;
        return 0;
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
