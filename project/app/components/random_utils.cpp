#include "random_utils.h"

#include <fmt/core.h>

#include "commands/command_view.h"
#include "../utils/random.h"

namespace fc
{
    namespace
    {
        std::string select(const CommandView& view)
        {
            if (view.size() == 1) return u8"所以是要选择什么？";
            if (view.size() == 2) return u8"这有什么可选择的空间吗？";
            return fmt::format(u8"那我帮你选择{}",
                view[random::uniform_int(1, static_cast<int32_t>(view.size() - 1))]);
        }

        std::string permute(const CommandView& view)
        {
            if (view.size() == 1) return u8"所以是要排列什么？";
            if (view.size() == 2) return u8"一个元素也需要我帮忙排列吗";
            const auto perm = random::permute(view.size() - 1);
            std::string res = u8"随机排列结果:";
            for (const size_t idx : perm)
                res += fmt::format(" {}", view[idx + 1]);
            return res;
        }
    }

    std::optional<std::string> RandomUtils::process(const std::string_view msg)
    {
        const CommandView view(msg);
        if (view.size() == 0) return {};
        if (view[0] == "select" || view[0] == u8"选择") return select(view);
        else if (view[0] == "permute" || view[0] == u8"排列") return permute(view);
        return {};
    }

    void RandomUtils::do_on_event(mirai::Session& sess, const mirai::Event& event)
    {
        check_and_reply(sess, event, &RandomUtils::process);
    }
}
