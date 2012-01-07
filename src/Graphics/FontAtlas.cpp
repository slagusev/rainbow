/// Copyright 2010-12 Bifrost Entertainment. All rights reserved.
/// \author Tommy Nguyen

#include "Platform.h"

#ifndef RAINBOW_IOS
#	include <cassert>
#endif

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "Common/Data.h"
#include "Graphics/FontAtlas.h"

FontAtlas::FontAtlas(const float pt) : pt(pt), texture(0) { }

bool FontAtlas::load(const Data &font)
{
	assert(font || !"Rainbow::FontAtlas: No data provided");

	// Instantiate FreeType
	FT_Library lib;
	if (FT_Init_FreeType(&lib))
	{
		assert(!"Rainbow::FontAtlas: Failed to initialise FreeType");
		return false;
	}

	// Load font face
	FT_Face face;
	if (FT_New_Memory_Face(lib, static_cast<const FT_Byte*>(font.bytes()), font.size(), 0, &face))
	{
		FT_Done_FreeType(lib);
		assert(!"Rainbow::FontAtlas: Failed to load font face");
		return false;
	}

	FT_Set_Char_Size(face, 0, static_cast<int>(this->pt * 64), 96, 96);

	// Simple algorithm for calculating texture size
	unsigned int max_width = 0;
	unsigned int max_height = 0;
	unsigned int w_offset = 0;
	unsigned int h_offset = 0;
	for (unsigned int i = 0; i < chars; ++i)
	{
		if (FT_Load_Char(face, i + ascii_offset, FT_LOAD_RENDER))
		{
			FT_Done_Face(face);
			FT_Done_FreeType(lib);
			assert(!"Rainbow::FontAtlas: Failed to load characters");
			return false;
		}

		const FT_Bitmap &bitmap = face->glyph->bitmap;

		const unsigned int width = bitmap.width + (padding << 1);
		const unsigned int height = bitmap.rows + (padding << 1);
		if (width > max_width) max_width = width;
		if (height > max_height) max_height = height;
		w_offset += width;
		h_offset += height;
	}
	const unsigned int tex_width = next_pow2(w_offset / chars * 10);
	const unsigned int tex_height = next_pow2(h_offset / chars * 10);

	// GL_LUMINANCE8_ALPHA8 buffer
	w_offset = (tex_width * tex_height) << 1;
	GLubyte *tex_buf = new GLubyte[w_offset];
	memset(tex_buf, 0, w_offset);

	// Read all glyph bitmaps and copy them to our texture
	w_offset = 0;
	h_offset = 0;
	const float tex_w_fraction = 1.0f / tex_width;
	const float tex_h_fraction = 1.0f / tex_height;
	for (unsigned int i = 0; i < chars; ++i)
	{
		FT_Load_Char(face, i + ascii_offset, FT_LOAD_RENDER);

		const FT_GlyphSlot &slot = face->glyph;
		const FT_Bitmap &bitmap = slot->bitmap;

		// Make sure bitmap data has enough space
		const int width = bitmap.width + (padding << 1);
		if (w_offset + width > tex_width)
		{
			w_offset = 0;
			h_offset += max_height;
		}

		// Copy bitmap data to texture
		if (bitmap.buffer)
		{
			const unsigned char *buf = bitmap.buffer;
			unsigned char *d_ptr = tex_buf + ((h_offset * tex_width + w_offset) << 1) + ((padding << 1) * (tex_width + 1));
			unsigned int tmp = (tex_width - bitmap.width) * 2;
			for (int y = 0; y < bitmap.rows; ++y)
			{
				for (int x = 0; x < bitmap.width; ++x)
				{
				#ifdef RAINBOW_IOS

					*d_ptr = *(d_ptr + 1) = *buf;

				#else

					if (*buf)
					{
						*d_ptr = 255;
						*(d_ptr + 1) = *buf;
					}

				#endif

					d_ptr += 2;
					++buf;
				}
				d_ptr += tmp;
			}
		}

		// Save font glyph
		FontGlyph &fg = this->charset[i];
		fg.advance = static_cast<short>(slot->advance.x >> 6);
		fg.left = slot->bitmap_left;

	#ifdef FONTATLAS_KERNING

		if (FT_HAS_KERNING(face))
		{
			unsigned int glyph = i + ascii_offset;
			FT_Vector kerning;
			for (unsigned int j = 0; j < chars; ++j)
			{
				FT_Get_Kerning(face, j + ascii_offset, glyph, FT_KERNING_DEFAULT, &kerning);
				fg.kern[j] = static_cast<short>(kerning.x >> 6);
			}
		}

	#endif

		// Vertices
		fg.quad[0].position.x = static_cast<float>(bitmap.width + margin);
		fg.quad[0].position.y = static_cast<float>(slot->bitmap_top + margin);
		fg.quad[1].position.x = -margin;
		fg.quad[1].position.y = fg.quad[0].position.y;
		fg.quad[2].position.x = fg.quad[0].position.x;
		fg.quad[2].position.y = static_cast<float>(slot->bitmap_top - bitmap.rows - margin);
		fg.quad[3].position.x = fg.quad[1].position.x;
		fg.quad[3].position.y = fg.quad[2].position.y;

		// Texture coordinates
		fg.quad[0].texcoord.x = (padding + bitmap.width + margin + w_offset) * tex_w_fraction;
		fg.quad[0].texcoord.y = (padding - margin + h_offset) * tex_h_fraction;
		fg.quad[1].texcoord.x = (padding - margin + w_offset) * tex_w_fraction;
		fg.quad[1].texcoord.y = fg.quad[0].texcoord.y;
		fg.quad[2].texcoord.x = fg.quad[0].texcoord.x;
		fg.quad[2].texcoord.y = (padding + bitmap.rows + margin + h_offset) * tex_h_fraction;
		fg.quad[3].texcoord.x = fg.quad[1].texcoord.x;
		fg.quad[3].texcoord.y = fg.quad[2].texcoord.y;

		// Advance to next "slot" in texture
		w_offset += width;
	}
	FT_Done_Face(face);
	FT_Done_FreeType(lib);

	/**
	 * For printing out texture buffer
	 *
	const GLubyte *i = tex_buf;
	for (unsigned int y = 0; y < tex_height; ++y)
	{
		for (unsigned int x = 0; x < tex_width; ++x)
		{
			printf("%c", (*i > 0) ? 'X' : ' ');
			i += 2;
		}
		puts("");
	}
	 */

	glGenTextures(1, &this->texture);
	glBindTexture(GL_TEXTURE_2D, this->texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, tex_width, tex_height, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, tex_buf);

	glBindTexture(GL_TEXTURE_2D, 0);
	delete[] tex_buf;

	return true;
}
