//
// Created by zr on 23-2-8.
//

#ifndef TANK_TROUBLE_CONTROLLER_H
#define TANK_TROUBLE_CONTROLLER_H
#include <memory>
#include <vector>
#include <mutex>
#include "object.h"
#include "util/cord.h"

namespace tank {

    class Controller {
    public:
        typedef std::unique_ptr<Object> ObjectPtr;
        typedef std::vector<ObjectPtr> ObjectList;
        typedef std::shared_ptr<ObjectList> ObjectListPtr;

        typedef std::vector<std::vector<util::Cord>> BarrierList;

        Controller();
        void start();
        ObjectListPtr getObjects();
        BarrierList getBarriers();

    private:
        void moveAll();
        ObjectListPtr objects;
        std::mutex mu;
    };

}

#endif //TANK_TROUBLE_CONTROLLER_H
