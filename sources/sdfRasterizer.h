#pragma once
#include <cmath>
#include <assert.h>
#include <freetype.h>

#include "Utils.h"


namespace Scriber
{
	namespace detail
	{
		inline float sdLine(vec2 pos, vec2 A, vec2 B)
		{
			if (A == B)
				return 1e6;
			//printf("dd = intersection(dd, sdLine(p, vec2(%f, %f), vec2(%f, %f)));\n", A.x, A.y, B.x, B.y);
			vec2 pa = pos - A, ba = B - A;
			float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
			float d = length(pa - ba * h);

			float sa = cross2(A, pos);
			float sc = cross2(ba, pa);
			float s0 = cross2(-B, pos - B);

			bool has_neg = (sa < 0.f) || (sc < 0.f) || (s0 < 0.f);
			bool has_pos = (sa > 0.f) || (sc > 0.f) || (s0 > 0.f);

			float ts = 1.0f - float(!(has_neg && has_pos)) * 2.0f;
			return d * ts;
		}

		// signed distance to a quadratic bezier
		inline float sdBezier(vec2 pos, vec2 A, vec2 B, vec2 C)
		{
			if (A == B)
				return sdLine(pos, B, C);
			if (B == C)
				return sdLine(pos, A, B);
			//printf("dd = intersection(dd, sdBezier(p, vec2(%f, %f), vec2(%f, %f), vec2(%f, %f)));\n", A.x, A.y, B.x, B.y, C.x, C.y);
			if (cross2(C - A, B - A) < 0.0f)
			{
				vec2 t = A;
				A = C;
				C = t;
			}
			vec2 a = B - A;
			vec2 b = A - 2.0f * B + C;
			vec2 c = a * 2.0f;
			vec2 d = A - pos;

			float kk = 1.0f / dot(b, b);
			float kx = kk * dot(a, b);
			float ky = kk * (2.0f * dot(a, a) + dot(d, b)) / 3.0f;
			float kz = kk * dot(d, a);

			float res = 0.0f;
			float sgn = 1.0f;

			float p = ky - kx * kx;
			float p3 = p * p * p;
			float q = kx * (2.0f * kx * kx - 3.0f * ky) + kz;
			float h = q * q + 4.0f * p3;
			float sa = cross2(A - 0.f, pos - 0.f);
			float sc = cross2(C - A, pos - A);
			float s0 = cross2(0.f - C, pos - C);

			if (h >= 0.0)
			{   // 1 root
				h = std::sqrt(h);
				vec2 x = (vec2(h, -h) - q) / 2.0f;
				vec2 uv = sign(x) * pow(abs(x), vec2(1.0f / 3.0f));
				float t = clamp(uv.x + uv.y - kx, 0.0f, 1.0f);
				vec2 q = d + (c + b * t) * t;
				res = dot2(q);
				sgn = cross2(c + 2.0f * b * t, q);
			}
			else
			{   // 3 roots
				float z = std::sqrt(-p);
				float v = std::acos(q / (p * z * 2.0f)) / 3.0f;
				float m = std::cos(v);
				float n = std::sin(v) * 1.732050808f;
				vec2 t = clamp(vec2(m + m, -n - m) * z - kx, 0.0f, 1.0f);
				vec2 qx = d + (c + b * t.x) * t.x;
				float dx = dot2(qx), sx = cross2(c + 2.0f * b * t.x, qx);
				vec2 qy = d + (c + b * t.y) * t.y;
				float dy = dot2(qy), sy = cross2(c + 2.0f * b * t.y, qy);
				if (dx < dy)
				{
					res = dx;
					sgn = sx;
				}
				else
				{
					res = dy;
					sgn = sy;
				}
			}
			bool has_neg = (sa < 0.f) || (sc < 0.f) || (s0 < 0.f);
			bool has_pos = (sa > 0.f) || (sc > 0.f) || (s0 > 0.f);

			float ts = 1.0f - float(!(has_neg && has_pos)) * 2.0f;

			return std::sqrt(res) * sign(sc <= 0.f ? 1.0f : -sgn) * ts;
		}

		inline float intersection(float d1, float d2)
		{
			float dmin = min(abs(d1), abs(d2));
			return dmin * sign(d1) * sign(d2);
		}

		struct FT_Glyph_Class_
		{
			FT_Long glyph_size;
			FT_Glyph_Format glyph_format;
			FT_Error (*glyph_init)(FT_Glyph glyph, FT_GlyphSlot slot);
			void (*glyph_done)(FT_Glyph glyph);
			FT_Error (*glyph_copy)(FT_Glyph source, FT_Glyph target);
			void (*glyph_transform)(FT_Glyph glyph, const FT_Matrix*  matrix, const FT_Vector*  delta);
			void (*glyph_bbox)(FT_Glyph glyph, FT_BBox* abbox);
			FT_Error (*glyph_prepare)(FT_Glyph glyph, FT_GlyphSlot slot);
		};
	}

    inline FT_BitmapGlyph RenderSDF(int margin, float cutoff, FT_Face ft_face)
    {
		FT_BitmapGlyphRec_* bitmap = (FT_BitmapGlyphRec_*)malloc(sizeof(FT_BitmapGlyphRec_));
		memset(bitmap, 0, sizeof(FT_BitmapGlyphRec_));
    	bitmap->root.format = FT_Glyph_Format::FT_GLYPH_FORMAT_BITMAP;
    	bitmap->root.library = ft_face->glyph->library;
    	static detail::FT_Glyph_Class_ custom_class = {sizeof(FT_BitmapGlyphRec_), FT_Glyph_Format::FT_GLYPH_FORMAT_BITMAP,
	                                                   nullptr,
	                                                   +[](FT_Glyph glyph){ free(((FT_BitmapGlyphRec_*)(glyph))->bitmap.buffer); },
	                                                   nullptr,nullptr,nullptr,nullptr};
    	bitmap->root.clazz = (FT_Glyph_Class_*)&custom_class;

        if (ft_face->glyph->format != FT_GLYPH_FORMAT_OUTLINE)
        {
            return bitmap;
        }

        FT_Outline outline = ft_face->glyph->outline;

        FT_BBox acbox;
        FT_Outline_Get_CBox(&outline, &acbox);

        int bbox_xmin = acbox.xMin / 64 - margin;
        int bbox_ymin = acbox.yMin / 64 - margin;
        int bbox_xmax = (acbox.xMax + 63) / 64 + margin;
        int bbox_ymax = (acbox.yMax + 63) / 64 + margin;

        bitmap->left = bbox_xmin;
        bitmap->top = bbox_ymax;
        int glyph_width = bbox_xmax - bbox_xmin;
        int glyph_height = bbox_ymax - bbox_ymin;

        unsigned int buffered_width = glyph_width;
        unsigned int buffered_height = glyph_height;
        unsigned int bitmap_size = buffered_width * buffered_height;

        bitmap->bitmap.buffer = (unsigned char*)malloc(bitmap_size);
        memset(bitmap->bitmap.buffer, 0, bitmap_size);
        bitmap->bitmap.width = buffered_width;
        bitmap->bitmap.rows = buffered_height;

	    int radius = 8;
	    int radius_by_256 = (256 / radius);

        // Loop over every pixel and determine the positive/negative distance to the outline.
        for (unsigned int y = 0; y < buffered_height; y++)
        {
            for (unsigned int x = 0; x < buffered_width; x++)
            {
                unsigned int ypos = buffered_height - y - 1;
                unsigned int i = ypos * buffered_width + x;
                vec2 pt = vec2(x, y) + vec2(bbox_xmin, bbox_ymin) + vec2(0.513f, 0.507f);

                float d = 1e6f;

				int first = 0;
				for (int n = 0; n < outline.n_contours; ++n)
				{
					using namespace detail;

					int last  = outline.contours[n];
					FT_Vector* limit = outline.points + last;
	                vec2 v_start = vec2(outline.points[first].x, outline.points[first].y) / 64.0f;
	                vec2 v_last = vec2(outline.points[last].x, outline.points[last].y) / 64.0f;
	                vec2 v_control;
					FT_Vector* point = outline.points + first;
					char* tags = outline.tags + first;
					char tag = FT_CURVE_TAG(tags[0]);
					if (tag == FT_CURVE_TAG_CUBIC)
						return bitmap;
					if ( tag == FT_CURVE_TAG_CONIC )
					{
						v_start = v_last;
						limit--;
					}
					else
					{
						v_start = (v_start + v_last) / 2.0f;
					}
					--point;
					--tags;

					vec2 prev = v_start;

					while (point < limit)
					{
						++point;
						++tags;
						tag = FT_CURVE_TAG(tags[0]);
						vec2 p = vec2(point->x, point->y) / 64.0f;
						switch ( tag )
						{
							case FT_CURVE_TAG_ON:
								d = intersection(d, sdLine(pt, prev, p));
								prev = p;
								continue;
							case FT_CURVE_TAG_CONIC:
								v_control = p;
								Do_Conic:
		                        if (point < limit)
		                        {
									++point;
									++tags;
									tag = FT_CURVE_TAG(tags[0]);
									vec2 p = vec2(point->x, point->y) / 64.0f;
									if ( tag == FT_CURVE_TAG_ON )
									{
										d = intersection(d, sdBezier(pt, prev, v_control, p));
										prev = p;
										continue;
									}
									if (tag != FT_CURVE_TAG_CONIC)
										return bitmap;
									vec2 v_middle = (v_control + p) / 2.0f;
									d = intersection(d, sdBezier(pt, prev, v_control, v_middle));
									prev = v_middle;
									v_control = p;
									goto Do_Conic;
		                        }
								d = intersection(d, sdBezier(pt, prev, v_control, v_start));
								prev = v_start;
								goto Close;
							default:
								return bitmap;

						}
					}
					if (prev != v_start)
					{
						d = intersection(d, sdLine(pt, prev, v_start));
						prev = v_start;
					}
                    Close:
						first = last + 1;
				}
				//return bitmap;
                d *= radius_by_256;
                d += cutoff * 256;

                // Clamp to 0-255 to prevent overflows or underflows.
                int n = d > 255 ? 255 : d;
                n = n < 0 ? 0 : n;

                bitmap->bitmap.buffer[i] = static_cast<unsigned char>(255 - n);
            }
        }
        return bitmap;
    }

}
