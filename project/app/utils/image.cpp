#include "image.h"

#include <cstring>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace fc::gfx
{
    Image::Image(const fs::path& file)
    {
        const std::string file_str = file.string();
        data_ = static_cast<uint8_t*>(stbi_load(
            file_str.c_str(), &width_, &height_, &channel_, 0));
        if (!data_) throw std::runtime_error("Failed to load image");
    }

    Image::~Image() noexcept
    {
        STBI_FREE(data_);
    }

    Image::Image(const Image& other)
    {
        if (other.empty()) return;
        width_ = other.width_;
        height_ = other.height_;
        channel_ = other.channel_;
        data_ = static_cast<uint8_t*>(STBI_MALLOC(size()));
        std::memcpy(data_, other.data_, other.size());
    }

    Image::Image(Image&& other) noexcept:
        width_(std::exchange(other.width_, 0)),
        height_(std::exchange(other.height_, 0)),
        channel_(std::exchange(other.channel_, 0)),
        data_(std::exchange(other.data_, nullptr)) {}

    Image& Image::operator=(const Image& other)
    {
        Image copy = other;
        swap(copy);
        return *this;
    }

    Image& Image::operator=(Image&& other) noexcept
    {
        Image copy = std::move(other);
        swap(copy);
        return *this;
    }

    void Image::swap(Image& other) noexcept
    {
        std::swap(width_, other.width_);
        std::swap(height_, other.height_);
        std::swap(channel_, other.channel_);
        std::swap(data_, other.data_);
    }

    // Alpha channel of color is not used currently
    // No anti-aliasing performed
    void Image::draw_circle(const float x, const float y, const float r, const uint32_t color)
    {
        static constexpr auto sqr = [](const float f) { return f * f; };
        const int min_x = std::max(static_cast<int>(x - r), 0);
        const int max_x = std::min(static_cast<int>(x + r), width_ - 1);
        const int min_y = std::max(static_cast<int>(y - r), 0);
        const int max_y = std::min(static_cast<int>(y + r), height_ - 1);
        const uint8_t red = static_cast<uint8_t>(color >> 24);
        const uint8_t green = static_cast<uint8_t>(color >> 16);
        const uint8_t blue = static_cast<uint8_t>(color >> 8);
        const float r2 = r * r;
        for (int i = min_x; i <= max_x; i++)
        {
            uint8_t* const row = data_ + i * width_ * channel_;
            for (int j = min_y; j <= max_y; j++)
            {
                if (sqr(i + 0.5f - x) + sqr(j + 0.5f - y) > r2) continue;
                uint8_t* const pixel = row + j * channel_;
                pixel[0] = red;
                pixel[1] = green;
                pixel[2] = blue;
            }
        }
    }

    void Image::save(const fs::path& file) const
    {
        const std::string file_str = file.string();
        const fs::path ext = file.extension();
        if (ext == ".bmp")
            stbi_write_bmp(file_str.c_str(), width_, height_, channel_, data_);
        else if (ext == ".jpg")
            stbi_write_jpg(file_str.c_str(), width_, height_, channel_, data_, 50);
        else if (ext == ".png")
            stbi_write_png(file_str.c_str(),
                width_, height_, channel_, data_, width_ * channel_);
        else if (ext == ".tga")
            stbi_write_tga(file_str.c_str(), width_, height_, channel_, data_);
        else
            throw std::runtime_error("Image format not supported");
    }
}
