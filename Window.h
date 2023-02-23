//
// Created by zr on 23-2-16.
//

#ifndef TANK_TROUBLE_WINDOW_H
#define TANK_TROUBLE_WINDOW_H
#include <gtkmm.h>
#include "Controller.h"

namespace TankTrouble
{
    class Window : public Gtk::Window
    {
    public:
        explicit Window(Controller* ctl);
        ~Window() override = default;
        bool on_key_press_event(GdkEventKey* key_event) override;
        bool on_key_release_event(GdkEventKey* key_event) override;

    private:
        Controller* ctl;
        bool KeyUpPressed, KeyDownPressed, KeyLeftPressed, KeyRightPressed;
        bool spacePressed;
        //testing
        bool KeyDPressed;
    };
}

#endif //TANK_TROUBLE_WINDOW_H
