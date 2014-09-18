// Copyright (c) 2010-15 Bifrost Entertainment AS and Tommy Nguyen
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)

#ifndef GRAPHICS_LYRICALLABEL_H_
#define GRAPHICS_LYRICALLABEL_H_

#include <vector>

#include "Graphics/Label.h"

class LyricalLabel : private Label
{
public:
	struct Attribute
	{
		enum class Type
		{
			Color,
			Offset
		} type;
		unsigned int start;
		unsigned int length;

		union
		{
			unsigned char color[4];
			int offset[2];
		};

		Attribute(const Colorb color,
		          const unsigned int start,
		          const unsigned int len);

		Attribute(const Vec2i &offset,
		          const unsigned int start,
		          const unsigned int len);
	};

	Label* as_label() { return static_cast<Label*>(this); }

	void clear_attributes() { attributes_.clear(); }
	void clear_attributes(const Attribute::Type type);

	void set_color(const Colorb c,
	               const unsigned int start,
	               const unsigned int length);

	void set_offset(const Vec2i &offset,
	                const unsigned int start,
	                const unsigned int length);

	void update() override;

	using Label::color;
	using Label::count;
	using Label::font;
	using Label::position;
	using Label::text;
	using Label::vertex_array;
	using Label::width;
	using Label::set_alignment;
	using Label::set_color;
	using Label::set_font;
	using Label::set_needs_update;
	using Label::set_position;
	using Label::set_rotation;
	using Label::set_scale;
	using Label::set_text;
	using Label::bind_textures;
	using Label::move;

private:
	std::vector<Attribute> attributes_;
};

#endif