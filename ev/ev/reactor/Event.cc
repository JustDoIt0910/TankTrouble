//
// Created by zr on 23-2-15.
//

#include "Event.h"

namespace ev
{
    Event::Event(const std::string& name, uint16_t type):
        _name(name), _type(type){}

    std::string Event::name() const {return _name;}

    uint16_t Event::type() const {return _type;}
}