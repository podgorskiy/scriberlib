#include "GlyphBitmapStash.h"
#include "FaceCollection.h"
#include "Image.h"

#include <freetype.h>

#define XXH_INLINE_ALL
#include <xxhash.h>

#include <algorithm>

using namespace Scriber;


GlyphBitmapStash::GlyphBitmapStash(FT_Library lib, FaceCollection* fc, IRenderAPIPtr renderAPI)

	: m_bitmap(nullptr)
	, m_bitmapSize(0)
	, m_fc(fc)
	, m_stashTextureSize(renderAPI->GetTextureSize())
	, m_maxHeight(0)
	, m_spacing(renderAPI->GetSpacing())
	, m_currentPos(m_spacing)
	, m_renderAPI(std::move(renderAPI))
	, m_stroker(nullptr)
	, m_glyph({{F26p6(0), F26p6(0), F26p6(0), i16vec2(0), u16vec2(0)} ,0, u16vec2(0), 0})
	, m_was_overflowed(false)
{
	FT_Stroker_New(lib, &m_stroker);
}

void GlyphBitmapStash::ResizeBitmap(uint16_t newSize)
{
	if (m_bitmapSize < newSize)
	{
		m_bitmapSize = NextPowerOf2(newSize);
	}
	m_bitmapSize = newSize;
	int size = m_bitmapSize * m_bitmapSize * 2;
	delete[] m_bitmap;
	m_bitmap = new uint8_t[size];
	memset(m_bitmap, 0, size);
}

GlyphBitmapStash::~GlyphBitmapStash()
{
	FT_Stroker_Done(m_stroker);
	delete[] m_bitmap;
}


inline FT_BitmapGlyph ConvertToStrokedBitmapGlyph(FT_Glyph glyph, FT_Stroker stroker, uint16_t stroke)
{
	FT_Stroker_Set(stroker, F26p6(stroke).v, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
	FT_Glyph_StrokeBorder(&glyph, stroker, 0, 0);
	FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
	FT_BitmapGlyph bitmapGlyph = (FT_BitmapGlyph)glyph;
	return bitmapGlyph;
}

inline FT_BitmapGlyph ConvertToBitmapGlyph(FT_Glyph glyph)
{
	FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 0);
	FT_BitmapGlyph bitmapGlyph = (FT_BitmapGlyph)glyph;
	return bitmapGlyph;
}

Glyph& GlyphBitmapStash::RetrieveGlyph(GlyphID glyphIndex, GlyphID previousGlyphIndex, FaceID faceId, const Font& font, u16vec2 dpi)
{
	struct Data
	{
		GlyphID glyphIndex;
		FaceID faceId;
		uint16_t height;
		FontStyle::Enum style;
		uint16_t stroke;
		u16vec2 dpi;
	} data;
	memset(&data, 0, sizeof(Data));
	
	data.glyphIndex = glyphIndex;
	data.faceId = faceId;
	data.height = font.height;
	data.style = font.style;
	data.stroke = font.stroke;
	data.dpi = dpi;

	GlyphHash hash = XXH32(&data, sizeof(Data), 0);

	auto lb = m_glyphs.lower_bound(hash);

	if (lb != m_glyphs.end() && (hash == lb->first))
	{
		return lb->second;
	}
	else
	{
        FT_Face face = m_fc->GetFace(faceId);

		FT_Set_Char_Size(face, 0, F26p6(font.height).v, dpi.x, dpi.y);

		FT_Error error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_NO_BITMAP);

        if (error == FT_Err_Ok)
        {
			FT_Glyph ftglyph = nullptr;

			FT_Error error = FT_Get_Glyph(face->glyph, &ftglyph);

			if (error == FT_Err_Ok)
			{
				m_glyph.m_metrics.horiAdvance.v = face->glyph->metrics.horiAdvance;
				//glyph.m_metrics.vertAdvance.v = face->glyph->metrics.vertAdvance;
				m_glyph.m_metrics.ascender.v = face->size->metrics.ascender;
				m_glyph.m_metrics.descender.v = face->size->metrics.descender;

				bool use_kerning = FT_HAS_KERNING( face );
				if (use_kerning && previousGlyphIndex != 0 && glyphIndex != 0)
				{
					FT_Vector  delta;
					FT_Get_Kerning(face, previousGlyphIndex, glyphIndex, FT_KERNING_DEFAULT, &delta);
					m_glyph.m_metrics.horiAdvance.v += delta.x;
				}

				FT_BitmapGlyph ftbitmapGlyph = ConvertToBitmapGlyph(ftglyph);

				if(font.stroke > 0)
				{
					FT_BitmapGlyph ftoutlinebitmapGlyph = ConvertToStrokedBitmapGlyph(ftglyph, m_stroker, font.stroke);

					Stash(m_glyph, ftbitmapGlyph, ftoutlinebitmapGlyph, font.userdata);

					FT_Done_Glyph((FT_Glyph)ftoutlinebitmapGlyph);
					FT_Done_Glyph((FT_Glyph)ftbitmapGlyph);
					FT_Done_Glyph((FT_Glyph)ftglyph);
				}
				else
				{
					Stash(m_glyph, ftbitmapGlyph, nullptr, font.userdata);

					FT_Done_Glyph((FT_Glyph)ftbitmapGlyph);
					FT_Done_Glyph((FT_Glyph)ftglyph);
				}
			}
        }
		else
		{
			FaceID result = m_fc->GetFaceIDFromCode(0x25A1, font.preferred_tf, font.style);
			FT_UInt glyphIndex = FT_Get_Char_Index(m_fc->GetFace(result), 0x25A1);
			m_glyph = RetrieveGlyph(glyphIndex, 0, result, font, dpi);
			m_glyph.m_code = 0;
			return m_glyph;
		}
		if (m_glyphs.size() == 0)
		{
			m_glyphs.insert(GlyphMap::value_type(hash, m_glyph));
			lb = m_glyphs.find(hash);
		}
		else
		{
			lb = m_glyphs.insert(lb, GlyphMap::value_type(hash, m_glyph));
		}
		return lb->second;
    }
}

void GlyphBitmapStash::Stash(Glyph& glyph, FT_BitmapGlyph bitmapGlyph, FT_BitmapGlyph outlineBitmapGlyph, UserData userdata)
{
	Image image;

	if (outlineBitmapGlyph != nullptr)
	{
		FT_Bitmap& outlinebitmap = outlineBitmapGlyph->bitmap;
		FT_Bitmap& fillbitmap = bitmapGlyph->bitmap;

		ResizeBitmap(std::max(outlinebitmap.width, outlinebitmap.rows));

		image = Image::FromMemory(m_bitmap, Image::RG8, ivec2(outlinebitmap.width, outlinebitmap.rows), 1);
		Image fill = Image::FromMemory(fillbitmap.buffer, Image::R8, ivec2(fillbitmap.width, fillbitmap.rows), 1);
		Image outline = Image::FromMemory(outlinebitmap.buffer, Image::R8, ivec2(outlinebitmap.width, outlinebitmap.rows), 1);

		image.AssignToChannel(outline, 1);
		ivec2 offset = ivec2(bitmapGlyph->left - outlineBitmapGlyph->left, outlineBitmapGlyph->top - bitmapGlyph->top);
		image.OpenView(offset, ivec2(fillbitmap.width, fillbitmap.rows)).AssignToChannel(fill, 0);

		//image.SaveToTGA("TEST_Outline.tga");

		glyph.m_metrics.glyphSize.x = outlinebitmap.width;
		glyph.m_metrics.glyphSize.y = outlinebitmap.rows;
		glyph.m_metrics.horizontalBearing.x = outlineBitmapGlyph->left;
		glyph.m_metrics.horizontalBearing.y = outlineBitmapGlyph->top;
	}
	else
	{
		FT_Bitmap& bitmap = bitmapGlyph->bitmap;

		ResizeBitmap(std::max(bitmap.width, bitmap.rows));

		image = Image::FromMemory(m_bitmap, Image::RG8, ivec2(bitmap.width, bitmap.rows), 1);
		Image fill = Image::FromMemory(bitmap.buffer, Image::R8, ivec2(bitmap.width, bitmap.rows), 1);
		image.AssignToChannelZeroOther(fill, 0);

		//image.SaveToTGA("TEST_fill.tga");

		glyph.m_metrics.glyphSize.x = bitmap.width;
		glyph.m_metrics.glyphSize.y = bitmap.rows;
		glyph.m_metrics.horizontalBearing.x = bitmapGlyph->left;
		glyph.m_metrics.horizontalBearing.y = bitmapGlyph->top;
	}

	if (bitmapGlyph->bitmap.buffer == nullptr)
		return;

	if (glyph.m_metrics.glyphSize.x + m_currentPos.x + m_spacing >= m_stashTextureSize.x)
	{
		m_currentPos.x = m_spacing;
		m_currentPos.y = m_maxHeight + m_spacing;
	}
	m_maxHeight = std::max(m_maxHeight, (uint16_t)(m_currentPos.y + glyph.m_metrics.glyphSize.y));

	if (m_maxHeight > m_stashTextureSize.y)
	{
		Purge();
		m_was_overflowed = true;
	}

	glyph.m_cacheUV = m_currentPos;
	m_renderAPI->UpdateTexture(image, m_currentPos);

	m_currentPos.x += glyph.m_metrics.glyphSize.x + m_spacing;

	//m_renderAPI->SaveTextureToFile();
}


void GlyphBitmapStash::Purge()
{
	m_glyphs.clear();
	m_currentPos = u16vec2(m_spacing);
	m_maxHeight = m_spacing;

	m_renderAPI->ClearTexture();
}

