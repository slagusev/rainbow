// Copyright (c) 2010-present Bifrost Entertainment AS and Tommy Nguyen
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)

#include "Graphics/TextLayout.h"

#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>

#include "Common/Logging.h"

using rainbow::TextLayout;

namespace
{
    /// <summary>Horizontal/vertical resolution in dpi.</summary>
    constexpr uint32_t kDPI = 96;

    /// <summary>26.6 fixed-point pixel coordinates.</summary>
    constexpr int kPixelFormat = 64;
}  // namespace

TextLayout::TextLayout()
{
    FT_Init_FreeType(&library_);
}

TextLayout::~TextLayout()
{
    for (auto&& face : faces_)
        FT_Done_Face(face.second);
    FT_Done_FreeType(library_);
}

void TextLayout::draw(const std::string& text,
                      const std::string& font_face,
                      int font_size)
{
    FT_Face face;
    auto search = faces_.find(font_face);
    if (search == faces_.end())
    {
        [[maybe_unused]] FT_Error error = FT_New_Memory_Face(
            library_,
            nullptr,  // static_cast<const FT_Byte*>(font.bytes()),
            0,        // static_cast<FT_Long>(font.size()),
            0,
            &face);

        R_ASSERT(error == 0, "Failed to load font face");
        R_ASSERT(FT_IS_SCALABLE(face), "Unscalable fonts are not supported");

        error = FT_Select_Charmap(face, FT_ENCODING_UNICODE);

        R_ASSERT(error == 0, "Failed to select Unicode character map");

        faces_[font_face] = face;
    }
    else
    {
        face = search->second;
    }

    FT_Set_Char_Size(face, 0, font_size * kPixelFormat, kDPI, kDPI);

    hb_buffer_t* hb_buffer_ = hb_buffer_create();
    hb_buffer_reset(hb_buffer_);
    hb_buffer_add_utf8(
        hb_buffer_, text.c_str(), text.length(), 0, text.length());
    hb_buffer_guess_segment_properties(hb_buffer_);
    hb_font_t* font = hb_ft_font_create(face, nullptr);
    hb_shape(font, hb_buffer_, nullptr, 0);

    Vec2i cursor;

    unsigned int length;
    hb_glyph_info_t* info = hb_buffer_get_glyph_infos(hb_buffer_, &length);
    hb_glyph_position_t* positions =
        hb_buffer_get_glyph_positions(hb_buffer_, &length);
    for (unsigned int i = 0; i < length; ++i)
    {
        const auto codepoint = info[i].codepoint;
        auto search = glyphs_.find({face, font_size, codepoint});
        if (search == glyphs_.end())
        {
            FT_Load_Char(face, codepoint, FT_LOAD_RENDER);
            FT_GlyphSlot slot = face->glyph;
            const FT_Bitmap& bitmap = slot->bitmap;
            // Copy bitmap data to texture.
            // Save font glyph data.
        }

        hb_glyph_position_t position = positions[i];

        Vec2i offset{
            position.x_offset / kPixelFormat, position.y_offset / kPixelFormat};

        cursor.x += position.x_advance / kPixelFormat;
        cursor.y += position.y_advance / kPixelFormat;

        // Vertex data
    }
}
