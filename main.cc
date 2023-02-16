#include <gtkmm/application.h>
#include "Window.h"
#include "View.h"
#include "Controller.h"

int main(int argc, char* argv[]) {
    auto app = Gtk::Application::create(argc, argv, "my.tank.trouble");
    TankTrouble::Controller ctl;
    TankTrouble::Window win(&ctl);
    TankTrouble::View view(&ctl);
    win.add(view);
    view.show();
    ctl.start();
    return app->run(win);
}
