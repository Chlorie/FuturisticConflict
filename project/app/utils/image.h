#pragma once

#include <filesystem>

namespace fc::gfx
{
    namespace fs = std::filesystem;

    class Image final
    {
    private:
        int width_ = 0, height_ = 0, channel_ = 0;
        uint8_t* data_ = nullptr;

    public:
        Image() = default;
        explicit Image(const fs::path& file);
        ~Image() noexcept;
        Image(const Image& other);
        Image(Image&& other) noexcept;
        Image& operator=(const Image& other);
        Image& operator=(Image&& other) noexcept;
        void swap(Image& other) noexcept;
        friend void swap(Image& lhs, Image& rhs) noexcept { lhs.swap(rhs); }

        size_t size() const { return static_cast<size_t>(width_ * height_ * channel_); }
        bool empty() const { return !data_; }

        void draw_circle(float x, float y, float r, uint32_t color);

        void save(const fs::path& file) const;
    };
}
