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
        void load_data();

        template <typename Func,
            std::void_t<std::invoke_result_t<const Func&, Component&>>* = nullptr>
        void for_each_component(const Func& func)
        {
            std::shared_lock lock(components_mutex_);
            for (Component& comp : components_)
                func(comp);
        }

        template <typename Func,
            std::enable_if_t<!std::is_same_v<std::invoke_result_t<Func&&, Component&>, void>>* = nullptr>
        auto for_component(const std::string_view name, Func&& func)
        -> std::optional<std::invoke_result_t<Func&&, Component&>>
        {
            std::shared_lock lock(components_mutex_);
            for (Component& comp : components_)
                if (comp.name() == name)
                    return std::forward<Func>(func)(comp);
            return {};
        }

        template <typename Func,
            std::enable_if_t<std::is_void_v<std::invoke_result_t<Func&&, Component&>>>* = nullptr>
        bool for_component(const std::string_view name, Func&& func)
        {
            std::shared_lock lock(components_mutex_);
            for (Component& comp : components_)
                if (comp.name() == name)
                {
                    std::forward<Func>(func)(comp);
                    return true;
                }
            return false;
        }

        [[nodiscard]] auto config() { return config_.to_write(); }
        [[nodiscard]] auto config() const { return config_.to_read(); }
    };
}
