#pragma once

#include "config.h"
#include "component.h"
#include "singleton.h"

namespace fc
{
    class App final : public Singleton<App>
    {
    private:
        GuardedData<Config> config_;
        mirai::Session session_;

        mutable std::shared_mutex components_mutex_;
        std::vector<Component> components_;

        void dispatch_event(const mirai::Event& event);

    public:
        App();
        ~App() noexcept;

        void add_component(Component&& component);

        template <typename T, typename... Ts>
        void add_component(Ts&&... args) { add_component(make_component<T>(std::forward<Ts>(args)...)); }

        void save_data();

        [[nodiscard]] auto config() { return config_.to_write(); }
        [[nodiscard]] auto config() const { return config_.to_read(); }
    };
}
