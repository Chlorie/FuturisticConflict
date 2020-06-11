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
        int channel;
        data_ = static_cast<uint8_t*>(stbi_load(
            file_str.c_str(), &width_, &height_, &channel, 4));
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
        data_ = static_cast<uint8_t*>(STBI_MALLOC(size()));
        std::memcpy(data_, other.data_, other.size());
    }

    Image::Image(Image&& other) noexcept:
        width_(std::exchange(other.width_, 0)),
        height_(std::exchange(other.height_, 0)),
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
        for (int i = min_y; i <= max_y; i++)
        {
            uint8_t* const row = data_ + 4 * i * width_;
            for (int j = min_x; j <= max_x; j++)
            {
                if (sqr(j + 0.5f - x) + sqr(i + 0.5f - y) > r2) continue;
                uint8_t* const pixel = row + 4 * j;
                pixel[0] = red;
                pixel[1] = green;
                pixel[2] = blue;
            }
        }
    }

    void Image::draw_rectangle(const int x, const int y,
        const int w, const int h, const uint32_t color)
    {
        const int r = static_cast<int>(static_cast<uint8_t>(color >> 24));
        const int g = static_cast<int>(static_cast<uint8_t>(color >> 16));
        const int b = static_cast<int>(static_cast<uint8_t>(color >> 8));
        const int a = static_cast<int>(static_cast<uint8_t>(color));
        const auto blend = [=](uint8_t* dst)
        {
            const int out_alpha = (255 * a + dst[3] * (255 - a)) / 255;
            if (out_alpha == 0)
            {
                dst[0] = dst[1] = dst[2] = dst[3] = static_cast<uint8_t>(0);
                return;
            }
            dst[0] = static_cast<uint8_t>(
                (255 * a * r + dst[0] * (255 - a) * dst[3]) / (255 * out_alpha));
            dst[1] = static_cast<uint8_t>(
                (255 * a * g + dst[1] * (255 - a) * dst[3]) / (255 * out_alpha));
            dst[2] = static_cast<uint8_t>(
                (255 * a * b + dst[2] * (255 - a) * dst[3]) / (255 * out_alpha));
            dst[3] = out_alpha;
        };
        const int min_x = std::max(x, 0);
        const int max_x = std::min(x + w, width_);
        const int min_y = std::max(y, 0);
        const int max_y = std::min(y + h, height_);
        for (int i = min_y; i < max_y; i++)
        {
            uint8_t* const row = data_ + 4 * i * width_;
            for (int j = min_x; j < max_x; j++)
                blend(row + 4 * j);
        }
    }

    void Image::draw_image(const Image& image, const int x, const int y)
    {
        if (x >= width_) return;
        if (y >= height_) return;
        const int x_end = std::min(image.width_ + x, width_);
        const int y_end = std::min(image.height_ + y, height_);
        const size_t copy_size = 4 * static_cast<size_t>(x_end - x);
        const size_t stride = 4 * width_;
        const size_t other_stride = 4 * image.width_;
        {
            int y_current = y, y_other = 0;
            uint8_t* ptr = data_ + stride * y_current + 4 * x;
            uint8_t* other_ptr = image.data_ + other_stride * y_other;
            for (; y_current < y_end;)
            {
                std::memcpy(ptr, other_ptr, copy_size);
                y_current++;
                y_other++;
                ptr += stride;
                other_ptr += other_stride;
            }
        }
    }

    void Image::save(const fs::path& file) const
    {
        const std::string file_str = file.string();
        const fs::path ext = file.extension();
        if (ext == ".bmp")
            stbi_write_bmp(file_str.c_str(), width_, height_, 4, data_);
        else if (ext == ".jpg")
            stbi_write_jpg(file_str.c_str(), width_, height_, 4, data_, 50);
        else if (ext == ".png")
            stbi_write_png(file_str.c_str(),
                width_, height_, 4, data_, 4 * width_);
        else if (ext == ".tga")
            stbi_write_tga(file_str.c_str(), width_, height_, 4, data_);
        else
            throw std::runtime_error("Image format not supported");
    }
}
