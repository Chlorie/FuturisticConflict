#include "app.h"
#include <fstream>
#include <sstream>

namespace fc
{
    void App::dispatch_event(const mirai::Event& event)
    {
        for (Component& comp : components_)
            comp.on_event(session_, event);
    }

    App::App()
    {
        std::ifstream fs("config.json");
        std::stringstream ss;
        ss << fs.rdbuf();
        config_ = json::parse(ss.str());

        const mirai::uid_t bot_uid = config_["botUid"].get<mirai::uid_t>();
        const std::string& auth_key = config_["authKey"].get_ref<const std::string&>();
        session_ = mirai::Session(auth_key, bot_uid);

        session_.config({}, true);
        session_.subscribe_all_events(
            [&](const mirai::Event& event) { dispatch_event(event); },
            mirai::error_logger,
            mirai::ExecutionPolicy::thread_pool);
    }

    void App::add_component(Component&& component)
    {
        components_.emplace_back(std::move(component));
    }
}
