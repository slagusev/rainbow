// Copyright (c) 2010-present Bifrost Entertainment AS and Tommy Nguyen
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)

#ifndef GRAPHICS_FONTGLYPH_H_
#define GRAPHICS_FONTGLYPH_H_

#include "Graphics/SpriteVertex.h"

namespace rainbow
{
    /// <summary>
    ///   Font glyph structure for storing advance and a textured sprite.
    /// </summary>
    struct FontGlyph
    {
        uint32_t code;         ///< UTF-32 code.
        int32_t advance;       ///< Horizontal advancement.
        int32_t left;          ///< Left alignment.
        SpriteVertex quad[4];  ///< Sprite vertices.

#ifdef FONTATLAS_KERNING
        int16_t kern[95];  ///< Font kerning information for each character.
#endif
    };
}

#endif
