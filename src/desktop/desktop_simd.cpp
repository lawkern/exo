/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

#if __ARM_NEON
#   define SIMD_WIDTH 1 // TODO: Implement NEON support.
#else
#   define SIMD_WIDTH 1
#endif

/////////////////////////////////////////////////////////////////////////////////

#if(SIMD_WIDTH == 1)

typedef float f32w;
typedef u32 u32w;

function u32w set_u32w(u32 scalar)
{
   return(scalar);
}

function f32w set_f32w(float scalar)
{
   return(scalar);
}

function u32w convert_to_u32w(f32w vector)
{
   return((u32w)vector);
}

function f32w convert_to_f32w(u32w vector)
{
   return((f32w)vector);
}

function u32w loadu_u32w(u32w *source)
{
   return(*source);
}

function void storeu_u32w(u32w *destination, u32w vector)
{
   *destination = vector;
}

/////////////////////////////////////////////////////////////////////////////////

#elif(SIMD_WIDTH == 4)

#  include <immintrin.h>

struct f32w
{
   __m128 value;
};

struct u32w
{
   __m128i value;
};

function f32w operator+(f32w a, f32w b)
{
   return {_mm_add_ps(a.value, b.value)};
}

function f32w operator-(f32w a, f32w b)
{
   return {_mm_sub_ps(a.value, b.value)};
}

function f32w operator*(f32w a, f32w b)
{
   return {_mm_mul_ps(a.value, b.value)};
}

function u32w operator&(u32w a, u32w b)
{
   return {_mm_and_si128(a.value, b.value)};
}

function u32w operator|(u32w a, u32w b)
{
   return {_mm_or_si128(a.value, b.value)};
}

function u32w operator>>(u32w vector, u32 immediate)
{
   return {_mm_srli_epi32(vector.value, immediate)};
}

function u32w operator<<(u32w vector, u32 immediate)
{
   return {_mm_slli_epi32(vector.value, immediate)};
}

function u32w set_u32w(u32 scalar)
{
   return {_mm_set1_epi32(scalar)};
}

function f32w set_f32w(float scalar)
{
   return {_mm_set1_ps(scalar)};
}

function u32w convert_to_u32w(f32w vector)
{
   return {_mm_cvtps_epi32(vector.value)};
}

function f32w convert_to_f32w(u32w vector)
{
   return {_mm_cvtepi32_ps(vector.value)};
}

function u32w loadu_u32w(u32w *source)
{
   return {_mm_loadu_si128(&source->value)};
}

function void storeu_u32w(u32w *destination, u32w vector)
{
   _mm_storeu_si128(&destination->value, vector.value);
}

/////////////////////////////////////////////////////////////////////////////////

#elif(SIMD_WIDTH == 8)

#  include <immintrin.h>

struct f32w
{
   __m256 value;
};

struct u32w
{
   __m256i value;
};

function f32w operator+(f32w a, f32w b)
{
   return {_mm256_add_ps(a.value, b.value)};
}

function f32w operator-(f32w a, f32w b)
{
   return {_mm256_sub_ps(a.value, b.value)};
}

function f32w operator*(f32w a, f32w b)
{
   return {_mm256_mul_ps(a.value, b.value)};
}

function u32w operator&(u32w a, u32w b)
{
   return {_mm256_and_si256(a.value, b.value)};
}

function u32w operator|(u32w a, u32w b)
{
   return {_mm256_or_si256(a.value, b.value)};
}

function u32w operator>>(u32w vector, u32 immediate)
{
   return {_mm256_srli_epi32(vector.value, immediate)};
}

function u32w operator<<(u32w vector, u32 immediate)
{
   return {_mm256_slli_epi32(vector.value, immediate)};
}

function u32w set_u32w(u32 scalar)
{
   return {_mm256_set1_epi32(scalar)};
}

function f32w set_f32w(float scalar)
{
   return {_mm256_set1_ps(scalar)};
}

function u32w convert_to_u32w(f32w vector)
{
   return {_mm256_cvtps_epi32(vector.value)};
}

function f32w convert_to_f32w(u32w vector)
{
   return {_mm256_cvtepi32_ps(vector.value)};
}

function u32w loadu_u32w(u32w *source)
{
   return {_mm256_loadu_si256(&source->value)};
}

function void storeu_u32w(u32w *destination, u32w vector)
{
   _mm256_storeu_si256(&destination->value, vector.value);
}

#else
#   error Unsupported SIMD width.
#endif
