// Copyright (c) 2010-present Bifrost Entertainment AS and Tommy Nguyen
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)

#include "Graphics/Label.h"

#include <cstring>

#include "Common/UTF8.h"
#include "Math/Transform.h"

using rainbow::Color;
using rainbow::FontAtlas;
using rainbow::Label;
using rainbow::SharedPtr;
using rainbow::TextAlignment;
using rainbow::Vec2f;
using rainbow::czstring;

namespace
{
    constexpr float kAlignmentFactor[]{0.0f, 1.0f, 0.5f};
}

Label::Label()
    : scale_(1.0f), alignment_(TextAlignment::Left), angle_(0.0f), count_(0),
      stale_(0), width_(0),
      cutoff_(std::numeric_limits<decltype(cutoff_)>::max()), size_(0)
{
    array_.reconfigure([this] { buffer_.bind(); });
}

void Label::set_alignment(TextAlignment a)
{
    alignment_ = a;
    set_needs_update(kStaleBuffer);
}

void Label::set_color(Color c)
{
    color_ = c;
    set_needs_update(kStaleColor);
}

void Label::set_font(SharedPtr<FontAtlas> f)
{
    font_ = std::move(f);
    set_needs_update(kStaleBuffer);
}

void Label::set_position(const Vec2f& position)
{
    position_.x = std::round(position.x);
    position_.y = std::round(position.y);
    set_needs_update(kStaleBuffer);
}

void Label::set_rotation(float r)
{
    if (are_equal(r, angle_))
        return;

    angle_ = r;
    set_needs_update(kStaleBuffer);
}

void Label::set_scale(float f)
{
    if (are_equal(f, scale_))
        return;

    scale_ = clamp(f, 0.01f, 1.0f);
    set_needs_update(kStaleBuffer);
}

void Label::set_text(czstring text)
{
    const size_t len = strlen(text);
    if (len > size_)
    {
        text_ = std::make_unique<char[]>(len + 1);
        size_ = len;
        set_needs_update(kStaleBufferSize);
    }
    std::copy_n(text, len, text_.get());
    text_[len] = '\0';
    set_needs_update(kStaleBuffer);
}

void Label::move(const Vec2f& delta)
{
    position_ += delta;
    set_needs_update(kStaleBuffer);
}

void Label::update()
{
    if (stale_ != 0)
    {
        update_internal();
        upload();
        clear_state();
    }
}

void Label::update_internal()
{
    // Note: This algorithm currently does not support font kerning.
    if ((stale_ & kStaleBuffer) != 0)
    {
        if ((stale_ & kStaleBufferSize) != 0)
            vertices_ = std::make_unique<SpriteVertex[]>(size_ * 4);
        width_ = 0;
        unsigned int start = 0;
        unsigned int count = 0;
        const bool is_rotated = !is_almost_zero(angle_);
        const Vec2f R = is_rotated ? Vec2f{cosf(-angle_), sinf(-angle_)}
                                   : Vec2f::Right;
        const bool needs_alignment =
            alignment_ != TextAlignment::Left || is_rotated;
        Vec2f pen = (needs_alignment ? Vec2f::Zero : position_);
        const float origin_x = pen.x;
        SpriteVertex* vx = vertices_.get();

        for_each_utf8(
            text_.get(),
            [this, &start, &count, &pen, origin_x, R, needs_alignment, &vx](
                uint32_t ch)
            {
                if (ch == '\n')
                {
                    save(start, count, pen.x - origin_x, R, needs_alignment);
                    pen.x = origin_x;
                    start = count;
                    pen.y -= font_->height() * scale_;
                    return;
                }

                const FontGlyph* glyph = font_->get_glyph(ch);
                if (glyph == nullptr)
                    return;

                pen.x += glyph->left * scale_;

                for (auto&& quad : glyph->quad)
                {
                    vx->color = color_;
                    vx->texcoord = quad.texcoord;
                    vx->position = quad.position;
                    vx->position *= scale_;
                    vx->position += pen;
                    ++vx;
                }

                pen.x += (glyph->advance - glyph->left) * scale_;
                ++count;
            });

        count_ = count * 4;
        save(start, count, pen.x - origin_x, R, needs_alignment);
    }
    else if ((stale_ & kStaleColor) != 0)
    {
        std::for_each(vertices_.get(),
                      vertices_.get() + count_,
                      [&color = color_](SpriteVertex& v)
                      {
                          v.color = color;
                      });
    }
}

void Label::upload() const
{
    buffer_.upload(vertices_.get(), count_ * sizeof(vertices_[0]));
}

void Label::save(unsigned int start,
                 unsigned int end,
                 float width,
                 const Vec2f& R,
                 bool needs_alignment)
{
    if (needs_alignment)
    {
        std::for_each(
            vertices_.get() + start * 4,
            vertices_.get() + end * 4,
            [
              offset = width * kAlignmentFactor[static_cast<int>(alignment_)],
              sin_r = Vec2f(R.y, R.y),
              cos_r = Vec2f(R.x, R.x),
              &translate = position_
            ](SpriteVertex& v) {
                auto p = v.position;
                p.x -= offset;
                v.position = transform_srt(p, sin_r, cos_r, translate);
            });
    }
    if (width > width_)
        width_ = width;
}
