// Copyright (c) 2010-present Bifrost Entertainment AS and Tommy Nguyen
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)

#include "Graphics/SpriteBatch.h"

#include "Graphics/Renderer.h"

using rainbow::SharedPtr;
using rainbow::SpriteBatch;
using rainbow::SpriteRef;
using rainbow::SpriteVertex;
using rainbow::TextureAtlas;
using rainbow::Vec2f;

namespace
{
    constexpr auto operator"" _z(unsigned long long int u) -> size_t
    {
        return u;
    }
}  // namespace

SpriteBatch::SpriteBatch(uint32_t count)
    : sprites_(count), vertices_(std::make_unique<SpriteVertex[]>(count * 4_z)),
      count_(0), visible_(true)
{
    R_ASSERT(count <= graphics::kMaxSprites, "Hard-coded limit reached");

    array_.reconfigure([this] { bind_arrays(); });
}

SpriteBatch::SpriteBatch(SpriteBatch&& batch) noexcept
    : sprites_(std::move(batch.sprites_)),
      vertices_(std::move(batch.vertices_)),
      normals_(std::move(batch.normals_)), count_(batch.count_),
      vertex_buffer_(std::move(batch.vertex_buffer_)),
      normal_buffer_(std::move(batch.normal_buffer_)),
      array_(std::move(batch.array_)), normal_(std::move(batch.normal_)),
      texture_(std::move(batch.texture_)), visible_(batch.visible_)
{
    batch.clear();
}

void SpriteBatch::set_normal(SharedPtr<TextureAtlas> texture)
{
    if (!normals_)
    {
        normals_ = std::make_unique<Vec2f[]>(sprites_.size() * 4_z);
        array_.reconfigure([this] { bind_arrays(); });
    }

    normal_ = std::move(texture);
}

void SpriteBatch::set_texture(SharedPtr<TextureAtlas> texture)
{
    texture_ = std::move(texture);
}

void SpriteBatch::bind_textures() const
{
    if (normal_)
        normal_->bind(1);
    texture_->bind();
}

void SpriteBatch::bring_to_front(uint32_t i)
{
    sprites_.move(i, count_ - 1);
}

auto SpriteBatch::create_sprite(uint32_t width, uint32_t height) -> SpriteRef
{
    if (count_ == sprites_.size())
    {
        LOGW(
            "Tried to add a sprite (size: %ux%u) to a full SpriteBatch "
            "(capacity: %u). Increase the capacity and try again.",
            width,
            height,
            sprites_.size());
        return {};
    }

    new (sprites_.data() + count_) Sprite(width, height);
    const uint32_t offset = count_ * 4;
    std::fill_n(vertices_.get() + offset, 4, SpriteVertex{});
    if (normals_)
        std::fill_n(normals_.get() + offset, 4, Vec2f::Zero);
    return {*this, sprites_.find_iterator(count_++)};
}

void SpriteBatch::erase(uint32_t i)
{
    bring_to_front(i);
    sprites_.data()[--count_].~Sprite();
}

auto SpriteBatch::find_sprite_by_id(int id) const -> SpriteRef
{
    auto sprites = sprites_.data();
    for (uint32_t i = 0; i < count_; ++i)
    {
        if (sprites[i].id() == id)
            return {*const_cast<SpriteBatch*>(this), sprites_.find_iterator(i)};
    }

    return {};
}

void SpriteBatch::move(const Vec2f& delta)
{
    if (delta.is_zero())
        return;

    for (auto&& sprite : *this)
        sprite.move(delta);
}

void SpriteBatch::swap(uint32_t i, uint32_t j)
{
    if (i == j)
        return;

    sprites_.swap(i, j);
}

void SpriteBatch::update()
{
    bool needs_update = false;
    auto sprites = sprites_.data();
    if (normals_)
    {
        for (uint32_t i = 0; i < count_; ++i)
        {
            ArraySpan<Vec2f> normal_buffer{normals_.get() + i * 4, 4};
            ArraySpan<SpriteVertex> vertex_buffer{vertices_.get() + i * 4, 4};
            needs_update |= sprites[i].update(normal_buffer, *normal_) |
                            sprites[i].update(vertex_buffer, *texture_);
        }
    }
    else
    {
        for (uint32_t i = 0; i < count_; ++i)
        {
            ArraySpan<SpriteVertex> buffer{vertices_.get() + i * 4, 4};
            needs_update |= sprites[i].update(buffer, *texture_);
        }
    }

    if (needs_update)
    {
        const uint32_t count = count_ * 4;
        vertex_buffer_.upload(vertices_.get(), count * sizeof(SpriteVertex));
        if (normals_)
            normal_buffer_.upload(normals_.get(), count * sizeof(Vec2f));
    }
}

void SpriteBatch::bind_arrays() const
{
    vertex_buffer_.bind();
    if (normals_)
        normal_buffer_.bind(Shader::kAttributeNormal);
}

#ifdef RAINBOW_TEST
SpriteBatch::SpriteBatch(const rainbow::ISolemnlySwearThatIAmOnlyTesting& test)
    : sprites_(4), vertices_(std::make_unique<SpriteVertex[]>(4 * 4)),
      count_(0), vertex_buffer_(test), normal_buffer_(test),
      texture_(make_shared<TextureAtlas>(test)), visible_(true)
{
    texture_->add_region(0, 0, 1, 1);
}
#endif  // RAINBOW_TEST
