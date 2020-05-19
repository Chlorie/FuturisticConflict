#pragma once

#include "command_view.h"
#include "../../core/component.h"

namespace fc
{
    class DevCommands final : public ComponentBase
    {
    public:
        inline static constexpr bool dev_only = true;
        inline static constexpr std::string_view name = "DevCommands";

    private:
        mirai::uid_t dev_id_;

        bool ping_pong(mirai::Session& sess, const CommandView& view) const;
        bool display_groups(mirai::Session& sess, const CommandView& view) const;

    protected:
        void do_on_event(mirai::Session& sess, const mirai::Event& event) override;
        void do_load_data(const Config& config) override { dev_id_ = config.dev_id; }
    };
}
