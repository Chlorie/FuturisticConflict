#include "repeat.h"

namespace fc
{
    RepeatChain::RepeatInfo RepeatChain::add_msg(const mirai::uid_t user, const std::string& msg)
    {
        if (msg != msg_)
        {
            msg_ = msg;
            length_ = 1;
            count_.clear();
            count_[user] = 1;
            return { 1, 1 };
        }
        const size_t count = ++count_[user];
        if (count == 1) ++length_;
        return { length_, count };
    }

    void Repeat::do_on_event(mirai::Session& sess, const mirai::Event& event)
    {
        event.dispatch([&](const mirai::GroupMessage& msg)
        {
            if (!passes(msg.sender.group.id)) return;
            const auto [users, repeat_count] = [&]()
            {
                const auto chains = chains_.to_write();
                return chains[msg.sender.group.id]
                   .add_msg(msg.sender.id, msg.message.content.stringify());
            }();
            if (users == 5)
                sess.send_message(msg.sender.group.id, msg.message.content);
            switch (repeat_count)
            {
                case 2: case 3:
                    if (msg.sender.bot_has_higher_permission())
                        sess.recall(msg.message.source.id);
                    return;
                case 4:
                {
                    const bool mute = msg.sender.id == dev_id_ ? false :
                                          msg.sender.bot_has_higher_permission();
                    if (mute) sess.mute(msg.sender.group.id, msg.sender.id, 30min);
                    sess.send_quote_message(msg, mute ? u8"那你也一定不是什么好东西"
                                                     : u8"听我一句劝吧……别再刷屏了……");
                    return;
                }
                default:
                    return;
            }
        });
    }
}
