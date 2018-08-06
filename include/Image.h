#pragma once
#include "Utils.h"

#include <memory>

namespace Scriber
{
	class Image
	{
	public:
		enum DataType
		{
			R8,
			RG8,
			RGB8
		};
		
		// Generation
		static Image Empty(ivec2 size, DataType d, uint8_t alignment);

		static Image Empty(const Image& sameAs);

		Image Copy() const;

		Image OpenView(const ivec2 pos, const ivec2 size);

		void Clear();

		// IO
		static Image FromUnalignedData(const void* data, DataType d, ivec2 size, uint8_t alignment);

		static Image FromMemory(const void* data, DataType d, ivec2 size, uint8_t alignment);

		void* ToUnalignedData();

		void Assign(Image x);

		void AssignToChannel(Image x, uint8_t channel);

		void AssignToChannelZeroOther(Image x, uint8_t channel);

		void SaveToTGA(const char* filename);

		// Attributes
		DataType GetType() const;

		size_t GetRowSizeAligned() const;

		size_t GetRowSize() const;

		bool IsValid() const;

		static size_t GetBPP(DataType d);

		size_t GetBPP() const;

		ivec2 GetSize() const;

		int GetChannelCount() const;

		int rows() const;

		int cols() const;

		// Elements access
		template<typename T>
		T* GetRow(int j);

		template<typename T>
		const T* GetRow(int j) const;

		template<typename T>
		const T* ptr(int j) const;

		template<typename T>
		T* ptr(int j);

	private:
		std::shared_ptr<uint8_t> _ptr;
		ivec2 size = ivec2(0);
		size_t data_size = 0;
		size_t offset = 0;
		size_t row_size = 0;
		size_t row_stride = 0;
		DataType dataType = R8;
		uint8_t alignment = 4;
	};

	inline ivec2 Image::GetSize() const
	{
		return size;
	}

	inline Image::DataType Image::GetType() const
	{
		return dataType;
	}

	inline size_t Image::GetRowSizeAligned() const
	{
		return row_stride;
	}

	inline size_t Image::GetRowSize() const
	{
		return size.x * GetBPP();
	}

	template<typename T>
	inline T* Image::GetRow(int j)
	{
		return reinterpret_cast<T*>(_ptr.get() + offset + row_stride * j);
	}

	template<typename T>
	inline const T* Image::GetRow(int j) const
	{
		return reinterpret_cast<T*>(_ptr.get() + offset + row_stride * j);
	}

	template<typename T>
	inline T* Image::ptr(int j)
	{
		return reinterpret_cast<T*>(_ptr.get() + offset + row_stride * j);
	}

	template<typename T>
	inline const T* Image::ptr(int j) const
	{
		return reinterpret_cast<T*>(_ptr.get() + offset + row_stride * j);
	}

	inline size_t Image::GetBPP(DataType d)
	{
		switch (d)
		{
		case R8: return 1;
		case RG8: return 2;
		case RGB8: return 3;
		default: return 0;
		}
	}

	inline int Image::GetChannelCount() const
	{
		switch (dataType)
		{
		case R8: return 1;
		case RG8: return 2;
		case RGB8: return 3;
		default: return 0;
		}
	}

	inline size_t Image::GetBPP() const
	{
		return GetBPP(dataType);
	}
}
