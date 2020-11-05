#pragma once
#include "Scriber.h"
#include "Glyph.h"
#include "IRenderAPI.h"
#include <map>

typedef struct FT_BitmapGlyphRec_*  FT_BitmapGlyph;

namespace Scriber
{
	class FaceCollection;

	class GlyphBitmapStash
	{
	public:
		GlyphBitmapStash(const GlyphBitmapStash& other) = delete;
		GlyphBitmapStash& operator=(const GlyphBitmapStash&) = delete;

		GlyphBitmapStash(FT_Library lib, FaceCollection* fc, IRenderAPIPtr rednerAPI);
		~GlyphBitmapStash();

		void Purge();

		Glyph& RetrieveGlyph(GlyphID glyphIndex, GlyphID previousGlyphIndex, FaceID faceId, const Font& font, u16vec2 dpi);

		bool CheckIfOverflowedAndResetFlag() { bool tmp = m_was_overflowed;  m_was_overflowed = false; return tmp;}
	private:
		typedef uint32_t GlyphHash;
		typedef std::map<GlyphHash, Glyph> GlyphMap;

		void Stash(Glyph& glyph, FT_BitmapGlyph bitmapGlyph, FT_BitmapGlyph outlineBitmapGlyph, UserData userdata);
		
		void ResizeBitmap(uint16_t newSize);

		GlyphMap m_glyphs;
		
		uint8_t* m_bitmap;
		uint16_t m_bitmapSize;
		FaceCollection* m_fc;
		ivec2 m_stashTextureSize;
		uint16_t m_maxHeight;
		int m_spacing;
		u16vec2 m_currentPos;
		IRenderAPIPtr m_renderAPI;
		FT_Stroker m_stroker;
		FT_Library m_lib;
		Glyph m_glyph;

		bool m_was_overflowed;
	};
}
