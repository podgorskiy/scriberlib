#include "Scriber.h"
#include "FaceCollection.h"
#include "Utils.h"
#include "Layout.h"

#define USE_HARFBUZZ
#ifdef USE_HARFBUZZ

#include <hb.h>
#include <hb-ft.h>

#endif

#include <freetype.h>
#include <algorithm>

using namespace Scriber;

FaceCollection::FaceCollection(FT_Library lib): m_lib(lib)
{
}

TypefaceID FaceCollection::NewTypeface(const char* name, int priority)
{
	TypefaceID id = m_typefaces.size();
	m_typefaces.push_back(Typeface());
	m_typefaceNames[name] = id;
	
	m_typefacesOrder.resize(m_typefaces.size());

	int n = 0;
	std::generate(m_typefacesOrder.begin(), m_typefacesOrder.end(), [&n] { return n++; });

	const auto& typefaces = m_typefaces;
	std::sort(m_typefacesOrder.begin(), m_typefacesOrder.end(), [&typefaces](TypefaceID a, const TypefaceID b)
	{
		return typefaces[a].priority < typefaces[b].priority;
	});

	return id;
}

TypefaceID FaceCollection::GetTypefaceByName(const char* name)
{
	auto it = m_typefaceNames.find(name);
	if (it != m_typefaceNames.end())
	{
		return it->second;
	}
	return InvalidTypefaceID;
}


static int ForceUCS2(FT_Face ftf)
{
	for (int i = 0; i < ftf->num_charmaps; i++)
	{
		if (((ftf->charmaps[i]->platform_id == 0) && (ftf->charmaps[i]->encoding_id == 3)) || ((ftf->charmaps[i]->platform_id == 3) && (ftf->charmaps[i]->encoding_id == 1)))
		{
			return FT_Set_Charmap(ftf, ftf->charmaps[i]);
		}
	}
	return -1;
}

void FaceCollection::AndFontToTypeface(TypefaceID tf, const char* filename, FontStyle::Enum style, int faceIndexInFile)
{
	FT_Face face = nullptr;
	FT_Error result = FT_New_Face(m_lib, filename, faceIndexInFile, &face);
	ForceUCS2(face);
	if (face != nullptr && result == FT_Err_Ok)
	{
		printf("\n\nCharmap family: %s\n", face->family_name);
		printf("Charmap num: %d\n", face->num_charmaps);
		for (int i = 0; i < face->num_charmaps; ++i)
		{
			uint64_t e = face->charmaps[i]->encoding;
			std::string name = GetEncoding(e);
			printf("%s\n", name.c_str());
		}

		m_typefaces[tf].m_faces[style] = face;
		m_typefaces[tf].m_HBfonts[style] = hb_ft_font_create(face, NULL);
	}
}

FaceCollection::Typeface::Typeface()
{
	InitArray(m_faces, FT_Face(nullptr));
}

bool FaceCollection::HasFaceIDCode(uint32_t code, FaceID faceID) const
{
	FT_UInt glyphIndex = FT_Get_Char_Index(GetFace(faceID), code);
	return glyphIndex != 0;
}

FaceID FaceCollection::GetFaceIDFromCode(uint32_t code, TypefaceID prefferedTypeface, FontStyle::Enum prefferedStyle) const
{
	Key key = { code, prefferedTypeface , prefferedStyle };

	std::map<Key, FaceID>::iterator lb = m_code2FaceID.lower_bound(key);

	if (lb != m_code2FaceID.end() && (key == lb->first))
	{
		return lb->second;
	}
	else
	{
		if (m_typefaces.size() == 0)
		{
			return InvalidFaceID;
		}

		if (prefferedTypeface > m_typefaces.size())
		{
			prefferedTypeface = m_typefacesOrder[0];
		}

		TypefaceID currentTypeface = prefferedTypeface;

		{
			FontStyle::Enum style = prefferedStyle;
			FT_Face face;

			const auto& typeface = m_typefaces[currentTypeface];

			if (typeface.m_faces[prefferedStyle] == nullptr)
			{
				style = FontStyle::Regular;
			}

			face = typeface.m_faces[style];

			if (face != nullptr)
			{
				FT_UInt glyphIndex = FT_Get_Char_Index(face, code);
				if (glyphIndex != 0)
				{
					FaceID id = GetFaceID(currentTypeface, style);
					m_code2FaceID.insert(lb, std::map<Key, FaceID>::value_type(key, id));
					return id;
				}
			}
		}

		int currentIndex = 0;
		while (currentIndex < (int)m_typefacesOrder.size())
		{
			currentTypeface = m_typefacesOrder[currentIndex];
			++currentIndex;
			if (currentTypeface == prefferedTypeface)
			{
				continue;
			}

			FontStyle::Enum style = prefferedStyle;
			FT_Face face;

			const auto& typeface = m_typefaces[currentTypeface];

			if (typeface.m_faces[prefferedStyle] == nullptr)
			{
				style = FontStyle::Regular;
			}

			face = typeface.m_faces[style];

			if (face != nullptr)
			{
				FT_UInt glyphIndex = FT_Get_Char_Index(face, code);
				if (glyphIndex != 0)
				{
					FaceID id = GetFaceID(currentTypeface, style);
					m_code2FaceID.insert(lb, std::map<Key, FaceID>::value_type(key, id));
					return id;
				}
			}
		}
	}

	m_code2FaceID.insert(lb, std::map<Key, FaceID>::value_type(key, InvalidFaceID));
	return InvalidFaceID;
}

FT_Face FaceCollection::GetFace(FaceID id) const
{
	if (id == InvalidFaceID)
	{
		return m_typefaces[0].m_faces[0];
	}
	return m_typefaces[GetTypefaceID(id)].m_faces[GetFontStyle(id)];
}

hb_font_t* FaceCollection::GetHBFontByFaceId(FaceID id)
{
	if (id == InvalidFaceID)
	{
		return m_typefaces[0].m_HBfonts[0];
	}
	return m_typefaces[GetTypefaceID(id)].m_HBfonts[GetFontStyle(id)];
}
