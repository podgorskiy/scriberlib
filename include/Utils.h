#pragma once
#include "ForwardDecl.h"
#include "Attributes.h"
#include <string>

namespace Scriber
{
	inline Script GetScript(uint8_t c[4])
	{
		Script result = 0;
		for (int i = 0; i < 4; ++i)
		{
			result = result << 8;
			result += c[i];
		}
		return result;
	}

	inline std::string GetEncoding(uint32_t encoding)
	{
		uint64_t buff = encoding;
		std::string name((const char*)&buff);
		return std::string(name.rbegin(), name.rend());;
	}

	template<typename T, int size>
	inline void InitArray(T(&a)[size], T defaultValue)
	{
		for (int i = 0; i < size; ++i)
		{
			a[i] = defaultValue;
		}
	}

	inline FaceID GetFaceID(TypefaceID tf, FontStyle::Enum style)
	{
		return tf * FontStyle::BitFieldSize | style;
	}

	inline FaceID GetTypefaceID(FaceID f)
	{
		return f / FontStyle::BitFieldSize;
	}

	inline FaceID GetFontStyle(FaceID f)
	{
		return f & FontStyle::BitFieldMask;
	}

	struct F26p6
	{
		F26p6() = default;
		F26p6(F26p6 const& v) = default;

		explicit F26p6(float f) : v(static_cast<int>(round(f * 64.0f))) {};
		explicit F26p6(int i) : v(i * 0x40) {};
		
		explicit operator float() const
		{
			return static_cast<float>(v) / 64.0f;
		}
		explicit operator int() const
		{
			return (v + 0x20) / 0x40;
		}
		int v;
	};

	inline F26p6 F26p6V(int v)
	{
		F26p6 x;
		x.v = v;
		return x;
	}

	inline F26p6 operator+(const F26p6& v1, const F26p6& v2)
	{
		F26p6 r;
		r.v = v1.v + v2.v;
		return r;
	}

	inline F26p6 operator-(const F26p6& v1, const F26p6& v2)
	{
		F26p6 r;
		r.v = v1.v - v2.v;
		return r;
	}

	inline F26p6 operator*(const F26p6& v1, const F26p6& v2)
	{
		F26p6 r;
		r.v = (v1.v * v2.v) / 0x40;
		return r;
	}

	struct F16p16
	{
		F16p16() = default;
		F16p16(F16p16 const& v) = default;

		explicit F16p16(float f) : v(static_cast<int>(round(f * 65536.0f))) {};
		explicit F16p16(int i) : v(i * 0x10000) {};

		operator float() const
		{
			return static_cast<float>(v) / 65536.0f;
		}

		int v;
	};

	inline F16p16 operator+(const F16p16& v1, const F16p16& v2)
	{
		return F16p16(v1.v + v2.v);
	}

	inline F16p16 operator-(const F16p16& v1, const F16p16& v2)
	{
		return F16p16(v1.v - v2.v);
	}

	template<typename T>
	struct vec2_t
	{
		typedef T value_type;
		union
		{
			struct { T x, y; };
			struct { T u, v; };

			T data[2];
		};

		T& operator[](int i)
		{
			return (&x)[i];
		}

		const T& operator[](int i) const
		{
			return (&x)[i];
		}

		vec2_t() = default;
		vec2_t(vec2_t const& v) = default;

		template<typename A>
		explicit vec2_t(vec2_t<A> v) : x(v.x), y(v.y) {};

		explicit vec2_t(T scalar) : x(scalar), y(scalar) {};
		vec2_t(T x, T y) : x(x), y(y) {};
	};

	template<typename T>
	T dot(const vec2_t<T>& v1, const vec2_t<T>& v2)
	{
		return v1.x * v2.x + v1.y * v2.y;
	}

	template<typename T>
	T cross(const vec2_t<T>& v1, const vec2_t<T>& v2)
	{
		return v1.x * v2.y - v1.y * v2.x;
	}

	template<typename T>
	inline vec2_t<T> operator+(const vec2_t<T>& v)
	{
		return v;
	}

	template<typename T>
	inline vec2_t<T> operator-(const vec2_t<T>& v)
	{
		return vec2_t<T>(-v.x, -v.y);
	}

	template<typename T>
	inline vec2_t<T> operator+(const vec2_t<T>& v, T scalar)
	{
		return vec2_t<T>(v.x + scalar, v.y + scalar);
	}

	template<typename T>
	inline vec2_t<T>& operator+=(vec2_t<T>& v, T scalar)
	{
		v = v + scalar;
		return v;
	}

	template<typename T>
	inline vec2_t<T> operator+(T scalar, const vec2_t<T>& v)
	{
		return v + scalar;
	}

	template<typename T>
	inline vec2_t<T> operator+(const vec2_t<T>& v1, const vec2_t<T>& v2)
	{
		return vec2_t<T>(v1.x + v2.x, v1.y + v2.y);
	}

	template<typename T>
	inline vec2_t<T>& operator+=(vec2_t<T>& v1, const vec2_t<T>& v2)
	{
		v1 = v1 + v2;
		return v1;
	}
	
	template<typename T>
	inline vec2_t<T> operator-(const vec2_t<T>& v, T scalar)
	{
		return vec2_t<T>(v.x - scalar, v.y - scalar);
	}

	template<typename T>
	inline vec2_t<T> operator-(T scalar, const vec2_t<T>& v)
	{
		return vec2_t<T>(scalar - v.x, scalar - v.y);
	}

	template<typename T>
	inline vec2_t<T>& operator-=(vec2_t<T>& v, T scalar)
	{
		v = v - scalar;
		return v;
	}

	template<typename T>
	inline vec2_t<T> operator-(const vec2_t<T>& v1, const vec2_t<T>& v2)
	{
		return vec2_t<T>(v1.x - v2.x, v1.y - v2.y);
	}

	template<typename T>
	inline vec2_t<T>& operator-=(vec2_t<T>& v1, const vec2_t<T>& v2)
	{
		v1 = v1 - v2;
		return v1;
	}

	template<typename T>
	inline vec2_t<T> operator*(const vec2_t<T>& v, T scalar)
	{
		return vec2_t<T>(v.x * scalar, v.y * scalar);
	}

	template<typename T>
	inline vec2_t<T> operator*(T scalar, const vec2_t<T>& v)
	{
		return v * scalar;
	}

	template<typename T>
	inline vec2_t<T>& operator*=(vec2_t<T>& v, T scalar)
	{
		v = v * scalar;
		return v;
	}

	template<typename T>
	inline vec2_t<T>& operator*=(vec2_t<T>& v1, const vec2_t<T>& v2)
	{
		v1 = v1 * v2;
		return v1;
	}

	template<typename T>
	inline vec2_t<T> operator*(const vec2_t<T>& v1, const vec2_t<T>& v2)
	{
		return vec2_t<T>(v1.x * v2.x, v1.y * v2.y);
	}

	template<typename T>
	inline vec2_t<T> operator/(const vec2_t<T>& v, T scalar)
	{
		return vec2_t<T>(v.x / scalar, v.y / scalar);
	}

	template<typename T>
	inline vec2_t<T> operator/(T scalar, const vec2_t<T>& v)
	{
		return vec2_t<T>(scalar / v.x, scalar / v.y);
	}

	template<typename T>
	inline vec2_t<T> operator/(const vec2_t<T>& v1, const vec2_t<T>& v2)
	{
		return vec2_t<T>(v1.x / v2.x, v1.y / v2.y);
	}

	template<typename T>
	inline vec2_t<T>& operator/=(vec2_t<T>& v, T scalar)
	{
		v = v / scalar;
		return v;
	}

	template<typename T>
	inline vec2_t<T>& operator/=(vec2_t<T>& v1, const vec2_t<T>& v2)
	{
		v1 = v1 / v2;
		return v1;
	}

	template<typename T>
	inline bool operator==(vec2_t<T>& v1, const vec2_t<T>& v2)
	{
		return v1.x == v2.x && v1.y == v2.y;
	}

	template<typename T>
	inline vec2_t<bool> lessThanEqual(vec2_t<T>& v1, const vec2_t<T>& v2)
	{
		return vec2_t<bool>(v1.x <= v2.x, v1.y <= v2.y);
	}

	template<typename T>
	inline vec2_t<bool> greaterThanEqual(vec2_t<T>& v1, const vec2_t<T>& v2)
	{
		return vec2_t<bool>(v1.x >= v2.x, v1.y >= v2.y);
	}

	template<typename T>
	inline vec2_t<bool> lessThan(vec2_t<T>& v1, const vec2_t<T>& v2)
	{
		return vec2_t<bool>(v1.x < v2.x, v1.y < v2.y);
	}

	template<typename T>
	inline vec2_t<bool> greaterThan(vec2_t<T>& v1, const vec2_t<T>& v2)
	{
		return vec2_t<bool>(v1.x > v2.x, v1.y > v2.y);
	}

	inline bool all(vec2_t<bool>& v1)
	{
		return v1.x && v1.y;
	}

	inline bool any(vec2_t<bool>& v1)
	{
		return v1.x || v1.y;
	}
	
	typedef vec2_t<int> ivec2;
	typedef vec2_t<float> vec2;

	typedef vec2_t<int16_t> i16vec2;
	typedef vec2_t<uint16_t> u16vec2;

	typedef vec2_t<int8_t> i8vec2;
	typedef vec2_t<uint8_t> u8vec2;

	typedef vec2_t<bool> bvec2;

	typedef vec2_t<F26p6> F26p6vec2;

	inline int toPixel(int v, uint16_t dpi)
	{
		v *= dpi;
		v /= 72;
		return v;
	}

	inline ivec2 toPixel(ivec2 v, u16vec2 dpi)
	{
		v *= ivec2(dpi);
		v /= 72;
		return v;
	}

	inline uint16_t NextPowerOf2(uint16_t v)
	{
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v++;
		return v;
	}

	inline uint32_t NextPowerOf2(uint32_t v)
	{
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v++;
		return v;
	}

	inline uint64_t NextPowerOf2(uint64_t v)
	{
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v |= v >> 32;
		v++;
		return v;
	}

	template <typename X>
	inline X* MemoryAlign(X* p, int bitCount)
	{
		uintptr_t x = reinterpret_cast<uintptr_t>(p);
		uintptr_t mask = bitCount - 1;
		return reinterpret_cast<X*>(~mask & (x + mask));
	}

	template <typename X>
	inline X MemoryAlign(X x, int bitCount)
	{
		X mask = bitCount - 1;
		return (~mask & (x + mask));
	}
}
