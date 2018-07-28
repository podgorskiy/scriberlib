#include "GlyphBitmapStash.h"
#include "FaceCollection.h"
#include "Image.h"

#include <freetype.h>

#define XXH_INLINE_ALL
#include <xxhash.h>

#include <algorithm>

using namespace Scriber;

/*
int const GlyphCache::k_bakePoolSize = 1000;
int const GlyphCache::k_textureSizeX = 1024;
int const GlyphCache::k_textureSizeY = 1024;
int const GlyphCache::k_spacing = 3;
*/

GlyphBitmapStash::GlyphBitmapStash(FT_Library lib, FaceCollection* fc, IRenderAPIPtr renderAPI)
    //: m_cachePosX(k_spacing)
    //, m_cachePosY(k_spacing)
    //, m_cacheMaxY(k_spacing)
	: m_bitmap(nullptr)
	, m_bitmapSize(0)
	, m_fc(fc)
	, m_stashTextureSize(1024)
	, m_maxHeight(0)
	, m_spacing(2)
	, m_currentPos(m_spacing)
	, m_renderAPI(renderAPI)
{
	FT_Stroker_New(lib, &m_stroker);

	/*
	m_rawTexture = Texture::GenerateTexture();
	m_rawTexture->Bind(0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, k_textureSizeX, k_textureSizeY, 0, GL_RG, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	m_rawTexture->UnBind();
	*/

	//m_backeryBuffer.Init(k_textureSizeX, k_textureSizeY, false, false);
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
		Font font;
		u16vec2 dpi;
	} data = { glyphIndex, faceId, font, dpi };
	
	data.font.preferred_tf = 0;

	GlyphHash hash = XXH32(&data, sizeof(Data), 0);

	GlyphMap::iterator lb = m_glyphs.lower_bound(hash);

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
						
			//bool use_kerning = FT_HAS_KERNING(face) != 0;
			//if (use_kerning && previousGlyphIndex != 0 && glyphIndex != 0)
			//{
			//	FT_Vector  delta;
			//	FT_Get_Kerning(face, previousGlyphIndex, glyphIndex, FT_KERNING_DEFAULT, &delta);
			//	newGlyph.metricsHoriAdvance += F26D6ToFloat(delta.x);
			//}
			
			if (error == FT_Err_Ok)
			{
				m_glyph.m_metrics.horiAdvance.v = face->glyph->metrics.horiAdvance;
				//glyph.m_metrics.vertAdvance.v = face->glyph->metrics.vertAdvance;
				m_glyph.m_metrics.ascender.v = face->size->metrics.ascender;
				m_glyph.m_metrics.descender.v = face->size->metrics.descender;

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

void GlyphBitmapStash::Stash(Glyph& glyph, const FT_BitmapGlyph bitmapGlyph, const FT_BitmapGlyph outlineBitmapGlyph, UserData userdata)
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

	if (glyph.m_metrics.glyphSize.x + m_currentPos.x + m_spacing >= m_stashTextureSize.x)
	{
		m_currentPos.x = m_spacing;
		m_currentPos.y = m_maxHeight + m_spacing;
	}
	m_maxHeight = std::max(m_maxHeight, (uint16_t)(m_currentPos.y + glyph.m_metrics.glyphSize.y));

	if (m_maxHeight > m_stashTextureSize.y)
	{
		Purge();
	}

	glyph.m_cacheUV = m_currentPos;
	m_renderAPI->UpdateTexture(image, m_currentPos);

	m_currentPos.x += glyph.m_metrics.glyphSize.x + m_spacing;

	//m_renderAPI->SaveTextureToFile();
}

/*
void GlyphBitmapStash::PutGlyphToCache(GlyphCacheEntry& glyph, const uint8_t* bitmap, const uint8_t* bitmapOutline, const uint8_t shadow)
{	
	m_rawTexture->Bind(0);

	GLint oldUnpackAlignment;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &oldUnpackAlignment);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexSubImage2D(GL_TEXTURE_2D, 0, m_cachePosX, m_cachePosY, glyph.bitmapWidth, glyph.bitmapHeight, GL_RG, GL_UNSIGNED_BYTE, m_bitmapBuffer);
	m_rawTexture->UnBind();

	if (oldUnpackAlignment != 1)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, oldUnpackAlignment);
	}
}

void GlyphBitmapStash::Bake(Shader* shader)
{
	if(m_backeCacheIt > 0)
	{
		m_backeryBuffer.BindFBO();
		glViewport(0, 0, k_textureSizeX, k_textureSizeY);
		m_rawTexture->Bind(0);
		for(int i =0; i < m_backeCacheIt; ++i)
		{
			glm::vec2 uv0 = glm::vec2(m_backeCache[i].uv0.x,  m_backeCache[i].uv0.y + m_backeCache[i].uvt.y);
			glm::vec2 uvt = glm::vec2(m_backeCache[i].uvt.x, -m_backeCache[i].uvt.y);
			m_backerySprite->SetUv0(uv0);
			m_backerySprite->SetUvt(uvt);
			m_backerySprite->Draw(*shader);
		}		
	}
	m_backeCacheIt = 0;
}

*/

void GlyphBitmapStash::Purge()
{
	m_glyphs.clear();
	m_currentPos = u16vec2(m_spacing);
	m_maxHeight = m_spacing;

	m_renderAPI->ClearTexture();

	//char data[2] = { 0 };
	//glClearTexImage(GL_TEXTURE_2D, 0, GL_RG8, GL_UNSIGNED_BYTE, &data);
}

