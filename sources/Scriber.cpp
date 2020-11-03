#include "Scriber.h"
#include "FaceCollection.h"
#include "Layout.h"
#include "GlyphBitmapStash.h"
#include "StringStash.h"
#include "StringFormater.h"
#include "TextRenderer.h"
#include "IRenderAPI.h"

#include <freetype.h>

#include <vector>
#include <map>

using namespace Scriber;

static fopenfunc s_open;
static fclosefunc s_close;
static freadfunc s_read;
static fseekfunc s_seek;
static ftellfunc s_tell;

namespace Scriber
{
	struct FTLib
	{
		FTLib()
		{
			FT_Init_FreeType(&lib);
		}

		~FTLib()
		{
			FT_Done_FreeType(lib);
		}

		FTLib(FTLib const&) = delete;
		FTLib& operator=(FTLib const&) = delete;

		FT_Library lib;
	};
	
	namespace detail
	{
		struct DriverImpl
		{
			DriverImpl()
				: faceCollection(lib.lib)
				, layoutEngine(&faceCollection)
				, renderAPI(new SoftwareRenderAPI)
				, glyphBitmapStash(lib.lib, &faceCollection, renderAPI)
				, stringFormater(&layoutEngine, &glyphBitmapStash)
				, textRenderer(renderAPI)
				, m_dpi(72)
			{
				using namespace std::placeholders;
				stringStash.AssignStringProcessor(std::bind(&StringFormater::Format, &stringFormater, _1, _2, _3, _4));
			};

			explicit DriverImpl(IRenderAPIPtr renderer)
				: faceCollection(lib.lib)
				, layoutEngine(&faceCollection)
				, renderAPI(std::move(renderer))
				, glyphBitmapStash(lib.lib, &faceCollection, renderAPI)
				, stringFormater(&layoutEngine, &glyphBitmapStash)
				, textRenderer(renderAPI)
				, m_dpi(72)
			{
				using namespace std::placeholders;
				stringStash.AssignStringProcessor(std::bind(&StringFormater::Format, &stringFormater, _1, _2, _3, _4));
			};

			DriverImpl(DriverImpl const&) = delete;
			DriverImpl& operator=(DriverImpl const&) = delete;

			FTLib            lib;
			FaceCollection   faceCollection;
			LayoutEngine     layoutEngine;
			IRenderAPIPtr    renderAPI;
			GlyphBitmapStash glyphBitmapStash;
			StringStash      stringStash;
			StringFormater   stringFormater;
			TextRenderer     textRenderer;
			/*
			LayoutEngine     textShaper;
			FontManager      fontManager;
			GlyphCache       glyphCache;
			TextPipeline     textPipeline;
			FontRenderer     fontRenderer;
			FaceProvider     faceProvider;
			*/

			u16vec2 m_dpi;
		};
	}
}

Driver::Driver()
{
	m_impl.reset(new detail::DriverImpl());
	ResetIOFunctions();
}

TypefaceID Driver::NewTypeface(const char* name, int priority)
{
	return m_impl->faceCollection.NewTypeface(name, priority);
}

TypefaceID Driver::GetTypefaceByName(const char* name) const
{
	return m_impl->faceCollection.GetTypefaceByName(name);
}

void Driver::AndFontToTypeface(TypefaceID tf, const char* filename, FontStyle::Enum style, int faceIndexInFile)
{
	return m_impl->faceCollection.AndFontToTypeface(tf, filename, style, faceIndexInFile);
}

void Driver::ResetIOFunctions()
{
	s_open = nullptr;
	s_close = nullptr;
	s_read = nullptr;
	s_seek = nullptr;
	s_tell = nullptr;
	ft_set_file_callback(
	[](const char* filename, const char* mode)
	{
		return fopen(filename, mode);
	},
	[](FILE* userfile)
	{
		return fclose(userfile);
	},
	[](void* ptr, size_t size, size_t count, FILE* file)
	{
		return fread(ptr, size, count, file);
	},
	[](FILE* userfile, long int offset, int origin)
	{
		return fseek(userfile, offset, origin);
	},
	[](FILE* userfile)
	{
		return ftell(userfile);
	});
}

void Driver::SetCustomIOFunctions(fopenfunc open, fclosefunc close, freadfunc read, fseekfunc seek, ftellfunc tell)
{
	s_open = open;
	s_close = close;
	s_read = read;
	s_seek = seek;
	s_tell = tell;
	ft_set_file_callback(
	[](const char* filename, const char* mode)
	{
		return (FILE*)(s_open(filename, mode));
	},
	[](FILE* userfile)
	{
		return s_close((UserFile*)userfile);
	},
	[](void* ptr, size_t size, size_t count, FILE* file)
	{
		return s_read(ptr, size, count, (UserFile*)file);
	},
	[](FILE* file, long int offset, int origin)
	{
		return s_seek((UserFile*)file, offset, origin);
	},
	[](FILE* file)
	{
		return s_tell((UserFile*)file);
	});
}

void Driver::SetDPI(uint16_t x, uint16_t y)
{
	m_impl->m_dpi = u16vec2(x, y);
}

void Driver::DrawLabel(const char* text, int position_x, int position_y, const Font& font, Align::Enum alignment, float true_hight)
{
	const GlyphString* glyphString = &m_impl->stringStash.GetGlyphString(text, m_impl->m_dpi, font);
	if (m_impl->glyphBitmapStash.CheckIfOverflowedAndResetFlag())
	{
		m_impl->stringStash.Purge();
		glyphString = &m_impl->stringStash.GetGlyphString(text, m_impl->m_dpi, font);
	}
	m_impl->textRenderer.SumbitGlyphString(*glyphString, ivec2(position_x, position_y), m_impl->m_dpi, font, alignment, true_hight);
}

void Driver::CleanStash()
{
	m_impl->stringStash.Purge();
	m_impl->glyphBitmapStash.Purge();
}

void Driver::Render()
{
	m_impl->textRenderer.CommitStashed();
	/*
	m_glyphCache->GetTexture()->Bind(0);
	m_fontRenderer->DrawCache(m_textShader);
	m_glyphCache->GetTexture()->UnBind();
	*/
}

void Driver::SetBackend(IRenderAPIPtr renderAPI)
{
	m_impl.reset(new detail::DriverImpl(std::move(renderAPI)));
}

/*
void TextEngine::Draw(const std::string& string, const glm::ivec4& color, int fontStyleId, const glm::vec2& position, Alignment alignment)
{
}

void TextEngine::Flush()
{
}

int TextEngine::GetFontID(const std::string& fontName)
{
	return m_fontManager->GetFontID(fontName);
}
*/
