#pragma once
#include "Utils.h"

#include <vector>
#include <iterator>

namespace Scriber
{
	struct Glyph
	{
		struct Metrics
		{
			/// The horizontal distance to increment the pen position when the glyph is drawn as part of a string of text.
			F26p6       horiAdvance;
			//F26p6       vertAdvance;

			/// The distance from the baseline to the highest or upper grid coordinate used to place
			/// an outline point. It is a positive value, due to the grid's orientation with the Y
			/// axis upwards.
			F26p6       ascender;

			/// The distance from the baseline to the lowest grid coordinate used to place an
			/// outline point. In FreeType, this is a negative value, due to the grid's orientation.
			/// Note that in some font formats this is a positive value.
			F26p6       descender;

			/// The horizontal/vertical distance from the current pen position to the glyph's left/top
			/// bbox edge. It is positive for horizontal layouts, and in most cases negative for vertical ones.
			i16vec2     horizontalBearing;
			//F26p6vec2   verticalBearing;

			/// Glyph bitmap's bounding box. It is independent of the layout direction.
			u16vec2     glyphSize;

			//F26p6vec2   outlineSize;
		} m_metrics;

		uint32_t m_code; // ?
		u16vec2  m_cacheUV;
		uint32_t m_color;
	};

	typedef std::vector<Glyph> GlyphString;
	typedef std::back_insert_iterator<GlyphString> GlyphStringInsert;
}
