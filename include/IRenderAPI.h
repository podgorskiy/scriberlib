#pragma once
#include "Scriber.h"
#include "Utils.h"
#include "Image.h"

#include <memory>

namespace Scriber
{
	struct Vertex
	{
		i16vec2 pos;
		u16vec2 uv;
		union
		{
			struct
			{
				uint8_t r;
				uint8_t g;
				uint8_t b;
				uint8_t a;
			};
			uint32_t color;
		};
	};

	class IRenderAPI
	{
	public:
		~IRenderAPI() {}

		virtual void SaveTextureToFile() = 0;

		virtual void UpdateTexture(Image image, u16vec2 pos) = 0;

		virtual void ClearTexture() = 0;

		virtual void Render(Vertex* vertexBuffer, uint16_t* indexBuffer, uint16_t vertex_count, uint16_t primitiveCount) = 0;

		virtual int GetTextureSize() { return 1024; }

		virtual int GetSpacing() { return 3; }
	};

	typedef std::shared_ptr<IRenderAPI> IRenderAPIPtr;


	class SoftwareRenderAPI: public IRenderAPI
	{
	public:
		SoftwareRenderAPI()
		{
			m_cacheTexture = Image::Empty(ivec2(1024), Image::RG8, 1);
			m_screen = Image::Empty(ivec2(1024), Image::RG8, 1);
		}

		~SoftwareRenderAPI() =default;

		void SaveTextureToFile() override
		{
			m_cacheTexture.SaveToTGA("TestCache.tga");
		}

		void UpdateTexture(Image image, u16vec2 pos) override
		{
			m_cacheTexture.OpenView(ivec2(pos), image.GetSize()).Assign(image);
		}

		void ClearTexture() override
		{
			m_cacheTexture.Clear();
		}

		void Render(Vertex* vertexBuffer, uint16_t* indexBuffer, uint16_t vertex_count, uint16_t primitiveCount) override
		{
			for (int i = 0; i < primitiveCount / 2; ++i)
			{
				i16vec2 pos = vertexBuffer[4 * i + 0].pos;
				i16vec2 size = vertexBuffer[4 * i + 3].pos - pos;

				u16vec2 uv0 = vertexBuffer[4 * i + 0].uv;
				u16vec2 uvSize = vertexBuffer[4 * i + 3].uv - uv0;

				Image glyph = m_cacheTexture.OpenView(ivec2(uv0), ivec2(uvSize));
				m_screen.OpenView(ivec2(pos), ivec2(size)).Assign(glyph);
			}
			m_screen.SaveToTGA("result.tga");
		}
	private:
		Image m_cacheTexture;
		Image m_screen;
	};
}
