#pragma once

#include "../core/component.h"

namespace fc
{
    class AntiRecall final : public ComponentBase
    {
    public:
        inline static constexpr bool dev_only = false;
        inline static constexpr std::string_view name = "AntiRecall";

    protected:
        void do_on_event(mirai::Session& sess, const mirai::Event& event) override;
    };
}
