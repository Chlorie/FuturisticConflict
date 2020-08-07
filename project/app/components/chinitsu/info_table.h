#pragma once

#include <vector>

#include "tiles.h"

namespace fc::cit
{
    // Info of a 13- or 14-tile hand
    // For 14-tile hand, if in agari state, the highest bit is set
    // For 13-tile hand, the lowest 9 bits corresponds to whether this tile leads to agari
    // 10~13 bits corresponds to tenpai kinds
    class HandInfo final
    {
    public:
        static constexpr uint16_t agari_bit = static_cast<uint16_t>(1 << 15);

    private:
        uint16_t data_ = 0;

    public:
        HandInfo() = default;
        explicit HandInfo(const uint16_t bits): data_(bits) {}

        bool agari() const { return (data_ & agari_bit) != 0; }
        bool tenpai() const { return (data_ & ~agari_bit) != 0; }
        size_t tenpai_kind_count() const { return (data_ >> 9) & 0b1111u; }

        uint16_t& data() { return data_; }
        uint16_t data() const { return data_; }

        std::string format_as_numbers() const;
        std::string format_as_unicode() const;
    };

    struct InfoTables final
    {
        std::vector<HandInfo> info_table;
        std::vector<size_t> agari_indices;
        std::vector<size_t> tenpai_indices;
        size_t tamenchan_count = 0;

        InfoTables();
    };

    inline static const InfoTables info_table{};
}
