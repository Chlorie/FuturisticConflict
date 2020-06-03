#pragma once

#include "../../core/component.h"

namespace fc
{
    class RepeatChain final
    {
    public:
        struct RepeatInfo final
        {
            size_t user_count = 0;
            size_t user_repeat_count = 0;
        };

    private:
        size_t length_ = 0;
        std::string msg_;
        std::map<mirai::uid_t, size_t> count_;

    public:
        RepeatInfo add_msg(mirai::uid_t user, const std::string& msg);
    };

    class Repeat final : public ComponentBase
    {
    public:
        inline static constexpr bool dev_only = false;
        inline static constexpr std::string_view name = "Repeat";

    private:
        lock::map<mirai::gid_t, RepeatChain> chains_;
        mirai::uid_t dev_id_;

    protected:
        void do_on_event(mirai::Session& sess, const mirai::Event& event) override;
        void do_load_data(const Config& config) override { dev_id_ = config.dev_id; }
    };
}
