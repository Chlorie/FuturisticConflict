#include "random_select.h"

#include <fmt/core.h>

#include "commands/command_view.h"
#include "../utils/random.h"

namespace fc
{
    std::optional<std::string> RandomSelect::process(const std::string_view msg)
    {
        const CommandView view(msg);
        if (view.size() == 0) return {};
        if (view[0] != "select" && view[0] != u8"选择") return {};
        if (view.size() == 1) return u8"所以是要选择什么？";
        if (view.size() == 2) return u8"这有什么可选择的空间吗？";
        return fmt::format(u8"那我帮你选择{}",
            view[random::uniform_int(1, static_cast<int32_t>(view.size() - 1))]);
    }

    void RandomSelect::do_on_event(mirai::Session& sess, const mirai::Event& event)
    {
        check_and_reply(sess, event, &RandomSelect::process);
    }
}
