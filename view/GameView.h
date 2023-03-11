//
// Created by zr on 23-3-3.
//

#ifndef TANK_TROUBLE_GAME_VIEW_H
#define TANK_TROUBLE_GAME_VIEW_H
#include <gtkmm.h>
#include "view/component/GameArea.h"
#include "view/component/PlayerInfoItem.h"

namespace TankTrouble
{
    class GameView : public Gtk::Fixed
    {
    public:
        explicit GameView(Controller* ctl);
        bool getPlayersInfo();

    private:
        Controller* ctl;
        GameArea gameArea;
        std::vector<PlayerInfoItem> playerInfoItems;
    };
}

#endif //TANK_TROUBLE_GAME_VIEW_H
