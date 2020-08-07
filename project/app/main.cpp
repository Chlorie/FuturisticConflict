#include "core/app.h"
#include "components/components.h"

int main() // NOLINT
{
    try
    {
        fc::App app;
        std::cout << "Bot session successfully started!\n";
        app.add_component<fc::Chinitsu>();
        app.add_component<fc::DevCommands>();
        app.add_component<fc::UserCommands>();
        app.add_component<fc::OthelloGame>();
        app.add_component<fc::UtttGame>();
        app.add_component<fc::Repeat>();
        app.add_component<fc::AntiRecall>();
        app.add_component<fc::DiceRoll>();
        app.add_component<fc::RandomUtils>();
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
