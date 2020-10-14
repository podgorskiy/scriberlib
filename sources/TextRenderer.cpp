#include "TextRenderer.h"
#include "Utils.h"

#include <algorithm>

using namespace Scriber;

enum
{
	k_initialBufferSize = 0x1000
};

TextRenderer::TextRenderer(IRenderAPIPtr renderAPI)
	: m_maxVertexBufferSize(0)
	, m_vertexBuffer(nullptr)
	, m_indexBuffer(nullptr)
	, m_vertexIterator(0)
	, m_indexIterator(0)
	, m_renderAPI(std::move(renderAPI))
{
	GrowBuffers(k_initialBufferSize);
}

void TextRenderer::GrowBuffers(uint32_t size)
{
	if (m_maxVertexBufferSize < int(size))
	{
		m_maxVertexBufferSize = NextPowerOf2(size);

		m_vertexBuffer = static_cast<Vertex*>(realloc(m_vertexBuffer, m_maxVertexBufferSize * sizeof(Vertex)));
		m_indexBuffer = static_cast<uint16_t*>(realloc(m_indexBuffer, m_maxVertexBufferSize * 6 / 4 * sizeof(uint16_t)));

		int vertexIterator = 0;
		int indexIterator = 0;

		for (int i = 0; i < m_maxVertexBufferSize / 4; ++i)
		{
			m_indexBuffer[indexIterator + 0] = vertexIterator + 0;
			m_indexBuffer[indexIterator + 1] = vertexIterator + 1;
			m_indexBuffer[indexIterator + 2] = vertexIterator + 2;
			m_indexBuffer[indexIterator + 3] = vertexIterator + 1;
			m_indexBuffer[indexIterator + 4] = vertexIterator + 3;
			m_indexBuffer[indexIterator + 5] = vertexIterator + 2;
			indexIterator += 6;
			vertexIterator += 4;
		}
	}
}

TextRenderer::~TextRenderer()
{
	free(m_vertexBuffer);
	free(m_indexBuffer);
	m_vertexBuffer = nullptr;
	m_indexBuffer = nullptr;
}

void TextRenderer::SumbitGlyphString(const GlyphString& glyphString, const ivec2& _position, u16vec2 dpi, const Font& font, Align::Enum alignment)
{
	ivec2 position = toPixel(_position, dpi);
	int fontHeight = toPixel(font.height, dpi.y);

	ivec2 glyphPosition = position;

	int highestPoint = position.y;
	int lowestPoint = 0;
	int textMaxWidth = 0;

	int startVertex = m_vertexIterator;

	for (int i = 0, l = glyphString.size(); i != l; ++i)
	{
		const Glyph& glyph = glyphString[i];

		if (glyph.m_code == '\n')
		{
			glyphPosition.x = position.x;
			glyphPosition.y += fontHeight;
			continue;
		}

		highestPoint = std::max(glyphPosition.y - int(glyph.m_metrics.ascender), highestPoint);
		lowestPoint = std::max(glyphPosition.y - int(glyph.m_metrics.descender), lowestPoint);

		ivec2 bitmapPos = glyphPosition + ivec2(glyph.m_metrics.horizontalBearing.x, -glyph.m_metrics.horizontalBearing.y);

		SubmitGlyph(bitmapPos, glyph);
		glyphPosition.x += int(float(glyph.m_metrics.horiAdvance) + 0.5f);
		textMaxWidth = std::max(glyphPosition.x, textMaxWidth);
	}

	textMaxWidth -= position.x;
	highestPoint -= position.y;
	lowestPoint -= position.y;

	if ((alignment & Align::Right) != 0)
	{
		int delta = textMaxWidth;
		for (int i = startVertex; i != m_vertexIterator; ++i)
		{
			m_vertexBuffer[i].pos.x -= delta;
		}
	}
	else if ((alignment & Align::HCenter) != 0)
	{
		int delta = textMaxWidth / 2;
		for (int i = startVertex; i != m_vertexIterator; ++i)
		{
			m_vertexBuffer[i].pos.x -= delta;
		}
	}
	if ((alignment & Align::Top) != 0)
	{
		int delta = highestPoint;
		for (int i = startVertex; i != m_vertexIterator; ++i)
		{
			m_vertexBuffer[i].pos.y += delta;
		}
	}
	else if ((alignment & Align::VCenter) != 0)
	{
		int delta = -(highestPoint + lowestPoint) / 2;
		for (int i = startVertex; i != m_vertexIterator; ++i)
		{
			m_vertexBuffer[i].pos.y -= delta;
		}
	}
}

void TextRenderer::SubmitGlyph(const ivec2& position, const Glyph& glyph)
{
	//glyph.m_metrics.glyphSize, glyph.m_cacheUV, glyph.m_cacheUV + glyph.m_metrics.glyphSize, ge.r, ge.g, ge.b, ge.a;

	Vertex vdefault;
	vdefault.color = glyph.m_color;
	Vertex v0(vdefault), v1(vdefault), v2(vdefault), v3(vdefault);

	v0.pos = i16vec2(position);
	v3.pos = i16vec2(position) + i16vec2(glyph.m_metrics.glyphSize);
	v1.pos.y = v0.pos.y;
	v2.pos.x = v0.pos.x;
	v1.pos.x = v3.pos.x;
	v2.pos.y = v3.pos.y;

	v0.uv = glyph.m_cacheUV;
	v3.uv = glyph.m_cacheUV + glyph.m_metrics.glyphSize;
	v1.uv.y = v0.uv.y;
	v2.uv.x = v0.uv.x;
	v1.uv.x = v3.uv.x;
	v2.uv.y = v3.uv.y;

	GrowBuffers(m_vertexIterator + 4);

	m_vertexBuffer[m_vertexIterator + 0] = v0;
	m_vertexBuffer[m_vertexIterator + 1] = v1;
	m_vertexBuffer[m_vertexIterator + 2] = v2;
	m_vertexBuffer[m_vertexIterator + 3] = v3;
	m_indexIterator += 6;
	m_vertexIterator += 4;
}

void TextRenderer::CommitStashed()
{
	m_renderAPI->Render(m_vertexBuffer, m_indexBuffer, m_vertexIterator, m_indexIterator / 3);

	m_indexIterator = 0;
	m_vertexIterator = 0;
}
