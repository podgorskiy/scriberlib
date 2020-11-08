#pragma once
#include "Utils.h"

#if (defined(_M_X64) || defined(_M_ARM64) || defined(__x86_64__) || defined(__aarch64__))
#define SCRIBER_SIMD
#endif

#if (defined(_M_X64) || ( defined(__x86_64__)) && defined(__SSE2__))
#if defined(_MSC_VER)
#include <intrin.h>
#endif

#define HAVE_SSE 1
#include <immintrin.h>

#elif defined(__ARM_NEON) || defined(__aarch64__)
#define SCRIBER_SIMD
#include <arm_neon.h>
#define HAVE_NEON 1

#endif


namespace simd
{
#if defined(HAVE_SSE)
	typedef __m128 f4;
#define VSTORE _mm_storeu_ps
#define VLD _mm_loadu_ps
#define VSET _mm_set1_ps
#define VADD _mm_add_ps
#define VSUB _mm_sub_ps
#define VMUL _mm_mul_ps
#define VDIV _mm_div_ps
#define VMIN _mm_min_ps
#define VMAX _mm_max_ps
#define RSQRT _mm_rsqrt_ps
#define SQRT _mm_sqrt_ps
#define VMAC(a, x, y) _mm_add_ps(a, _mm_mul_ps(x, y))
#define VMSB(a, x, y) _mm_sub_ps(a, _mm_mul_ps(x, y))
#define VMUL_S(x, s)  _mm_mul_ps(x, _mm_set1_ps(s))
#define VREV(x) _mm_shuffle_ps(x, x, _MM_SHUFFLE(0, 1, 2, 3))
#elif defined(HAVE_SSE)
	typedef float32x4_t f4;
#define VSTORE vst1q_f32
#define VLD vld1q_f32
#define VSET vmovq_n_f32
#define VADD vaddq_f32
#define VSUB vsubq_f32
#define VMUL vmulq_f32
#define VMAC(a, x, y) vmlaq_f32(a, x, y)
#define VMSB(a, x, y) vmlsq_f32(a, x, y)
#define VMUL_S(x, s)  vmulq_f32(x, vmovq_n_f32(s))
#define VREV(x) vcombine_f32(vget_high_f32(vrev64q_f32(x)), vget_low_f32(vrev64q_f32(x)))
#endif
}
