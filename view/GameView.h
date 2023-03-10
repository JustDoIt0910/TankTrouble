//
// Created by zr on 23-3-3.
//

#ifndef TANK_TROUBLE_GAME_VIEW_H
#define TANK_TROUBLE_GAME_VIEW_H
#include <gtkmm.h>
#include "GameArea.h"

namespace TankTrouble
{
    class GameView : public Gtk::Fixed
    {
    public:
        explicit GameView(Controller* ctl);

    private:
        GameArea gameArea;
    };
}

#endif //TANK_TROUBLE_GAME_VIEW_H
