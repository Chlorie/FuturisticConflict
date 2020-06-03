#include "component.h"

#include <fmt/core.h>

#include "app.h"
#include "../utils/lock_algorithm.h"

namespace fc
{
    bool ComponentBase::passes(const mirai::gid_t group) const
    {
        return lock::contains(group_whitelist_, group);
    }

    bool ComponentBase::passes(const mirai::uid_t user) const
    {
        return !lock::contains(user_blacklist_, user);
    }

    std::optional<ComponentBase::TextMessage> ComponentBase::check_lists_and_trigger(
        const mirai::uid_t bot_id, const mirai::Event& event) const
    {
        std::optional<TextMessage> opt;
        event.dispatch([&](const mirai::FriendMessage& e)
        {
            if (!passes(e.sender.id)) return;
            if (e.message.quote) return;
            const auto segments = e.message.content.match_types<mirai::msg::Plain>();
            if (!segments) return;
            const auto& [plain] = *segments;
            opt = {
                e.sender.id, {}, e.message.source.id,
                mirai::utils::trim_whitespace(plain.view())
            };
        });
        event.dispatch([&, bot_id](const mirai::GroupMessage& e)
        {
            if (!passes(e.sender.group.id)) return;
            if (e.message.quote) return;
            const auto segments = e.message.content.match_types<mirai::msg::At, mirai::msg::Plain>();
            if (!segments) return; // Not "At + Plain"
            const auto& [at, plain] = *segments;
            if (at.target != bot_id) return; // Not at bot
            opt = {
                e.sender.id, e.sender.group.id, e.message.source.id,
                mirai::utils::trim_whitespace(plain.view())
            };
        });
        return opt;
    }

    void Component::save_data(Config& config) const
    {
        auto& data = config.components[std::string(name_)];
        {
            const auto list = ptr_->group_whitelist_.to_read();
            data.group_whitelist = *list;
        }
        {
            const auto list = ptr_->user_blacklist_.to_read();
            data.user_blacklist = *list;
        }
        ptr_->do_save_data(config);
    }

    void Component::load_data(const Config& config)
    {
        ptr_->do_load_data(config);
        const auto iter = config.components.find(std::string(name_));
        if (iter == config.components.end()) return;
        ptr_->help_string_ = iter->second.help_string;
        ptr_->description_ = iter->second.description;
        {
            const auto list = ptr_->group_whitelist_.to_write();
            *list = iter->second.group_whitelist;
        }
        {
            const auto list = ptr_->user_blacklist_.to_write();
            *list = iter->second.user_blacklist;
        }
    }

    void Component::on_event(mirai::Session& session, const mirai::Event& event)
    {
        try { ptr_->do_on_event(session, event); }
        catch (const std::exception& exc)
        {
            std::string str;
            switch (event.type())
            {
                case mirai::EventType::friend_message:
                {
                    const auto& e = event.get<mirai::FriendMessage>();
                    str = fmt::format(u8"FriendMessage\n好友: {} ({}, {})\n内容: \n{}",
                        e.sender.remark, e.sender.nickname, e.sender.id.id,
                        e.message.content.stringify());
                    break;
                }
                case mirai::EventType::group_message:
                {
                    const auto& e = event.get<mirai::GroupMessage>();
                    str = fmt::format(u8"GroupMessage\n群: {} ({})\n成员: {} ({})\n内容:\n{}",
                        e.sender.group.name, e.sender.group.id.id,
                        e.sender.member_name, e.sender.id.id,
                        e.message.content.stringify());
                    break;
                }
                case mirai::EventType::temp_message:
                {
                    const auto& e = event.get<mirai::TempMessage>();
                    str = fmt::format(u8"TempMessage\n群: {} ({})\n成员: {} ({})\n内容:\n{}",
                        e.sender.group.name, e.sender.group.id.id,
                        e.sender.member_name, e.sender.id.id,
                        e.message.content.stringify());
                    break;
                }
                default:
                {
                    str = mirai::event_type_names[static_cast<size_t>(event.type())];
                    break;
                }
            }
            const std::string full_msg = fmt::format(u8"在运行组件 {} 时抛出了异常: {}\n"
                u8"事件类型: {}\n能不能抽时间修复一下呢？", name_, exc.what(), str);
            try
            {
                const auto config = App::instance().config();
                session.send_message(config->dev_id, full_msg);
            }
            catch (...) {} // Ignore all exceptions if log failed
        }
    }
}
