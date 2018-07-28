#pragma once
#include "Scriber.h"
#include "Glyph.h"

#include <map>
#include <functional>

namespace Scriber
{
	typedef std::function<void(utf32string& string, const Font& font, u16vec2 dpi, GlyphStringInsert& inserter)> StringProcessor;

	class StringStash
	{
	public:
		const GlyphString& GetGlyphString(const char* text, u16vec2 dpi, const Font& font);

		void AssignStringProcessor(const StringProcessor& processor);

		void Purge();

	private:
		typedef uint32_t string_hash;
		typedef std::map<string_hash, GlyphString> StringCache;

		StringProcessor m_stringProcessor;
		StringCache     m_stringCache;
		utf32string     m_text_utf32;
		GlyphString     m_glyphs;
	};
}
