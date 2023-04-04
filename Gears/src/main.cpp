#include <engine/VkEngineApp.hpp>

#include <iostream>

class GearsApp : public vke::VkEngineApp {
};

int main(int argc, char* argv[]) {
    std::cout << "[GEARS]: Launching Gears\n";

    GearsApp app{}; 

    std::cout << "[GEARS]: Bye!\n";
    return 0;
}
