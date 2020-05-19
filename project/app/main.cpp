#include "core/app.h"
#include "components/components.h"

int main() // NOLINT
{
    try
    {
        fc::App app;
        std::cout << "Bot session successfully started!\n";
        app.add_component<fc::DevCommands>();
        app.add_component<fc::Repeat>();
        app.add_component<fc::DiceRoll>();
        std::cout << "All components initialized!\nPress Enter to stop...";
        std::cin.get();
        std::cout << "Stopping...\n";
    }
    catch (...)
    {
        mirai::error_logger();
    }
    return 0;
}
