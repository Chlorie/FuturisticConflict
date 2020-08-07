#pragma once

#include "../../core/component.h"
#include "../../utils/scheduler.h"

namespace fc
{
    class Chinitsu final : public ComponentBase
    {
    public:
        inline static constexpr bool dev_only = false;
        inline static constexpr std::string_view name = "Chinitsu";

    private:
        Scheduler<mirai::msgid_t> scheduler_;

    protected:
        void do_on_event(mirai::Session& sess, const mirai::Event& event) override;
    };
}
