//
// Created by zr on 23-3-8.
//

#ifndef TANK_TROUBLE_LOGIN_VIEW_H
#define TANK_TROUBLE_LOGIN_VIEW_H
#include <gtkmm.h>


namespace TankTrouble
{
    class LoginView : public Gtk::Fixed
    {
    public:
        LoginView();

    private:
        Gtk::Button loginBtn;
        Gtk::Entry nicknameEntry;
    };
}

#endif //TANK_TROUBLE_LOGIN_VIEW_H
