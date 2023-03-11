#include <gtkmm/application.h>
#include "Window.h"

int main(int argc, char* argv[]) {
    auto app = Gtk::Application::create(argc, argv, "my.tank.trouble4");
    TankTrouble::Window win;
    return app->run(win);
}
