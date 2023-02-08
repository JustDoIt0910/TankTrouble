#include <gtkmm/application.h>
#include <gtkmm/window.h>
#include "field.h"
#include "controller.h"

int main(int argc, char* argv[]) {
    auto app = Gtk::Application::create(argc, argv, "my.tank.trouble");

    Gtk::Window win;
    win.set_title("TankTrouble");
    win.set_default_size(600, 400);
    win.set_resizable(false);

    tank::Controller ctl;
    tank::Field f(&ctl);
    win.add(f);
    f.show();

    return app->run(win);
    return 0;
}
