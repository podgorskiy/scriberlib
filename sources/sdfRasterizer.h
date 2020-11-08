#pragma once
#include <cmath>
#include <assert.h>
#include <freetype.h>

#include "Utils.h"
#include "simd.h"

#if !defined(SCRIBER_SDF_USE_OMP)
#if defined(_OPENMP)
#define SCRIBER_SDF_USE_OMP 1
#else
#define SCRIBER_SDF_USE_OMP 0
#endif
#endif

#if SCRIBER_SDF_USE_OMP
#include <omp.h>
#endif

#ifdef _MSC_VER
#define T4_Pragma(X) __pragma(X)
#else
#define T4_Pragma(X) _Pragma(#X)
#endif

#if SCRIBER_SDF_USE_OMP
#define OMP_THREAD_ID omp_get_thread_num()
#define OMP_MAX_THREADS omp_get_max_threads()
#define parallel_for T4_Pragma(omp parallel for) for
#else
#define OMP_THREAD_ID 0
#define OMP_MAX_THREADS 1
#define parallel_for for
#endif


namespace Scriber
{
	namespace detail
	{
		inline float sdLine(vec2 pos, vec2 A, vec2 B)
		{
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

#define CROSS2(X1, X2) VSUB(VMUL(X1##x, X2##y), VMUL(X1##y, X2##x))
#define DOT(X1, X2) VADD(VMUL(X1##x, X2##x), VMUL(X1##y, X2##y))

		inline simd::f4 sdLineSIMD(float* _pos, vec2 _A, vec2 _B)
		{
			vec2 ba = _B - _A;
			float _dba = dot(ba, ba);
			simd::f4 posx = VLD(_pos);
			simd::f4 posy = VLD(_pos + 4);
			simd::f4 Ax = VSET(_A.x);
			simd::f4 Ay = VSET(_A.y);
			simd::f4 Bx = VSET(_B.x);
			simd::f4 By = VSET(_B.y);
			simd::f4 pax = VSUB(posx, Ax);
			simd::f4 pay = VSUB(posy, Ay);
			simd::f4 bax = VSET(ba.x);
			simd::f4 bay = VSET(ba.y);
			simd::f4 dba = VSET(_dba);
			simd::f4 h = VMAX(VMIN(VDIV(DOT(pa, ba), dba), VSET(1.0)) , VSET(0.0));
			simd::f4 vdx = VSUB(pax, VMUL(bax, h));
			simd::f4 vdy = VSUB(pay, VMUL(bay, h));
			simd::f4 d = SQRT(VADD(VMUL(vdx, vdx), VMUL(vdy, vdy)));
			simd::f4 posmBx = VSUB(posx, Bx);
			simd::f4 posmBy = VSUB(posy, By);

			simd::f4 sa = CROSS2(A, pos);
			simd::f4 sc = CROSS2(ba, pa);
			simd::f4 s0 = CROSS2(-B, posmB);

			float sa4[4];
			float sc4[4];
			float s04[4];
			VSTORE(sa4,sa);
			VSTORE(sc4,sc);
			VSTORE(s04,s0);

			bool has_neg0 = (sa[0] < 0.f) || (sc[0] < 0.f) || (s0[0] < 0.f);
			bool has_pos0 = (sa[0] > 0.f) || (sc[0] > 0.f) || (s0[0] > 0.f);
			bool has_neg1 = (sa[1] < 0.f) || (sc[1] < 0.f) || (s0[1] < 0.f);
			bool has_pos1 = (sa[1] > 0.f) || (sc[1] > 0.f) || (s0[1] > 0.f);
			bool has_neg2 = (sa[2] < 0.f) || (sc[2] < 0.f) || (s0[2] < 0.f);
			bool has_pos2 = (sa[2] > 0.f) || (sc[2] > 0.f) || (s0[2] > 0.f);
			bool has_neg3 = (sa[3] < 0.f) || (sc[3] < 0.f) || (s0[3] < 0.f);
			bool has_pos3 = (sa[3] > 0.f) || (sc[3] > 0.f) || (s0[3] > 0.f);

			float ts[4] = {
					1.0f - float(!(has_neg0 && has_pos0)) * 2.0f,
					1.0f - float(!(has_neg1 && has_pos1)) * 2.0f,
					1.0f - float(!(has_neg2 && has_pos2)) * 2.0f,
					1.0f - float(!(has_neg3 && has_pos3)) * 2.0f,
					};

			return VMUL(d, VLD(ts));
		}

		// signed distance to a quadratic bezier
		inline float sdBezier(vec2 pos, vec2 A, vec2 B, vec2 C)
		{
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

		inline void intersection(float* d0, simd::f4 d2)
		{
			float d4[4];
			VSTORE(d4, d2);
			d0[0] = intersection(d0[0], d4[0]);
			d0[1] = intersection(d0[1], d4[1]);
			d0[2] = intersection(d0[2], d4[2]);
			d0[3] = intersection(d0[3], d4[3]);
		}

		inline void intersection(float* d0, float d1, float d2, float d3, float d4)
		{
			d0[0] = intersection(d0[0], d1);
			d0[1] = intersection(d0[1], d2);
			d0[2] = intersection(d0[2], d3);
			d0[3] = intersection(d0[3], d4);
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

		enum Command: uint8_t
		{
			MoveTo,
			LineTo,
			ConicTo,
			End
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

        vec2 points[4096 * 4];
        assert(outline.n_points < 4096 * 4);
        detail::Command commands[4096 * 4];
        int point_it = 0;
        int cmd_it = 0;
        auto push_line = [&points, &point_it, &commands, &cmd_it](vec2 b)
        {
        	if (points[point_it - 1] != b)
	        {
		        points[point_it++] = b;
		        commands[cmd_it++] = detail::LineTo;
	        }
        };
        auto push_conic = [&points, &point_it, &commands, &cmd_it, &push_line](vec2 b, vec2 c)
        {
        	if (points[point_it - 1] == b || b == c)
	        {
        		push_line(c);
	        }
        	else
	        {
	            points[point_it++] = b;
		        points[point_it++] = c;
		        commands[cmd_it++] = detail::ConicTo;
	        }
        };

	    int first = 0;
	    for (int n = 0; n < outline.n_contours; ++n)
	    {
		    using namespace detail;

		    int last = outline.contours[n];
		    FT_Vector* limit = outline.points + last;
		    vec2 v_start = vec2(outline.points[first].x, outline.points[first].y) / 64.0f;
		    vec2 v_last = vec2(outline.points[last].x, outline.points[last].y) / 64.0f;
		    vec2 v_control;
		    FT_Vector* point = outline.points + first;
		    char* tags = outline.tags + first;
		    char tag = FT_CURVE_TAG(tags[0]);
		    if (tag == FT_CURVE_TAG_CUBIC)
		    {
			    return bitmap;
		    }
		    if (tag == FT_CURVE_TAG_CONIC)
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

		    points[point_it++] = v_start;
		    commands[cmd_it++] = detail::MoveTo;

		    while (point < limit)
		    {
			    ++point;
			    ++tags;
			    tag = FT_CURVE_TAG(tags[0]);
			    vec2 p = vec2(point->x, point->y) / 64.0f;
			    switch (tag)
			    {
				    case FT_CURVE_TAG_ON:
				    	push_line(p);
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
						    if (tag == FT_CURVE_TAG_ON)
						    {
								push_conic(v_control, p);
							    continue;
						    }
						    if (tag != FT_CURVE_TAG_CONIC)
						    {
							    return bitmap;
						    }
						    vec2 v_middle = (v_control + p) / 2.0f;
							push_conic(v_control, v_middle);
						    v_control = p;
						    goto Do_Conic;
					    }
						push_conic(v_control, v_start);
					    goto Close;
				    default:
					    return bitmap;

			    }
		    }
		    if (points[point_it] != v_start)
		    {
				push_line(v_start);
		    }
		    Close:
		    first = last + 1;
	    }
		commands[cmd_it] = detail::End;
        assert(cmd_it < 4096 * 4);
        assert(point_it < 4096 * 4);

	    int radius = 8;
	    float radius_by_256 = (256.0f / radius);

	    unsigned char* __restrict bmp = bitmap->bitmap.buffer;
	    parallel_for(unsigned int i = 0; i < buffered_height * buffered_width; i += 4)
	    {
		    vec2 pt[4] =
				    {
		    		    vec2((i + 0) % buffered_width, buffered_height - 1 - ((i + 0) / buffered_width)) + vec2(bbox_xmin, bbox_ymin) + vec2(0.513f, 0.507f),
		    		    vec2((i + 1) % buffered_width, buffered_height - 1 - ((i + 1) / buffered_width)) + vec2(bbox_xmin, bbox_ymin) + vec2(0.513f, 0.507f),
		    		    vec2((i + 2) % buffered_width, buffered_height - 1 - ((i + 2) / buffered_width)) + vec2(bbox_xmin, bbox_ymin) + vec2(0.513f, 0.507f),
		    		    vec2((i + 3) % buffered_width, buffered_height - 1 - ((i + 3) / buffered_width)) + vec2(bbox_xmin, bbox_ymin) + vec2(0.513f, 0.507f),
				    };
		    float pos_simd[8] = {pt[0].x,pt[1].x,pt[2].x,pt[3].x,pt[0].y,pt[1].y,pt[2].y,pt[3].y};

		    float d[4] = {1e6f, 1e6f, 1e6f, 1e6f};

		    const vec2* __restrict p = points;
		    --p;

		    for (auto* __restrict cmd = commands; *cmd != detail::End; ++cmd)
		    {
			    switch (*cmd)
			    {
				    case detail::MoveTo:
					    ++p;
					    break;
				    case detail::LineTo:
					    // detail::intersection(d, detail::sdLine(pt[0], p[0], p[1]), detail::sdLine(pt[1], p[0], p[1]), detail::sdLine(pt[2], p[0], p[1]), detail::sdLine(pt[3], p[0], p[1]));
					    detail::intersection(d, detail::sdLineSIMD(pos_simd, p[0], p[1]));
					    ++p;
					    break;
				    case detail::ConicTo:
					    detail::intersection(d, detail::sdBezier(pt[0], p[0], p[1], p[2]), detail::sdBezier(pt[1], p[0], p[1], p[2]), detail::sdBezier(pt[2], p[0], p[1], p[2]), detail::sdBezier(pt[3], p[0], p[1], p[2]));
					    p += 2;
					    break;
				    case detail::End:
					    break;
			    }
		    }

		    d[0] = d[0] * radius_by_256 + cutoff * 256.0f;
		    d[1] = d[1] * radius_by_256 + cutoff * 256.0f;
		    d[2] = d[2] * radius_by_256 + cutoff * 256.0f;
		    d[3] = d[3] * radius_by_256 + cutoff * 256.0f;
		    bmp[i + 0] = 255 - clamp((int) d[0], 0, 255);
		    bmp[i + 1] = 255 - clamp((int) d[1], 0, 255);
		    bmp[i + 2] = 255 - clamp((int) d[2], 0, 255);
		    bmp[i + 3] = 255 - clamp((int) d[3], 0, 255);
	    }
        return bitmap;
    }
}
