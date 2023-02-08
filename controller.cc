//
// Created by zr on 23-2-8.
//

#include "controller.h"
#include "tank.h"

tank::Controller::Controller(): objects(new tank::Controller::ObjectList)
{
    objects->push_back(std::unique_ptr<Object>(new Tank(util::Cord(100, 100), 90.0, RED)));
}

void tank::Controller::start()
{

}

tank::Controller::ObjectListPtr tank::Controller::getObjects()
{
    std::lock_guard<std::mutex> lg(mu);
    return objects;
}

tank::Controller::BarrierList tank::Controller::getBarriers()
{

}

void tank::Controller::moveAll()
{

}