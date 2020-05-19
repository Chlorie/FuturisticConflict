#include "dev_commands.h"

#include <fmt/core.h>

namespace fc
{
    bool DevCommands::ping_pong(mirai::Session& sess, const CommandView& view) const
    {
        if (!view.single_arg("!ping")) return false;
        sess.send_message(dev_id_, "pong!");
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

    void DevCommands::do_on_event(mirai::Session& sess, const mirai::Event& event)
    {
        event.dispatch([&](const mirai::FriendMessage& e)
        {
            if (e.sender.id != dev_id_) return;
            const std::string text = e.message.content.extract_text();
            const CommandView view(text);
            if (ping_pong(sess, view)) return;
            if (display_groups(sess, view)) return;
        });
    }
}
