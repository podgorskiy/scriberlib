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
		F26p6     advance;
		i16vec2   offset;
		FaceID    id;
	};


	struct LayoutMode
	{
		enum Enum
		{
			None                  = 0,
			MinibidiReordering    = 1 << 0,
			MinibidiShaping       = 1 << 1,
			HarfbuzzShaping       = 1 << 2,

			Basic = None,
			SimpleShaping_RTL = MinibidiReordering | MinibidiShaping,
			ComplexShaping_noRTL = HarfbuzzShaping,
			ComplexShaping_RTL = MinibidiReordering | HarfbuzzShaping
		};
	};

	typedef std::vector<LayoutData> LayoutDataString;

	class LayoutEngine
	{
	public:
		LayoutEngine(FaceCollection* collection);

		const LayoutDataString& Process(utf32string& text, int start, int end, u16vec2 dpi, const Font& font);

		void SetMode(LayoutMode::Enum mode);
	private:
		void ShapeFragment(utf32string& text, FaceID faceId, u16vec2 dpi, const Font& font);
		void FlushShapedData(const std::vector<LayoutData>& data);

		FaceCollection*         m_faceCollection;
		LayoutDataString        m_shapedData;
		LayoutDataString        m_fragmentshapedData;
		utf32string             m_fragment;

		hb_buffer_t*			m_HBbuf;

		LayoutMode::Enum        m_mode;
	};
}
