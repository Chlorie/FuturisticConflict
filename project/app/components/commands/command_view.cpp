#include "command_view.h"

namespace fc
{
    CommandView::CommandView(const std::string_view cmd) : cmd_(cmd)
    {
        static constexpr const char* whitespace = " \t\r\n\v\f";
        size_t begin = 0;
        while (true)
        {
            begin = cmd.find_first_not_of(whitespace, begin);
            if (begin == std::string::npos) return;
            size_t end = cmd.find_first_of(whitespace, begin);
            if (end == std::string::npos) end = cmd.size();
            views_.emplace_back(&cmd[begin], end - begin);
            begin = end;
        }
    }
}
