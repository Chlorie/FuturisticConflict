#include "dev_commands.h"

#include <fmt/core.h>

#include "../../core/app.h"
#include "../../utils/lock_algorithm.h"

namespace fc
{
    bool DevCommands::ping_pong(mirai::Session& sess, const CommandView& view) const
    {
        if (!view.single_arg("!ping")) return false;
        sess.send_message(dev_id_, "pong!");
        return true;
    }

    bool DevCommands::help(mirai::Session& sess, const CommandView& view) const
    {
        if (!view.single_arg("!help")) return false;
        static constexpr auto msg =
            u8R"(指令列表：
!help: 显示这个帮助列表
!save|!load: 保存/读取数据
!components: 列出所有组件
!groups: 列出已加入的群
!ping: 测试是否在线
!list (user|group) (add|remove) ($component|all) ($id|all): 操作某个组件的黑白名单
!list show (user|group|component) $id: 显示某个用户/群/组件的名单状况
)"sv;
        sess.send_message(dev_id_, msg);
        return true;
    }

    bool DevCommands::display_groups(mirai::Session& sess, const CommandView& view) const
    {
        if (!view.single_arg("!groups")) return false;
        const auto list = sess.group_list();
        std::string str = u8"查询完成！我已加入的所有群如下：";
        for (const auto& g : list)
            str += fmt::format("\n{} ({})", g.name, g.id.id);
        sess.send_message(dev_id_, str);
        return true;
    }

    bool DevCommands::save(mirai::Session& sess, const CommandView& view) const
    {
        if (!view.single_arg("!save")) return false;
        App::instance().save_data();
        sess.send_message(dev_id_, u8"数据保存成功！");
        return true;
    }

    bool DevCommands::load(mirai::Session& sess, const CommandView& view) const
    {
        if (!view.single_arg("!load")) return false;
        App::instance().load_data();
        sess.send_message(dev_id_, u8"数据读取成功！");
        return true;
    }

    bool DevCommands::components(mirai::Session& sess, const CommandView& view) const
    {
        if (!view.single_arg("!components")) return false;
        std::string msg = u8"任务列表：";
        App::instance().for_each_component([&msg](const Component& comp)
        {
            msg += fmt::format("\n{}: {}", comp.name(), comp->description());
        });
        sess.send_message(dev_id_, msg);
        return true;
    }

    void DevCommands::list_show_user(mirai::Session& sess, const CommandView& view) const
    {
        const auto id = mirai::utils::parse<std::optional<int64_t>>(view[3]);
        if (!id) return;
        std::string str;
        App::instance().for_each_component([&str, id = *id](const Component& comp)
        {
            const auto list = comp->user_blacklist().to_read();
            if (std::find(list->begin(), list->end(), id) != list->end())
            {
                if (!str.empty()) str += ' ';
                str += comp.name();
            }
        });
        sess.send_message(dev_id_, fmt::format(u8"黑名单中包含用户 {} 的组件如下: {}", *id, str));
    }

    void DevCommands::list_show_group(mirai::Session& sess, const CommandView& view) const
    {
        const auto id = mirai::utils::parse<std::optional<int64_t>>(view[3]);
        if (!id) return;
        std::string str;
        App::instance().for_each_component([&str, id = *id](const Component& comp)
        {
            const auto list = comp->group_whitelist().to_read();
            if (std::find(list->begin(), list->end(), id) != list->end())
            {
                if (!str.empty()) str += ' ';
                str += comp.name();
            }
        });
        sess.send_message(dev_id_, fmt::format(u8"白名单中包含群 {} 的组件如下: {}", *id, str));
    }

    namespace
    {
        void component_not_found(mirai::Session& sess,
            const mirai::uid_t dev_id, const std::string_view name)
        {
            sess.send_message(dev_id, fmt::format(u8"我找不到名为 {} 的组件……", name));
        }
    }

    void DevCommands::list_show_component(mirai::Session& sess, const CommandView& view) const
    {
        const auto name = view[3];
        const auto str = App::instance().for_component(name, [&](const Component& comp)
        {
            std::string res = fmt::format(u8"组件 {} 的用户黑名单包含 ", name);
            {
                const auto locked = comp->user_blacklist().to_read();
                const auto& list = *locked;
                for (const auto v : list)
                    res += fmt::format("{} ", v.id);
            }
            res += u8"群白名单包含 ";
            {
                const auto locked = comp->group_whitelist().to_read();
                const auto& list = *locked;
                for (const auto v : list)
                    res += fmt::format("{} ", v.id);
            }
            return res;
        });
        if (str)
            sess.send_message(dev_id_, *str);
        else
            component_not_found(sess, dev_id_, name);
    }

    void DevCommands::list_show(mirai::Session& sess, const CommandView& view) const
    {
        if (view.size() != 4) return;
        const auto opt = view[2];
        if (opt == "user")
            list_show_user(sess, view);
        else if (opt == "group")
            list_show_group(sess, view);
        else if (opt == "component")
            list_show_component(sess, view);
    }

    namespace
    {
        void insert_groups(Component& comp, const std::vector<mirai::Group>& groups)
        {
            const auto locked = comp->group_whitelist().to_write();
            auto& list = *locked;
            list.reserve(list.size() + groups.size());
            for (const auto& group : groups)
                list.emplace_back(group.id);
            std::sort(list.begin(), list.end());
            list.erase(std::unique(list.begin(), list.end()), list.end());
        }

        void clear_groups(Component& comp) { comp->group_whitelist()->clear(); }

        template <typename Id>
        void insert_element(Component& comp, const Id id)
        {
            if constexpr (std::is_same_v<Id, mirai::gid_t>)
                lock::insert(comp->group_whitelist(), id);
            else
                lock::insert(comp->user_blacklist(), id);
        }

        template <typename Id>
        void remove_element(Component& comp, const Id id)
        {
            if constexpr (std::is_same_v<Id, mirai::gid_t>)
                lock::remove(comp->group_whitelist(), id);
            else
                lock::remove(comp->user_blacklist(), id);
        }

        void list_operate_single_impl(Component& comp,
            const bool is_user, const bool is_add, const int64_t id)
        {
            if (is_user)
            {
                if (is_add)
                    insert_element(comp, mirai::uid_t(id));
                else
                    remove_element(comp, mirai::uid_t(id));
            }
            else
            {
                if (is_add)
                    insert_element(comp, mirai::gid_t(id));
                else
                    remove_element(comp, mirai::gid_t(id));
            }
        }
    }

    void DevCommands::list_operate_all(mirai::Session& sess, const CommandView& view) const
    {
        bool is_add = false;
        if (view[2] == "add") is_add = true;
        else if (view[2] != "remove") return;
        const auto groups = sess.group_list();
        const auto binded_insert = [&](Component& comp) { insert_groups(comp, groups); };
        const auto name = view[3];
        if (name == "all") // component = all
        {
            if (is_add)
            {
                App::instance().for_each_component(binded_insert);
                sess.send_message(dev_id_, u8"已为我加入的所有群启用所有组件！");
            }
            else
            {
                App::instance().for_each_component(clear_groups);
                sess.send_message(dev_id_, u8"已将所有组件在所有群禁用！");
            }
        }
        else
        {
            if (is_add
                    ? App::instance().for_component(name, binded_insert)
                    : App::instance().for_component(name, clear_groups))
                sess.send_message(dev_id_, fmt::format(u8"已为我加入的所有群{}组件 {}！",
                    is_add ? u8"启用" : u8"禁用", name));
            else
                component_not_found(sess, dev_id_, name);
        }
    }

    void DevCommands::list_operate_single(mirai::Session& sess,
        const bool is_user, const bool is_add, const int64_t id, const std::string_view name) const
    {
        const auto operate = [&](Component& comp)
        {
            list_operate_single_impl(comp, is_user, is_add, id);
        };
        if (name == "all") // Component = all
        {
            App::instance().for_each_component(operate);
            sess.send_message(dev_id_, fmt::format(u8"已将{} {} {}所有组件的名单！",
                is_user ? u8"用户" : u8"群", id, is_add ? u8"加入" : u8"移出"));
        }
        else
        {
            if (App::instance().for_component(name, operate))
                sess.send_message(dev_id_, fmt::format(u8"已将{} {} {}组件 {} 的名单！",
                    is_user ? u8"用户" : u8"群", id, is_add ? u8"加入" : u8"移出", name));
            else
                component_not_found(sess, dev_id_, name);
        }
    }

    void DevCommands::list_operate(mirai::Session& sess, const CommandView& view) const
    {
        if (view[4] == "all") // group = all
        {
            if (view[1] != "group") return;
            list_operate_all(sess, view);
        }
        else
        {
            const bool is_user = view[1] == "user";
            if (!is_user && view[1] != "group") return;
            const bool is_add = view[2] == "add";
            if (!is_add && view[2] != "remove") return;
            const auto id = mirai::utils::parse<std::optional<int64_t>>(view[4]);
            if (!id) return;
            list_operate_single(sess, is_user, is_add, *id, view[3]);
        }
    }

    bool DevCommands::list(mirai::Session& sess, const CommandView& view) const
    {
        if (view.size() < 4) return false;
        if (view[0] != "!list") return false;
        if (view[1] == "show")
            list_show(sess, view);
        else if (view.size() == 5)
            list_operate(sess, view);
        return true;
    }

    void DevCommands::do_on_event(mirai::Session& sess, const mirai::Event& event)
    {
        event.dispatch([&](const mirai::FriendMessage& e)
        {
            if (e.sender.id != dev_id_) return;
            if (!e.message.content.starts_with("!")) return;
            const std::string text = e.message.content.extract_text();
            const CommandView view(text);
            if (ping_pong(sess, view)) return;
            if (help(sess, view)) return;
            if (display_groups(sess, view)) return;
            if (save(sess, view)) return;
            if (load(sess, view)) return;
            if (components(sess, view)) return;
            if (list(sess, view)) return;
        });
    }
}
