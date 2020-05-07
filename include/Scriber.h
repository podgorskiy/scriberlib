#pragma once
#include "ForwardDecl.h"
#include "Attributes.h"

namespace Scriber
{
	struct Font
	{
		Font(TypefaceID tf, uint16_t height, FontStyle::Enum style, uint32_t color, uint16_t stroke, UserData userdata)
			: preferred_tf(tf)
			, height(height)
			, style(style)
			, color(color)
			, stroke(stroke)
			, userdata(userdata) {};

		Font(TypefaceID tf, uint16_t height, FontStyle::Enum style, uint32_t color, uint16_t stroke)
			: preferred_tf(tf)
			, height(height)
			, style(style)
			, color(color)
			, stroke(stroke)
			, userdata(0) {};

		Font(TypefaceID tf, uint16_t height, FontStyle::Enum style, uint32_t color)
			: preferred_tf(tf)
			, height(height)
			, style(style)
			, color(color)
			, stroke(0)
			, userdata(0) {};

		Font(TypefaceID tf, uint16_t height, FontStyle::Enum style)
			: preferred_tf(tf)
			, height(height)
			, style(style)
			, color(0xFFFFFFFF)
			, stroke(0)
			, userdata(0) {};

		Font(TypefaceID tf, uint16_t height)
			: preferred_tf(tf)
			, height(height)
			, style(FontStyle::Regular)
			, color(0xFFFFFFFF)
			, stroke(0)
			, userdata(0) {};

		TypefaceID preferred_tf;
		uint16_t height;
		FontStyle::Enum style;
		uint32_t color;
		uint16_t stroke;
		UserData userdata;
	};

	class IRenderAPI;
	typedef std::shared_ptr<IRenderAPI> IRenderAPIPtr;

	class Driver
	{
	public:
		Driver();

		static void SetCustomIOFunctions(fopenfunc open, fclosefunc close, freadfunc read, fseekfunc seek, ftellfunc tell);

		static void ResetIOFunctions();

		TypefaceID NewTypeface(const char* name, int priority = 1);

		TypefaceID GetTypefaceByName(const char* name) const;

		void AndFontToTypeface(TypefaceID tf, const char* filename, FontStyle::Enum style, int faceIndexInFile = 0);

		//void UnloadTypeface(TypefaceID tf);

		void SetDPI(uint16_t x, uint16_t y);

		void DrawLabel(const char* text, int position_x, int position_y, const Font& font, Align::Enum alignment = Align::Left);

		void CleanStash();

		void Render();

		void SetBackend(IRenderAPIPtr renderer);

	private:
		detail::DriverImplPtr m_impl;
	};
}
