#include "app.h"

#include <fstream>
#include <sstream>
#include <filesystem>

namespace fc
{
    void App::dispatch_event(const mirai::Event& event)
    {
        std::shared_lock lock(components_mutex_);
        for (auto& comp : components_)
            comp.on_event(session_, event);
    }

    App::App()
    {
        std::ifstream fs("config.json");
        std::stringstream ss;
        ss << fs.rdbuf();
        {
            const auto config = config_.to_write();
            *config = json::parse(ss.str());
            session_ = mirai::Session(config->auth_key, config->bot_id);
        }
        session_.config({}, true);
        session_.subscribe_all_events(
            [&](const mirai::Event& event) { dispatch_event(event); },
            mirai::error_logger,
            mirai::ExecutionPolicy::thread_pool);
    }

    App::~App() noexcept
    {
        try { save_data(); }
        catch (...) {}
    }

    void App::add_component(Component&& component)
    {
        std::unique_lock lock(components_mutex_);
        Component& comp = components_.emplace_back(std::move(component));
        const auto config = config_.to_read();
        comp.load_data(*config);
    }

    void App::save_data()
    {
        {
            const auto config = config_.to_write();
            for (const auto& comp : components_)
                comp.save_data(*config);
        }
        namespace fs = std::filesystem;
        fs::remove("config-backup.json");
        fs::rename("config.json", "config-backup.json");
        std::ofstream stream("config.json");
        {
            const auto config = config_.to_read();
            stream << json(*config).dump(2);
        }
    }
}
