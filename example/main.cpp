#include <Scriber.h>

struct Weight
{
	enum Enum: uint8_t
	{
		None       = 0,
		Thin       = 1, // 100
		ExtraLight = 2, // 200 Ultralight
		Light      = 3, // 300
		Regular    = 4, // 400 Book, Normal, Roman
		Medium     = 5, // 500
		Semibold   = 6, // 600 Demibold
		Bold       = 7, // 700
		ExtraBold  = 8, // 800 Ultrabold
		Black      = 9, // 900 Heavy

		Count
	};

	static const char WeightChar[];
	static const char* WeightStrings[Count][7];
};

struct Width
{
	enum Enum: uint8_t
	{
		None           = 0, // 50%
		UltraCondensed = 1, // 50% UltraCompressed
		ExtraCondensed = 2, // 62.5% ExtraCompressed
		Condensed      = 3, // 75% Cond Compressed Compact
		Semicondensed  = 4, // 87.5% Narrow
		Normal         = 5, // 100% Normal Medium Regular(usually omitted)
		Semiexpanded   = 6, // 112.5%
		Expanded       = 7, // 125% Extended Elongated
		ExtraExpanded  = 8, // 150%
		UltraExpanded  = 9, // 200% Wide

		Count
	};

	static const char WidthChar[];
	static const char* WidthStrings[Count][7];
};

const char Weight::WeightChar[] =
{
	'\0', // None
	'a',  // 100
	'j',  // 200
	'l',  // 300
	'r',  // 400
	'm',  // 500
	's',  // 600
	'b',  // 700
	'x',  // 800
	'c',  // 900
};

const char* Weight::WeightStrings[Count][7] = 
{
	{ nullptr },
	{ "Thin", "Hairline", nullptr },
	{ "ExtraLight", "UltraLight", nullptr },
	{ "Light", nullptr },
	{ "Regular", "Book", "Normal", "Roman", "Semilight", "Plain", nullptr },
	{ "Medium", nullptr },
	{ "Semibold", "Demibold", nullptr },
	{ "Bold", nullptr },
	{ "ExtraBold", "UltraBold", nullptr },
	{ "Black", "Heavy", nullptr }
};

const char Width::WidthChar[] =
{
	'\0', // None
	'o',  // UltraCondensed
	'q',  // ExtraCondensed ExtraCompressed
	'c',  // Condensed Cond
	'n',  // Semicondensed Narrow
	'r',  // Normal Medium Regular(usually omitted)
	'y',  // Semiexpanded
	'e',  // Expanded
	'v',  // ExtraExpanded
	'w',  // UltraExpanded Wide
};

const char* Width::WidthStrings[Count][7] =
{
	{ nullptr },
	{ "UltraCondensed", "UltraCompressed", nullptr },
	{ "ExtraCondensed", "ExtraCompressed", nullptr },
	{ "Condensed", "Cond", "Compact", "Compressed", nullptr },
	{ "Semicondensed", "Narrow", nullptr },
	{ "Normal", "Medium", "Regular", nullptr },
	{ "Semiexpanded", nullptr },
	{ "Expanded", "Extended", "Elongated", nullptr },
	{ "ExtraExpanded", nullptr },
	{ "UltraExpanded", "Wide", nullptr }
};


bool AcceptChar(char c, const char*& it)
{
	if (*it == c)
	{
		++it;
		return 1;
	}
	return 0;
}

bool AcceptString(const char* str, const char*& it)
{
	while (*str)
	{
		if (*str++ != *it++)
		{
			return false;
		}
	}

	return !*str;
}

bool AcceptSpace(const char*& it)
{
	return AcceptChar(' ', it) || AcceptChar('\t', it);
}

Weight::Enum AcceptWeight(const char*& it)
{
	while (AcceptSpace(it));
	const char* it_backup = it;

	for (int i = 1; i < Weight::Count; ++i)
	{
		const char** weightName = Weight::WeightStrings[i];
		while (*weightName != 0)
		{
			bool accepted = AcceptString(*weightName, it);
			if (accepted && (*it == 0 || AcceptSpace(it)))
			{
				return Weight::Enum(i);
			}
			it = it_backup;
			++weightName;
		}
	}

	return Weight::None;
}

int main()
{
	//const char* str = "ExtraCondensed Black Italic";
	const char* str = " Black Italic";
	//const char* str = " Blackk Italic";
	Weight::Enum w = AcceptWeight(str);
	
	Scriber::Driver m_driver;

	Scriber::TypefaceID id;

	id = m_driver.NewTypeface("NotoSans");
	m_driver.AndFontToTypeface(id, "../data/NotoSans-ExtraCondensedBlackItalic.ttf", Scriber::FontStyle::Regular);
	id = m_driver.NewTypeface("NotoSans");
	m_driver.AndFontToTypeface(id, "../data/Roboto-Regular.ttf", Scriber::FontStyle::Regular);
	id = m_driver.NewTypeface("NotoSans");
	m_driver.AndFontToTypeface(id, "../data/Roboto-LightItalic.ttf", Scriber::FontStyle::Regular);
	id = m_driver.NewTypeface("NotoSans");
	m_driver.AndFontToTypeface(id, "../data/Roboto-MediumItalic.ttf", Scriber::FontStyle::Regular);
	id = m_driver.NewTypeface("NotoSans");
	m_driver.AndFontToTypeface(id, "../data/NotoSansArabic-Light.ttf", Scriber::FontStyle::Regular);
	id = m_driver.NewTypeface("NotoSans");
	m_driver.AndFontToTypeface(id, "../data/NotoSansArabic-Bold.ttf", Scriber::FontStyle::Regular);
	id = m_driver.NewTypeface("NotoSans");
	m_driver.AndFontToTypeface(id, "../data/NotoSansArabicUI-Light.ttf", Scriber::FontStyle::Regular);
	id = m_driver.NewTypeface("NotoSans");
	m_driver.AndFontToTypeface(id, "../data/NotoSans-Bold.ttf", Scriber::FontStyle::Regular);
	id = m_driver.NewTypeface("NotoSans");
	m_driver.AndFontToTypeface(id, "../data/ltromatic bold.ttf", Scriber::FontStyle::Regular);
	id = m_driver.NewTypeface("NotoSans");
	m_driver.AndFontToTypeface(id, "../data/ltromatic italic.ttf", Scriber::FontStyle::Regular);
	id = m_driver.NewTypeface("NotoSans");
	m_driver.AndFontToTypeface(id, "../data/ltromatic.ttf", Scriber::FontStyle::Regular);
	id = m_driver.NewTypeface("NotoSans");
	m_driver.AndFontToTypeface(id, "../data/NotoSans-Regular.ttf", Scriber::FontStyle::Regular);
	id = m_driver.NewTypeface("NotoSans");
	m_driver.AndFontToTypeface(id, "../data/NotoSans-BoldItalic.ttf", Scriber::FontStyle::Regular);
	id = m_driver.NewTypeface("NotoSans");
	m_driver.AndFontToTypeface(id, "../data/NotoSansArabic-Regular.ttf", Scriber::FontStyle::Regular);
	id = m_driver.NewTypeface("NotoSans");
	m_driver.AndFontToTypeface(id, "../data/NotoSansThai-Regular.ttf", Scriber::FontStyle::Regular);

	m_driver.SetDPI(72, 72);

	//const char *text = "رايد ذي لايتنينغ (بالإنجليزية : Ride ก็๋ป๋ป็ป็๋ Light\nning)و معناها ركوب البرق ،";
	const char *text = "ก็๋ป๋ป็ป็๋ ";
	m_driver.DrawLabel(text, 110, 110, Scriber::Font(0, 84));
	//m_driver.DrawLabel(text, 0, 0, Scriber::Font(0, 42, Scriber::FontStyle::Regular, 0xFFFFFF, 3));
	m_driver.Render();
	return 0;
}
