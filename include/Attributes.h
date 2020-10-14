#pragma once
#include <stdint.h>

namespace Scriber
{
	struct FontStyle
	{
		enum Enum : uint8_t
		{
			Regular     = 0,
			Italic      = 1 << 0,
			Bold        = 1 << 1,

			BitFieldSize = Bold << 1,
			BitFieldMask = BitFieldSize -1,
		};
	};

	struct Align
	{
		enum Enum : uint8_t
		{
			None     = 0,
			
			Left     = 1 << 0,
			Right    = 1 << 1,
			HCenter  = 1 << 2,
			
			Top      = 1 << 3,
			Bottom   = 1 << 4,
			VCenter  = 1 << 5,

			BitFieldSize = VCenter << 1,
			BitFieldMask = BitFieldSize - 1,
		};
	};

	inline Align::Enum operator|(Align::Enum a, Align::Enum b)
	{
		return static_cast<Align::Enum>(static_cast<int>(a) | static_cast<int>(b));
	}

	inline FontStyle::Enum operator|(FontStyle::Enum a, FontStyle::Enum b)
	{
		return static_cast<FontStyle::Enum>(static_cast<int>(a) | static_cast<int>(b));
	}

	inline uint8_t Aggregate(FontStyle::Enum style, Align::Enum align)
	{
		return (uint8_t(style) & uint8_t(FontStyle::BitFieldMask)) | ((uint8_t(align) & uint8_t(Align::BitFieldMask)) << 2u);
	}

	inline uint8_t Deaggregate(uint8_t a, FontStyle::Enum& style, Align::Enum& align)
	{
		style = FontStyle::Enum(a & uint8_t(FontStyle::BitFieldMask));
		align = Align::Enum((a >> 2u) & uint8_t(Align::BitFieldMask));
	}
}
