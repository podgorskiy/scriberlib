#pragma once
#include <vector>
#include <memory>
#include <stdint.h>
#include <functional>

typedef struct FT_FaceRec_*  FT_Face;
typedef struct FT_LibraryRec_* FT_Library;
typedef struct FT_StrokerRec_*  FT_Stroker;

struct hb_font_t;
struct hb_buffer_t;

namespace Scriber
{
	struct UserFile;

	typedef std::function<UserFile*(const char* filename, const char* mode)> fopenfunc;
	typedef std::function<int(UserFile* userfile)> fclosefunc;
	typedef std::function<size_t(void* ptr, size_t size, size_t count, UserFile* userfile)> freadfunc;
	typedef std::function<int(UserFile* userfile, long int offset, int origin)> fseekfunc;
	typedef std::function<long int(UserFile* userfile)> ftellfunc;

	namespace detail
	{
		struct DriverImpl;
		typedef std::shared_ptr<DriverImpl> DriverImplPtr;

		class TypefaceImpl;
		typedef std::shared_ptr<TypefaceImpl> TypefaceImplPtr;
	}

	typedef uint16_t TypefaceID;
	typedef uint16_t FaceID;
	typedef uint32_t GlyphID;
	typedef uint32_t Script;
	typedef uint16_t UserData;
	typedef int32_t FixedF26;
	typedef int32_t FixedF16;

	typedef std::vector<uint32_t> utf32string;
	
	enum: uint16_t
	{
		InvalidTypefaceID = TypefaceID(-1),
		InvalidFaceID = FaceID(-1),
	};
	enum : uint32_t
	{
		InvalidGlyphID = GlyphID(-1),
	};
};
