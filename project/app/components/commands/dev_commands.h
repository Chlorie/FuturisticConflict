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
        bool help(mirai::Session& sess, const CommandView& view) const;
        bool display_groups(mirai::Session& sess, const CommandView& view) const;
        bool save(mirai::Session& sess, const CommandView& view) const;
        bool load(mirai::Session& sess, const CommandView& view) const;
        bool components(mirai::Session& sess, const CommandView& view) const;

        void list_show_user(mirai::Session& sess, const CommandView& view) const;
        void list_show_group(mirai::Session& sess, const CommandView& view) const;
        void list_show_component(mirai::Session& sess, const CommandView& view) const;
        void list_show(mirai::Session& sess, const CommandView& view) const;
        void list_operate_all(mirai::Session& sess, const CommandView& view) const;
        void list_operate_single(mirai::Session& sess,
            bool is_user, bool is_add, int64_t id, std::string_view comp_name) const;
        void list_operate(mirai::Session& sess, const CommandView& view) const;
        bool list(mirai::Session& sess, const CommandView& view) const;

    protected:
        void do_on_event(mirai::Session& sess, const mirai::Event& event) override;
        void do_load_data(const Config& config) override { dev_id_ = config.dev_id; }
    };
}
