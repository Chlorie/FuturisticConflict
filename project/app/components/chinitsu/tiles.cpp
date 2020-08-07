#include "tiles.h"

#include "../othello/othello_game.h"

namespace fc::cit
{
    namespace
    {
        constexpr std::array<std::string_view, 9> unicode_tiles
        {
            u8"🀇", u8"🀈", u8"🀉", u8"🀊", u8"🀋", u8"🀌", u8"🀍", u8"🀎", u8"🀏"
        };
        constexpr size_t unicode_tile_size = unicode_tiles[0].size();
    }

    size_t Tiles::get(const size_t index) const { return static_cast<size_t>((data_ >> (index * 3)) & 0b111u); }

    void Tiles::set(const size_t index, const size_t value)
    {
        data_ &= ~(0b111u << (index * 3));
        data_ |= static_cast<uint32_t>(value) << (index * 3);
    }

    size_t Tiles::tile_count() const
    {
        uint32_t bits = data_;
        size_t result = 0;
        for (size_t i = 0; i < 9; i++)
        {
            result += bits & 0b111u;
            bits >>= 3;
        }
        return result;
    }

    size_t Tiles::get_base5_index() const
    {
        uint32_t bits = data_;
        size_t result = 0;
        for (size_t i = 0; i < 9; i++)
        {
            result = result * 5 + (bits & 0b111u);
            bits >>= 3;
        }
        return result;
    }

    Tiles Tiles::from_base5_index(size_t index)
    {
        uint32_t result = 0;
        for (size_t i = 0; i < 9; i++)
        {
            result = (result << 3) | static_cast<uint32_t>(index % 5);
            index /= 5;
        }
        return Tiles(result);
    }

    std::string Tiles::format_as_numbers() const
    {
        std::string result;
        result.reserve(15);
        for (size_t i = 0; i < 9; i++)
            result.append(get(i), static_cast<char>('1' + i));
        result += 'm';
        return result;
    }

    std::string Tiles::format_as_unicode() const
    {
        std::string result;
        result.reserve(14 * unicode_tile_size);
        for (size_t i = 0; i < 9; i++)
            for (size_t j = 0; j < get(i); j++)
                result += unicode_tiles[i];
        return result;
    }

    bool Tiles::add_at(const size_t index)
    {
        if (data_ & (4u << (index * 3))) return false;
        data_ += 1u << (index * 3);
        return true;
    }

    bool Tiles::add_toitsu(const size_t index)
    {
        if (get(index) > 2) return false;
        data_ += 2u << (index * 3);
        return true;
    }

    bool Tiles::add_shuntsu(const size_t index)
    {
        if (data_ & (0b100'100'100u) << (index * 3)) return false;
        data_ += 0b001'001'001u << (index * 3);
        return true;
    }

    bool Tiles::add_kotsu(const size_t index)
    {
        if (data_ & (0b110u << (index * 3))) return false;
        data_ += 3u << (index * 3);
        return true;
    }

    NoRiipaiTiles::NoRiipaiTiles(const Tiles& tiles)
    {
        for (size_t i = 0; i < 9; i++)
            for (size_t j = 0; j < tiles[i]; j++)
                data_[size_++] = i;
    }

    void NoRiipaiTiles::shuffle()
    {
        std::shuffle(data_.begin(), data_.begin() + size_, random::generator());
    }

    std::string NoRiipaiTiles::format_as_numbers() const
    {
        std::string result;
        result.reserve(15);
        for (size_t i = 0; i < size_; i++)
            result += static_cast<char>('1' + data_[i]);
        result += 'm';
        return result;
    }

    std::string NoRiipaiTiles::format_as_unicode() const
    {
        std::string result;
        result.reserve(14 * unicode_tile_size);
        for (size_t i = 0; i < size_; i++)
            result += unicode_tiles[data_[i]];
        return result;
    }
}
