#pragma once
#include "Scriber.h"
#include "FaceCollection.h"
#include "Utils.h"

#include <vector>

namespace Scriber
{
	struct LayoutData
	{
		uint32_t  code;
		uint16_t  glyph;
		uint16_t  group;
		float     advance;
		vec2      offset;
		FaceID    id;
	};

	typedef std::vector<LayoutData> LayoutDataString;

	class LayoutEngine
	{
	public:
		LayoutEngine(FaceCollection* collection);

		const LayoutDataString& Process(utf32string& text, int start, int end, u16vec2 dpi, const Font& font);

	private:
		void ShapeFragment(utf32string& text, FaceID faceId, u16vec2 dpi, const Font& font);
		void FlushShapedData(const std::vector<LayoutData>& data);

		FaceCollection*         m_faceCollection;
		LayoutDataString        m_shapedData;
		LayoutDataString        m_fragmentshapedData;
		utf32string             m_fragment;

		hb_buffer_t*			m_HBbuf;
	};
}
