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
	m_HBbuf = hb_buffer_create();
	hb_buffer_set_direction(m_HBbuf, HB_DIRECTION_LTR);
}

void LayoutEngine::ShapeFragment(utf32string& text, FaceID faceId, u16vec2 dpi, const Font& font)
{
	FT_Face face = m_faceCollection->GetFace(faceId);

	FT_Set_Char_Size(face, 0, F26p6(font.height).v, dpi.x, dpi.y);

	m_fragmentshapedData.clear();

	for (int i = 0; text[i] != 0; ++i)
	{
		FT_Fixed advance = 0;
		FT_UInt glyphIndex = FT_Get_Char_Index(face, text[i]);
		LayoutData layout;
		layout.advance = 0;// ToFloatFromF26(advance);
		layout.offset = vec2(0);
		layout.glyph = static_cast<uint16_t>(glyphIndex);
		layout.group = static_cast<uint16_t>(i);
		layout.id = faceId;
		layout.code = text[layout.group];
		m_fragmentshapedData.push_back(layout);
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
	
	FaceID prevFaceID = m_faceCollection->GetFaceIDFromCode(text[0], font.preferred_tf, font.style);

	FaceID faceId = InvalidFaceID;

	int lastPosition = start;
	for (int i = start; (i < end) && (text[i] != 0); i++)
	{
		if (text[i] == '\n')
		{
			doBidi(&text[lastPosition], i - lastPosition, 1, 0, nullptr, nullptr);
			lastPosition = i;
		}
	}
	doBidi(&text[lastPosition], end - lastPosition, 1, 0, nullptr, nullptr);

	for (int i = start; (i < end) && (text[i] != 0); i++)
	{
		faceId = m_faceCollection->GetFaceIDFromCode(text[i], font.preferred_tf, font.style);

		if (faceId != prevFaceID || text[i] == '\n')
		{
			m_fragment.push_back(0);
			ShapeFragment(m_fragment, prevFaceID, dpi, font);
			FlushShapedData(m_fragmentshapedData);

			m_fragment.clear();
		}
		m_fragment.push_back(text[i]);
		prevFaceID = faceId;
	}

	m_fragment.push_back(0);
	ShapeFragment(m_fragment, prevFaceID, dpi, font);

	FlushShapedData(m_fragmentshapedData);

	return m_shapedData;
}
