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
        sigc::signal<void> signal_quit_game();

    private:
        void onQuit();

        Controller* ctl;
        GameArea gameArea;
        std::vector<PlayerInfoItem> playerInfoItems;
        Gtk::Button quitBtn;
        sigc::signal<void> quit_s;
    };
}

#endif //TANK_TROUBLE_GAME_VIEW_H
