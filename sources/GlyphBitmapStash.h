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
		GlyphBitmapStash(FT_Library lib, FaceCollection* fc, IRenderAPIPtr rednerAPI);
		~GlyphBitmapStash();

		void Purge();

		Glyph& RetrieveGlyph(GlyphID glyphIndex, GlyphID previousGlyphIndex, FaceID faceId, const Font& font, u16vec2 dpi);

	private:
		typedef uint32_t GlyphHash;
		typedef std::map<GlyphHash, Glyph> GlyphMap;

		void Stash(Glyph& glyph, const FT_BitmapGlyph bitmapGlyph, const FT_BitmapGlyph outlineBitmapGlyph, UserData userdata);
		
		void ResizeBitmap(uint16_t newSize);

		ivec2 m_stashTextureSize;
		int m_spacing;

		u16vec2 m_currentPos;
		uint16_t m_maxHeight;

		GlyphMap m_glyphs;
		
		uint8_t* m_bitmap;
		uint16_t m_bitmapSize;

		FT_Stroker m_stroker;
		FaceCollection* m_fc;

		IRenderAPIPtr m_renderAPI;
		Glyph m_glyph;
	};
}
