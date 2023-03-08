//
// Created by zr on 23-2-16.
//

#ifndef TANK_TROUBLE_WINDOW_H
#define TANK_TROUBLE_WINDOW_H
#include <gtkmm.h>
#include <memory>
#include "view/EntryView.h"
#include "view/LoginView.h"
#include "view/GameArea.h"
#include "Controller.h"

namespace TankTrouble
{
    class Window : public Gtk::Window
    {
    public:
        Window();
        ~Window() override = default;
        bool on_key_press_event(GdkEventKey* key_event) override;
        bool on_key_release_event(GdkEventKey* key_event) override;

    private:
        EntryView entryView;
        LoginView loginView;
        void onUserChooseLocal();
        void onUserChooseOnline();
        void onUserLogin();

        std::unique_ptr<Controller> ctl;
        std::unique_ptr<GameArea> gameArea;

        bool KeyUpPressed, KeyDownPressed, KeyLeftPressed, KeyRightPressed;
        bool spacePressed;
    };
}

#endif //TANK_TROUBLE_WINDOW_H
