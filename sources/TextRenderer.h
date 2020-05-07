#pragma once
#include "GlyphBitmapStash.h"
#include "Scriber.h"
#include "Glyph.h"
#include "IRenderAPI.h"

namespace Scriber
{
	class TextRenderer
	{
	public:
		TextRenderer(const TextRenderer& other) = delete;
		TextRenderer& operator=(const TextRenderer&) = delete;

		TextRenderer(IRenderAPIPtr renderAPI);
		~TextRenderer();

		void SumbitGlyphString(const GlyphString& glyphString, const ivec2& position, u16vec2 dpi, const Font& font, Align::Enum alignment);

		void CommitStashed();
	private:
		void SubmitGlyph(const ivec2& position, const Glyph& glyph);
		
		void GrowBuffers(uint32_t size);

		int m_maxVertexBufferSize;
		Vertex* m_vertexBuffer;
		uint16_t* m_indexBuffer;
		uint16_t m_vertexIterator;
		uint16_t m_indexIterator;

		IRenderAPIPtr m_renderAPI;
	};
}
