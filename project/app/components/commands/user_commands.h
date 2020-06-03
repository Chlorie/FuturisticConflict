#pragma once

#include "command_view.h"
#include "../../core/component.h"

namespace fc
{
    class UserCommands final : public ComponentBase
    {
    public:
        inline static constexpr bool dev_only = true;
        inline static constexpr std::string_view name = "UserCommands";

    private:
        mirai::uid_t dev_id_;

        bool check_admin(const mirai::Member& member) const;

        bool ping_pong(mirai::Session& sess, const mirai::GroupMessage& msg, const CommandView& view) const;
        bool help(mirai::Session& sess, const mirai::GroupMessage& msg, const CommandView& view) const;
        bool components(mirai::Session& sess, const mirai::GroupMessage& msg, const CommandView& view) const;
        bool show_enabled(mirai::Session& sess, const mirai::GroupMessage& msg, const CommandView& view) const;
        bool enable_disable(mirai::Session& sess, const mirai::GroupMessage& msg, const CommandView& view) const;

    protected:
        void do_on_event(mirai::Session& sess, const mirai::Event& event) override;
        void do_load_data(const Config& config) override { dev_id_ = config.dev_id; }
    };
}
