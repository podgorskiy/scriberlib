#include "StringFormater.h"

using namespace Scriber;

StringFormater::StringFormater(LayoutEngine* le, GlyphBitmapStash* gs)
	: m_layout(le)
	, m_glyphStash(gs)
{
}

void StringFormater::Format(utf32string& string, const Font& font, u16vec2 dpi, GlyphStringInsert& inserter)
{
	const LayoutDataString& layout = m_layout->Process(string, 0, string.size() - 1, dpi, font);

	uint16_t lastGlyph = 0;

	for (auto it = layout.begin(); it != layout.end(); ++it)
	{
		Glyph glyph = m_glyphStash->RetrieveGlyph(it->glyph, lastGlyph, it->id, font, dpi);
		
		glyph.m_code = it->code;
		glyph.m_color = font.color;
		glyph.m_metrics.horizontalBearing.x += it->offset.x;
		glyph.m_metrics.horizontalBearing.y += it->offset.y;
		glyph.m_metrics.horiAdvance.v = it->advance.v;

		inserter = glyph;

		lastGlyph = it->glyph;
	}
}
