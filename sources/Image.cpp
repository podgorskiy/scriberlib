#include "Image.h"
#include <assert.h>
#include <math.h>
#include <algorithm>
#include <string.h>

using namespace Scriber;


bool Image::IsValid() const
{
	return (data_size != 0) && (MemoryAlign(size.x * GetBPP(), alignment) * size.y == data_size);
}

Image Image::Empty(ivec2 size, Image::DataType d, uint8_t alignment)
{
	Image im;
	im.size = size;
	im.dataType = d;
	im.alignment = alignment;
	im.row_stride = MemoryAlign(im.GetRowSize(), im.alignment);
	im.row_size = im.GetRowSize();
	im.data_size = im.row_stride * (size_t)im.size.y;
	im._ptr.reset(new uint8_t[im.data_size]);
	memset(im._ptr.get(), 0, im.data_size);
	return im;
}

Image Image::Empty(const Image& sameAs)
{
	Image im = Empty(sameAs.size, sameAs.dataType, sameAs.alignment);
	memset(im._ptr.get(), 0, im.data_size);
	return im;
}

Image Image::FromUnalignedData(const void* data, DataType d, ivec2 size, uint8_t alignment)
{
	Image im = Empty(size, d, alignment);
	// size_t bpp = im.GetBPP();

	for (int j = 0; j < im.size.y; ++j)
	{
		memcpy(
			im.GetRow<uint8_t>(j),
			(const char*)data + im.row_size * j,
			im.row_size);
	}
	return im;
}

static void NoDelete(const void*)
{
}

Image Image::FromMemory(const void* data, DataType d, ivec2 size, uint8_t alignment)
{
	Image im;
	im.size = size;
	im.dataType = d;
	im.alignment = alignment;
	im.row_stride = MemoryAlign(im.GetRowSize(), im.alignment);
	im.row_size = im.GetRowSize();
	im.data_size = im.row_stride * (size_t)im.size.y;
	im._ptr.reset((uint8_t*)data, NoDelete);
	return im;
}

void* Image::ToUnalignedData()
{
	size_t bpp = GetBPP();
	void* p = new uint8_t[bpp * size.x * size.y];
	for (int j = 0; j < size.y; ++j)
	{
		memcpy(
			(char*)p + row_size * j,
			GetRow<uint8_t>(j),
			row_size);
	}
	return p;
}

void Image::Assign(Image x)
{
	assert(x.dataType == dataType);
	assert(x.size == size);

	for (int j = 0; j < x.size.y; ++j)
	{
		memcpy(
			GetRow<uint8_t>(j),
			x.GetRow<uint8_t>(j),
			x.row_size);
	}
}

void Image::AssignToChannel(Image x, uint8_t channel)
{
	assert(x.dataType == R8);
	assert(x.size == size);

	size_t bpp = GetBPP();

	for (int j = 0; j < x.size.y; ++j)
	{
		uint8_t* dstRow = GetRow<uint8_t>(j);
		const uint8_t* srcRow = x.GetRow<uint8_t>(j);

		for (int i = 0; i < x.size.x; ++i)
		{
			uint8_t* dst = dstRow + i * bpp + channel;
			const uint8_t* src = srcRow + i;
			*dst = *src;
		}
	}
}

void Image::AssignToChannelZeroOther(Image x, uint8_t channel)
{
	assert(x.dataType == R8);
	assert(x.size == size);

	size_t bpp = GetBPP();

	for (int j = 0; j < x.size.y; ++j)
	{
		uint8_t* dstRow = GetRow<uint8_t>(j);
		const uint8_t* srcRow = x.GetRow<uint8_t>(j);

		for (int i = 0; i < x.size.x; ++i)
		{
			uint8_t* dst = dstRow + i * bpp;
			memset(dst, 0, bpp);
			const uint8_t* src = srcRow + i;
			*(dst + channel) = *src;
		}
	}
}

Image Image::OpenView(const ivec2 pos, const ivec2 size)
{
	ivec2 max_point = size + pos;
	auto lessoreq = lessThanEqual(max_point, GetSize());
	assert(all(lessoreq));

	Image view;
	view._ptr = _ptr;
	size_t bpp = GetBPP();
	view.dataType = dataType;
	view.data_size = data_size;
	view.size = size;
	view.offset = pos.y * row_stride + bpp * pos.x;
	view.row_size = bpp * size.x;
	view.row_stride = row_stride;

	return view;
}

Image Image::Copy() const
{
	Image im = Empty(size, dataType, alignment);
	// size_t bpp = GetBPP();

	for (int j = 0; j < im.size.y; ++j)
	{
		memcpy(
			im.GetRow<uint8_t>(j),
			GetRow<uint8_t>(j),
			im.row_size);
	}
	return im;
}

void Image::Clear()
{
	for (int j = 0; j < size.y; ++j)
	{
		memset(
			GetRow<uint8_t>(j),
			0,
			row_size);
	}
}

int Image::rows() const
{
	return GetSize().y;
}

int Image::cols() const
{
	return GetSize().x;
}

void Image::SaveToTGA(const char * filename)
{
	FILE* file = fopen(filename, "wb");
	char buff[18];
	int headerSize = sizeof(buff);
	memset(buff, 0, headerSize);
	buff[2] = 2;
	int height = GetSize().y;
	int width = GetSize().x;

	buff[0xc] = width % 256;
	buff[0xd] = width / 256;
	buff[0xe] = height % 256;
	buff[0xf] = height / 256;
	buff[0x10] = 24;
	fwrite(buff, headerSize, 1, file);

	int channelCount = GetChannelCount();

	uint8_t* row = new uint8_t[GetSize().x * 3];

	for (int j = size.y - 1; j >= 0; --j)
	{
		uint8_t* p = GetRow<uint8_t>(j);
		for (int j = 0; j < GetSize().x; ++j)
		{
			uint8_t rgb[3] = { 0, 0, 0 };

			for (int k = 0; k < channelCount; ++k)
			{
				rgb[k] = *(p + j * channelCount + k);
			}

			row[j * 3 + 0] = rgb[2];
			row[j * 3 + 1] = rgb[1];
			row[j * 3 + 2] = rgb[0];
		}
		fwrite(row, GetSize().x * 3, 1, file);
	}

	delete[] row;
	fclose(file);
}
