#define SDL_SET_MAIN_HANDLED

#include <engine/VkEngineApp.hpp>

#include <iostream>

class GearsApp : public vke::VkEngineApp {
};

int main(int argc, char* argv[]) {
    std::cout << "[GEARS]: Launching Gears\n";

    GearsApp app{};
    vke::utils::Result<void, vke::EngineError> result = app.create(1024, 720, "Gears");

    if (!result) {
        std::cerr << "[GEARS] [FATAL]: " << result.getError() << '\n';
        return  1;
    }

    app.run();

    std::cout << "[GEARS]: Bye!\n";
    return 0;
}
