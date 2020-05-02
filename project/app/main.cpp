#include "core/app.h"

struct PingPongComponent final
{
    static void on_event(const mirai::Session& sess, const mirai::Event& event)
    {
        event.dispatch([&](const mirai::FriendMessage& msg)
        {
            if (msg.message.content == "!ping")
                sess.send_message(msg.sender.id, "pong!");
        });
    }
};

int main() // NOLINT
{
    try
    {
        fc::App app;
        app.add_component<PingPongComponent>();
        std::cin.get();
    }
    catch (...)
    {
        mirai::error_logger();
    }
    return 0;
}
