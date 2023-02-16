//
// Created by zr on 23-2-8.
//

#ifndef TANK_TROUBLE_VIEW_H
#define TANK_TROUBLE_VIEW_H
#include <gtkmm/drawingarea.h>
#include "Controller.h"

namespace TankTrouble
{
    class View : public Gtk::DrawingArea
    {
    public:
        explicit View(Controller*);
    private:
        //Override default signal handler:
        bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
        bool on_timeout();
        Controller* ctl;
    };
}

#endif //TANK_TROUBLE_VIEW_H
