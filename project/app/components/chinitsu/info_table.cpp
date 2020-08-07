#include "info_table.h"

#include <array>
#include <algorithm>

namespace fc::cit
{
    namespace
    {
        constexpr std::array<std::string_view, 9> unicode_tiles
        {
            u8"🀇", u8"🀈", u8"🀉", u8"🀊", u8"🀋", u8"🀌", u8"🀍", u8"🀎", u8"🀏"
        };
        constexpr size_t unicode_tile_size = unicode_tiles[0].size();
        constexpr size_t table_size = 1953125; // 5 ^ 9
    }

    void generate_agari_info(std::vector<HandInfo>& table)
    {
        static const auto add_mentsu = [](Tiles& tiles, const size_t i)
        {
            return i < 9
                       ? tiles.add_kotsu(i)
                       : tiles.add_shuntsu(i - 9);
        };

        for (size_t m1 = 0; m1 < 16; m1++) // 9 kinds of kotsu + 7 kinds of shuntsu
        {
            Tiles tiles1;
            add_mentsu(tiles1, m1); // First mentsu cannot overflow
            for (size_t m2 = m1; m2 < 16; m2++)
            {
                Tiles tiles2 = tiles1;
                if (!add_mentsu(tiles2, m2)) continue; // Overflows
                for (size_t m3 = m2; m3 < 16; m3++)
                {
                    Tiles tiles3 = tiles2;
                    if (!add_mentsu(tiles3, m3)) continue; // Overflows
                    for (size_t m4 = m3; m4 < 16; m4++)
                    {
                        Tiles tiles4 = tiles3;
                        if (!add_mentsu(tiles4, m4)) continue; // Overflows
                        for (size_t t = 0; t < 9; t++)
                        {
                            Tiles tiles_final = tiles4;
                            if (!tiles_final.add_toitsu(t)) continue; // Overflows
                            table[tiles_final.get_base5_index()] = HandInfo(HandInfo::agari_bit);
                        }
                    }
                }
            }
        }
    }

    void generate_tenpai_info(std::vector<HandInfo>& table)
    {
        for (size_t i = 0; i < table_size; i++)
        {
            const Tiles tiles = Tiles::from_base5_index(i);
            if (tiles.tile_count() != 13) continue;
            uint16_t& data = table[i].data();
            for (size_t j = 0; j < 9; j++)
            {
                Tiles added = tiles;
                if (!added.add_at(j)) continue;
                if (table[added.get_base5_index()].agari())
                {
                    data |= static_cast<uint16_t>(1u << j);
                    data += 1u << 9;
                }
            }
        }
    }

    std::string HandInfo::format_as_numbers() const
    {
        uint16_t bits = data_;
        std::string result;
        result.reserve(10);
        for (size_t i = 0; i < 9; i++)
        {
            if (bits & 1u) result += static_cast<char>('1' + i);
            bits >>= 1;
        }
        result += 'm';
        return result;
    }

    std::string HandInfo::format_as_unicode() const
    {
        uint16_t bits = data_;
        std::string result;
        result.reserve(9 * unicode_tile_size);
        for (size_t i = 0; i < 9; i++)
        {
            if (bits & 1u) result += unicode_tiles[i];
            bits >>= 1;
        }
        return result;
    }

    InfoTables::InfoTables()
    {
        info_table.resize(table_size);
        generate_agari_info(info_table);
        generate_tenpai_info(info_table);

        for (size_t i = 0; i < table_size; i++)
        {
            if (info_table[i].agari()) agari_indices.push_back(i);
            else if (info_table[i].tenpai()) tenpai_indices.push_back(i);
        }

        std::sort(tenpai_indices.begin(), tenpai_indices.end(),
            [&](const size_t lhs, const size_t rhs)
            {
                return info_table[lhs].tenpai_kind_count() > info_table[rhs].tenpai_kind_count();
            });

        for (size_t i = 0; i < tenpai_indices.size(); i++)
            if (info_table[tenpai_indices[i]].tenpai_kind_count() == 2)
            {
                tamenchan_count = i;
                break;
            }
    }
}
