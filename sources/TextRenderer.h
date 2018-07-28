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
		TextRenderer(IRenderAPIPtr renderAPI);
		~TextRenderer();

		void SumbitGlyphString(const GlyphString& glyphString, const ivec2& position, u16vec2 dpi, const Font& font, Align::Enum alignment);

		void CommitStashed();
	private:
		void SubmitGlyph(const ivec2& position, const Glyph& glyph);
		
		void GrowBuffers(uint32_t size);

		Vertex* m_vertexBuffer;
		uint16_t* m_indexBuffer;
		uint16_t m_indexIterator;
		uint16_t m_vertexIterator;

		int m_maxVertexVufferSize;

		//glm::vec2 m_K;
		//glm::vec2 m_KUV;

		//glm::ivec2 m_viewportSize;
		//glm::ivec2 m_textureSize;

		IRenderAPIPtr m_renderAPI;
	};
}
