#if !defined(GAME_INTRINSICS_H)
#include <intrin.h>

inline s32 Truncatef32(f32 input)
{
	return _mm_cvtt_ss2si(_mm_load_ss(&input));
}

inline s32 Ceilingf32(f32 input)
{
	return _mm_cvtss_si32(_mm_load_ss(&input));
}

inline f32 Sqrtf32(f32 input)
{
	f32 result = 0.0f;
	__m128 root = _mm_sqrt_ps(_mm_load_ss(&input));
	_mm_store_ps(&result, root);
	return result;
}

#define GAME_INTRINSICS_H

#endif	