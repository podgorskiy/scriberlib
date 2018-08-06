#include "TextRenderer.h"
#include "Utils.h"

#include <algorithm>

using namespace Scriber;

enum
{
	k_initialBufferSize = 0x1000
};

TextRenderer::TextRenderer(IRenderAPIPtr renderAPI)
	: m_maxVertexVufferSize(0)
	, m_vertexBuffer(nullptr)
	, m_indexBuffer(nullptr)
	, m_vertexIterator(0)
	, m_indexIterator(0)
	, m_renderAPI(renderAPI)
{
	GrowBuffers(k_initialBufferSize);
}

void TextRenderer::GrowBuffers(uint32_t size)
{
	if (m_maxVertexVufferSize < int(size))
	{
		m_maxVertexVufferSize = NextPowerOf2(size);

		m_vertexBuffer = static_cast<Vertex*>(realloc(m_vertexBuffer, m_maxVertexVufferSize * sizeof(Vertex)));
		m_indexBuffer = static_cast<uint16_t*>(realloc(m_indexBuffer, m_maxVertexVufferSize * 6 / 4 * sizeof(uint16_t)));

		int vertexIterator = 0;
		int indexIterator = 0;

		for (int i = 0; i < m_maxVertexVufferSize / 4; ++i)
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
		glyphPosition.x += round(float(glyph.m_metrics.horiAdvance));
		textMaxWidth = std::max(glyphPosition.x, textMaxWidth);
	}

	textMaxWidth -= position.x;
	highestPoint -= position.y;
	lowestPoint -= position.y;

	/*
	if ((alignment & TextEngine::Alignment::RIGHT) != 0)
	{
		float delta = textMaxWidth * 2.0f / m_viewportSize.x;
		for (int i = startVertex; i != m_vertexCacheIt; ++i)
		{
			m_vertexCache[i].p.x -= delta;
		}
	}
	else if ((alignment & TextEngine::Alignment::HCENTER) != 0)
	{
		float delta = textMaxWidth * 2.0f / m_viewportSize.x / 2.0f;
		for (int i = startVertex; i != m_vertexCacheIt; ++i)
		{
			m_vertexCache[i].p.x -= delta;
		}
	}
	if ((alignment & TextEngine::Alignment::TOP) != 0)
	{
		float delta = highestPoint * 2.0f / m_viewportSize.y;
		for (int i = startVertex; i != m_vertexCacheIt; ++i)
		{
			m_vertexCache[i].p.y += delta;
		}
	}
	else if ((alignment & TextEngine::Alignment::VCENTER) != 0)
	{
		float delta = -(highestPoint + lowestPoint) * 2.0f / m_viewportSize.y / 2.0f;
		for (int i = startVertex; i != m_vertexCacheIt; ++i)
		{
			m_vertexCache[i].p.y -= delta;
		}
	}
	*/
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

	m_vertexBuffer[m_vertexIterator + 0] = v0;
	m_vertexBuffer[m_vertexIterator + 1] = v1;
	m_vertexBuffer[m_vertexIterator + 2] = v2;
	m_vertexBuffer[m_vertexIterator + 3] = v3;
	m_indexIterator += 6;
	m_vertexIterator += 4;
}

void TextRenderer::CommitStashed()
{
	m_renderAPI->Render(m_vertexBuffer, m_indexBuffer, m_indexIterator / 3);

	m_indexIterator = 0;
	m_vertexIterator = 0;
	/*
	if (shader->u_texture.Valid())
	{
		shader->u_texture.SetValue(0);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	{
		glEnableVertexAttribArray((int)shader->positionAttribute);
		glVertexAttribPointer((int)shader->positionAttribute, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), m_vertexCache);
		glEnableVertexAttribArray((int)shader->uvAttribute);
		glVertexAttribPointer((int)shader->uvAttribute, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<float*>(m_vertexCache) + 2);
		glEnableVertexAttribArray((int)shader->colorAttribute);
		glVertexAttribPointer((int)shader->colorAttribute, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(Vertex), reinterpret_cast<float*>(m_vertexCache) + 4);
		glDrawElements(GL_TRIANGLES, m_indexCacheIt, GL_UNSIGNED_SHORT, m_indexCache);
		glDisableVertexAttribArray((int)shader->positionAttribute);
		glDisableVertexAttribArray((int)shader->uvAttribute);
		glDisableVertexAttribArray((int)shader->colorAttribute);
	}

	m_vertexCacheIt = 0;
	m_indexCacheIt = 0;
	*/
}
