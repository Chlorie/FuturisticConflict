#include "user_commands.h"

#include <fmt/core.h>

#include "../../core/app.h"
#include "../../utils/lock_algorithm.h"

namespace fc
{
    bool UserCommands::check_admin(const mirai::Member& member) const
    {
        return member.permission > mirai::Permission::member || member.id == dev_id_;
    }

    bool UserCommands::ping_pong(mirai::Session& sess,
        const mirai::GroupMessage& msg, const CommandView& view) const
    {
        if (!view.single_arg("%ping")) return false;
        sess.send_quote_message(msg, "pong!");
        return true;
    }

    namespace
    {
        inline constexpr auto not_found = u8"我找不到你说的这个组件啊……"sv;
    }

    bool UserCommands::help(mirai::Session& sess,
        const mirai::GroupMessage& msg, const CommandView& view) const
    {
        if (view[0] != "%help") return false;
        static constexpr auto help_msg =
            u8R"(这里是对立，或者叫Tairitsu，是音乐游戏Arcaea中的第一女主角（确信）（不服气的话可能会被拿伞打成骨折）。
为了更好地愉快（棒读）地玩耍，你可以通过以下的指令来跟我互动！
--全员可用--
%help: 显示这个帮助列表
%help $component: 显示对应组件的帮助
%components: 列出所有组件
%ping: 测试是否在线
%show_enabled: 列出所有在本群启用的组件
--管理可用--
(%enable|%disable) ($component|all): 在本群内启用/禁用某个/所有组件
)"sv;
        if (view.size() == 1)
            sess.send_quote_message(msg, help_msg);
        else if (view.size() == 2)
        {
            const auto comp_name = view[1];
            const bool found = App::instance().for_component(comp_name, [&](const Component& comp)
            {
                if (comp.dev_only())
                    sess.send_quote_message(msg, not_found);
                else
                    sess.send_quote_message(msg, fmt::format(u8"组件: {}\n简介: {}\n{}",
                        comp_name, comp->description(), comp->help_string()));
            });
            if (!found) sess.send_quote_message(msg, not_found);
        }
        return true;
    }

    bool UserCommands::components(mirai::Session& sess,
        const mirai::GroupMessage& msg, const CommandView& view) const
    {
        if (!view.single_arg("%components")) return false;
        std::string res = u8"诶？你问我都会什么？\n";
        App::instance().for_each_component([&](const Component& comp)
        {
            if (comp.dev_only()) return;
            res += fmt::format(u8"{}：{}\n", comp.name(), comp->description());
        });
        res += u8"大概就这么多了~";
        sess.send_quote_message(msg, res);
        return true;
    }

    bool UserCommands::show_enabled(mirai::Session& sess,
        const mirai::GroupMessage& msg, const CommandView& view) const
    {
        if (!view.single_arg("%show_enabled")) return false;
        std::string res = u8"本群中已经开启的任务如下：\n";
        App::instance().for_each_component([&](const Component& comp)
        {
            if (comp.dev_only()) return;
            const auto locked = comp->group_whitelist();
            if (std::find(locked->begin(), locked->end(), msg.sender.group.id) != locked->end())
                res += fmt::format("{} ", comp.name());
        });
        sess.send_quote_message(msg, res);
        return true;
    }

    bool UserCommands::enable_disable(mirai::Session& sess,
        const mirai::GroupMessage& msg, const CommandView& view) const
    {
        if (view.size() != 2) return false;
        const bool is_enable = view[0] == "%enable";
        if (!is_enable && view[0] != "%disable") return false;
        const auto comp_name = view[1];
        const bool is_all = comp_name == "all";
        const auto insert = [group = msg.sender.group.id](Component& comp)
        {
            if (!comp.dev_only())
                lock::insert(comp->group_whitelist(), group);
        };
        const auto remove = [group = msg.sender.group.id](Component& comp)
        {
            if (!comp.dev_only())
                lock::remove(comp->group_whitelist(), group);
        };
        if (is_all)
        {
            if (is_enable)
            {
                App::instance().for_each_component(insert);
                sess.send_quote_message(msg, u8"已在本群内启用所有组件！");
            }
            else
            {
                App::instance().for_each_component(remove);
                sess.send_quote_message(msg, u8"已在本群内禁用所有组件！");
            }
        }
        else
        {
            if (is_enable
                    ? App::instance().for_component(comp_name, insert)
                    : App::instance().for_component(comp_name, remove))
                sess.send_quote_message(msg, fmt::format(u8"已在本群内{}组件 {}！",
                    is_enable ? u8"启用" : u8"禁用", comp_name));
            else
                sess.send_quote_message(msg, not_found);
        }
        return true;
    }

    void UserCommands::do_on_event(mirai::Session& sess, const mirai::Event& event)
    {
        event.dispatch([&](const mirai::GroupMessage& e)
        {
            if (!passes(e.sender.group.id)) return;
            if (e.message.quote) return;
            const auto opt = e.message.content.match_types<mirai::msg::Plain>();
            if (!opt) return;
            const auto& [plain] = *opt;
            if (!mirai::utils::starts_with(plain.view(), "%")) return;
            const CommandView view(plain.view());
            if (view.size() == 0) return;
            if (ping_pong(sess, e, view)) return;
            if (help(sess, e, view)) return;
            if (components(sess, e, view)) return;
            if (show_enabled(sess, e, view)) return;
            if (!check_admin(e.sender)) return; // Admin commands start here
            if (enable_disable(sess, e, view)) return;
        });
    }
}
