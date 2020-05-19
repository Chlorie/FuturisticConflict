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
                case 2:
                    sess.send_message(msg.sender.group.id, u8"刷屏可不是好习惯！", msg.message.source.id);
                    return;
                case 3:
                    sess.send_message(msg.sender.group.id, u8"刷屏会给其他群友带来困扰的！", msg.message.source.id);
                    return;
                case 4:
                {
                    bool result = false;
                    if (msg.sender.id != dev_id_)
                    {
                        try
                        {
                            sess.mute(msg.sender.group.id, msg.sender.id, 600s);
                            result = true;
                        }
                        catch (...)
                        {
                            result = false;
                        }
                    }
                    sess.send_message(msg.sender.group.id,
                        result ? u8"希望你能利用这个机会好好反省一下自己都做了些什么"
                            : u8"听我一句劝吧……别再刷屏了……", msg.message.source.id);
                    return;
                }
                default:
                    return;
            }
        });
    }
}
