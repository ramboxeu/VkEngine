#define SDL_SET_MAIN_HANDLED

#include <engine/VkEngineApp.hpp>

#include <iostream>

class GearsApp : public vke::VkEngineApp {
};

int main(int argc, char* argv[]) {
    std::cout << "[GEARS]: Launching Gears\n";

    GearsApp app{};
    if (app.create(1024, 720, "Gears"))
        app.run();

    std::cout << "[GEARS]: Bye!\n";
    return 0;
}
