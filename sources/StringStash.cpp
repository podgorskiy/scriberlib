#include "StringStash.h"

#define XXH_INLINE_ALL
#include <xxhash.h>
#include <utf8.h>

using namespace Scriber;

const GlyphString& StringStash::GetGlyphString(const char* text, u16vec2 dpi, const Font& font)
{
	string_hash fontHash = XXH32(&font, sizeof(Font), 0);
	size_t length = strlen(text);
	string_hash hash = XXH32(text, length, reinterpret_cast<string_hash&>(dpi) ^ fontHash);

	auto lb = m_stringCache.lower_bound(hash);
	
	if (lb != m_stringCache.end() && (hash == lb->first))
	{
		return lb->second;
	}
	else
	{
		m_glyphs.clear();
		m_text_utf32.clear();
		utf8::utf8to32(text, text + length, std::back_inserter(m_text_utf32));

		auto inserter = std::back_inserter(m_glyphs);

		m_stringProcessor(m_text_utf32, font, dpi, inserter);

		m_stringCache.insert(lb, StringCache::value_type(hash, m_glyphs));
	}

	return m_glyphs;
}

void StringStash::AssignStringProcessor(const StringProcessor& processor)
{
	m_stringProcessor = processor;
}

void StringStash::Purge()
{
	m_stringCache.clear();
	m_glyphs.clear();
	m_glyphs.shrink_to_fit();
	m_text_utf32.clear();
	m_text_utf32.shrink_to_fit();
}
