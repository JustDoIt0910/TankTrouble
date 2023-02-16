//
// Created by zr on 23-2-15.
//

#ifndef EV_EVENT_H
#define EV_EVENT_H
#include <string>

namespace ev
{
    class Event
    {
    public:
        explicit Event(const std::string& name, uint16_t type = 0);
        [[nodiscard]] std::string name() const;
        [[nodiscard]] uint16_t type() const;
        virtual ~Event() = default;
    private:
        std::string _name;
        uint16_t _type;
    };
}

#endif //EV_EVENT_H
