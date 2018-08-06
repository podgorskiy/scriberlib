#pragma once
#include "ForwardDecl.h"
#include "Attributes.h"
#include <vector>
#include <map>

namespace Scriber
{
	class FaceCollection
	{
	public:
		FaceCollection(FT_Library lib);

		TypefaceID NewTypeface(const char* name, int priority);

		TypefaceID GetTypefaceByName(const char* name);

		void AndFontToTypeface(TypefaceID tf, const char* filename, FontStyle::Enum style, int faceIndexInFile = 0);

		FaceID GetFaceIDFromCode(uint32_t code, TypefaceID prefferedTypeface, FontStyle::Enum prefferedStyle) const;

		bool HasFaceIDCode(uint32_t code, FaceID faceID) const;

		FT_Face GetFace(FaceID id) const;

		hb_font_t* GetHBFontByFaceId(FaceID id);
	private:
		struct Typeface
		{
			Typeface();

			FT_Face m_faces[FontStyle::BitFieldSize];
			hb_font_t* m_HBfonts[FontStyle::BitFieldSize];
			Script  m_script;
			int priority;
		};

		struct Key
		{
			uint32_t code;
			TypefaceID prefferedTypeface;
			FontStyle::Enum style;

			bool operator<(const Key& x) const
			{
				if (code < x.code)  return true;
				if (code > x.code)  return false;
				if (prefferedTypeface < x.prefferedTypeface)  return true;
				if (prefferedTypeface > x.prefferedTypeface)  return false;
				if (style < x.style)  return true;
				if (style > x.style)  return false;
				return false;
			};

			bool operator==(const Key& x)
			{
				return !(*this < x || x < *this );
			}
		};

		mutable std::map<Key, FaceID> m_code2FaceID;

		std::vector<Typeface> m_typefaces;
		std::map<std::string, TypefaceID> m_typefaceNames;
		std::vector<TypefaceID> m_typefacesOrder;

		FT_Library m_lib;
	};
}
