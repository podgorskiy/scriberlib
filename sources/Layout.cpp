#include "Layout.h"
#include "minibidi.h"

#include <freetype.h>

#define USE_HARFBUZZ
#ifdef USE_HARFBUZZ

#include <hb.h>
#include <hb-ft.h>

#endif

using namespace Scriber;

LayoutEngine::LayoutEngine(FaceCollection* collection) :m_faceCollection(collection)
{
#ifdef USE_HARFBUZZ
	m_HBbuf = hb_buffer_create();
#endif
}

void LayoutEngine::ShapeFragment(utf32string& text, FaceID faceId, u16vec2 dpi, const Font& font)
{
	FT_Face face = m_faceCollection->GetFace(faceId);

#ifdef USE_HARFBUZZ
	hb_font_t* hb_font = m_faceCollection->GetHBFontByFaceId(faceId);
	
	FT_Set_Char_Size(hb_ft_font_get_face(hb_font), 0, F26p6(font.height).v, dpi.x, dpi.y);
	hb_font_set_scale(hb_font, (F26p6(font.height).v * dpi.x) / 72, (F26p6(font.height).v * dpi.y) / 72);

	hb_buffer_clear_contents(m_HBbuf);
	hb_buffer_set_content_type(m_HBbuf, HB_BUFFER_CONTENT_TYPE_UNICODE);
	hb_buffer_set_direction(m_HBbuf, HB_DIRECTION_LTR);
	hb_buffer_add_utf32(m_HBbuf, &text[0], text.size(), 0, -1);
	hb_buffer_set_unicode_funcs(m_HBbuf, hb_unicode_funcs_get_default());
	hb_buffer_guess_segment_properties(m_HBbuf);


	hb_shape(hb_font, m_HBbuf, NULL, 0);

	unsigned int         glyph_count;
	hb_glyph_info_t     *glyph_info = hb_buffer_get_glyph_infos(m_HBbuf, &glyph_count);
	hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(m_HBbuf, &glyph_count);
#endif

	m_fragmentshapedData.clear();
	if (false)
	{
		for (int i = 0, s = text.size(); i < s; ++i)
		{
			FT_UInt glyphIndex = FT_Get_Char_Index(face, text[i]);
			LayoutData layout;
			layout.advance.v = 0;
			layout.offset = i16vec2(0);
			layout.glyph = static_cast<uint16_t>(glyphIndex);
			layout.group = static_cast<uint16_t>(i);
			layout.id = faceId;
			layout.code = text[layout.group];
			m_fragmentshapedData.push_back(layout);
		}
	}
	else
	{
		for (int i = 0; i < (int)glyph_count; ++i)
		{
			LayoutData layout;
			layout.advance.v = glyph_pos[i].x_advance;
			layout.offset.x = (int)F26p6V(glyph_pos[i].x_offset);
			layout.offset.y = (int)F26p6V(glyph_pos[i].y_offset);

			layout.glyph = glyph_info[i].codepoint;
			layout.group = glyph_info[i].cluster;
			layout.id = faceId;
			layout.code = text[layout.group];
			m_fragmentshapedData.push_back(layout);
		}
	}
}

void LayoutEngine::FlushShapedData(const std::vector<LayoutData>& data)
{
	for (LayoutDataString::const_iterator it = data.begin(); it != data.end(); ++it)
	{
		m_shapedData.push_back(*it);
	}
}

const LayoutDataString& LayoutEngine::Process(utf32string& text, int start, int end, u16vec2 dpi, const Font& font)
{
	m_fragment.clear();
	m_shapedData.clear();
	
	if (!!(m_mode | LayoutMode::MinibidiReordering))
	{
		int lastPosition = start;
		for (int i = start; (i < end) && (text[i] != 0); i++)
		{
			if (text[i] == '\n')
			{
				doBidi(&text[lastPosition], i - lastPosition, 0, 0, nullptr, nullptr);
				lastPosition = i;
			}
		}
		doBidi(&text[lastPosition], end - lastPosition, 1, 0, nullptr, nullptr);
	}

	FaceID prevFaceID = m_faceCollection->GetFaceIDFromCode(text[0], font.preferred_tf, font.style);
	FaceID faceId = InvalidFaceID;

	for (int i = start; (i < end) && (text[i] != 0); i++)
	{
		faceId = m_faceCollection->GetFaceIDFromCode(text[i], font.preferred_tf, font.style);

		if (faceId != prevFaceID || text[i] == '\n')
		{
			ShapeFragment(m_fragment, prevFaceID, dpi, font);
			FlushShapedData(m_fragmentshapedData);

			m_fragment.clear();
		}
		m_fragment.push_back(text[i]);
		prevFaceID = faceId;
	}

	ShapeFragment(m_fragment, prevFaceID, dpi, font);

	FlushShapedData(m_fragmentshapedData);

	return m_shapedData;
}

void LayoutEngine::SetMode(LayoutMode::Enum mode)
{
	m_mode = mode;
}
