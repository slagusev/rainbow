// Copyright (c) 2010-present Bifrost Entertainment AS and Tommy Nguyen
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)

#ifndef GRAPHICS_TEXTLAYOUT_H_
#define GRAPHICS_TEXTLAYOUT_H_

#include <string>
#include <unordered_map>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif
#include <ft2build.h>  // NOLINT(llvm-include-order)
#include FT_FREETYPE_H
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#include "Math/Vec2.h"

namespace rainbow
{
    struct GlyphInfo
    {
        Vec2f x0;
        Vec2f x1;
        Vec2f y0;
        Vec2f y1;
        uint32_t use_count;
    };

    struct Glyph
    {
        FT_Face face;
        int32_t font_size;
        uint32_t code;

        friend bool operator==(const Glyph& lhs, const Glyph& rhs)
        {
            return lhs.face == rhs.face && lhs.font_size == rhs.font_size &&
                   lhs.code == rhs.code;
        }
    };

    template <typename T>
    void hash_combine(size_t& seed, T&& value)
    {
        const auto hash = std::hash<std::decay_t<T>>{}(value);
        seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
}  // namespace rainbow

namespace std
{
    template <>
    struct hash<rainbow::Glyph>
    {
        auto operator()(const rainbow::Glyph& glyph) const -> size_t
        {
            size_t seed = 0;
            rainbow::hash_combine(seed, glyph.face);
            rainbow::hash_combine(seed, glyph.font_size);
            rainbow::hash_combine(seed, glyph.code);
            return seed;
        }
    };
}  // namespace std

namespace rainbow
{
    // TODO: Split out FontCache
    class TextLayout
    {
    public:
        TextLayout();
        ~TextLayout();

        void draw(const std::string& str,
                  const std::string& font_face,
                  int font_size);

    private:
        FT_Library library_;
        std::unordered_map<std::string, FT_Face> faces_;
        std::unordered_map<Glyph, GlyphInfo> glyphs_;
    };
}  // namespace rainbow

#endif
