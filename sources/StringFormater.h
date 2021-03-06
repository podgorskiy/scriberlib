#pragma once
#include "Scriber.h"
#include "Glyph.h"
#include "Layout.h"
#include "GlyphBitmapStash.h"

#include <map>

namespace Scriber
{
	class StringFormater
	{
	public:
		StringFormater(LayoutEngine* le, GlyphBitmapStash* gs);

		void Format(utf32string& string, const Font& font, u16vec2 dpi, GlyphStringInsert& inserter);

	private:
		LayoutEngine* m_layout;
		GlyphBitmapStash* m_glyphStash;
		uint16_t m_empty_characters_replacement;
	};
}
