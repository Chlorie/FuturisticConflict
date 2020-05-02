#pragma once

#include <nlohmann/json.hpp>
#include "component.h"

namespace fc
{
    using json = nlohmann::json;

    class App final
    {
    private:
        json config_;
        mirai::Session session_;
        std::vector<Component> components_;

        void dispatch_event(const mirai::Event& event);

    public:
        App();

        void add_component(Component&& component);

        template <typename T, typename... Ts>
        void add_component(Ts&&... args)
        {
            components_.emplace_back(
                make_component<T>(std::forward<Ts>(args)...));
        }
    };
}
