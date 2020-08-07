#pragma once

#include <string>
#include <array>

namespace fc::cit
{
    class Tiles final
    {
    private:
        class TileProxy final
        {
        private:
            size_t index_;
            Tiles& ref_;

        public:
            TileProxy(const size_t index, Tiles& ref): index_(index), ref_(ref) {}
            operator size_t() const { return ref_.get(index_); }
            TileProxy& operator=(const size_t value)
            {
                ref_.set(index_, value);
                return *this;
            }
        };

    private:
        // Uses 3 bits to save amount of each kind of tile, 27 bits in total
        uint32_t data_ = 0;

    public:
        Tiles() = default;
        explicit Tiles(const uint32_t bits): data_(bits) {}

        size_t get(size_t index) const;
        void set(size_t index, size_t value);

        size_t tile_count() const;

        size_t get_base5_index() const;
        static Tiles from_base5_index(size_t index);

        TileProxy operator[](const size_t index) { return { index, *this }; }
        size_t operator[](const size_t index) const { return get(index); }

        uint32_t data() const { return data_; }

        std::string format_as_numbers() const;
        std::string format_as_unicode() const;

        bool add_at(size_t index);
        bool add_toitsu(size_t index);
        bool add_shuntsu(size_t index);
        bool add_kotsu(size_t index);
    };

    class NoRiipaiTiles final
    {
    private:
        size_t size_ = 0;
        std::array<uint8_t, 14> data_{};

    public:
        NoRiipaiTiles() = default;
        explicit NoRiipaiTiles(const Tiles& tiles);

        void shuffle();

        std::string format_as_numbers() const;
        std::string format_as_unicode() const;
    };
}
