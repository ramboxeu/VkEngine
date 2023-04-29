#define SDL_SET_MAIN_HANDLED

#include <engine/VkEngineApp.hpp>
#include <engine/EngineResult.hpp>

#include <iostream>

class GearsApp : public vke::VkEngineApp {
};

int main(int argc, char* argv[]) {
    std::cout << "[GEARS]: Launching Gears\n";

    GearsApp app{};

    if (vke::EngineResult<void> result = app.create(1024, 720, "Gears"); !result) {
        std::cerr << "[GEARS] [FATAL]: " << result.getError() << '\n';
        return 1;
    }

    if (vke::EngineResult<void> result = app.run(); !result) {
        std::cerr << "[GEARS] [FATAL]: " << result.getError() << '\n';
        return 1;
    }

    std::cout << "[GEARS]: Bye!\n";
    return 0;
}
