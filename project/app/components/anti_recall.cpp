#include "anti_recall.h"

namespace fc
{
    void AntiRecall::do_on_event(mirai::Session& sess, const mirai::Event& event)
    {
        event.dispatch([&](const mirai::GroupRecallEvent& e)
        {
            if (!passes(e.group.id)) return;
            if (e.operator_ && e.author_id == e.operator_->id)
                sess.send_message(e.group.id, u8"不许撤回", e.message_id);
        });
    }
}
