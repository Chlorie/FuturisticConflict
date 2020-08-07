#pragma once

#include "../core/component.h"

namespace fc
{
    class RandomUtils final : public ComponentBase
    {
    public:
        inline static constexpr bool dev_only = false;
        inline static constexpr std::string_view name = "RandomUtils";

    private:
        static std::optional<std::string> process(std::string_view msg);

    protected:
        void do_on_event(mirai::Session& sess, const mirai::Event& event) override;
    };
}
