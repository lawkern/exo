/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

#define SIMD_WIDTH 4

/////////////////////////////////////////////////////////////////////////////////

#if(SIMD_WIDTH == 1)

typedef float f32w;
typedef u32 u32w;

function u32w set_u32w(u32 value)
{
   return(value);
}

function f32w set_f32w(float value)
{
   return(value);
}

function u32w convert_to_u32w(f32w values)
{
   return((u32w)values);
}

function f32w convert_to_f32w(u32w values)
{
   return((f32w)values);
}

function u32w loadu_u32w(u32w *source)
{
   return(*source);
}

function void storeu_u32w(u32w *destination, u32w value)
{
   *destination = value;
}

/////////////////////////////////////////////////////////////////////////////////

#elif(SIMD_WIDTH == 4)

#  include <immintrin.h>

typedef __m128 f32w;
typedef __m128i u32w;

function f32w operator+(f32w a, f32w b)
{
   return _mm_add_ps(a, b);
}

function f32w operator*(f32w a, f32w b)
{
   return _mm_mul_ps(a, b);
}

function u32w operator&(u32w a, u32w b)
{
   return _mm_and_si128(a, b);
}

function u32w operator|(u32w a, u32w b)
{
   return _mm_or_si128(a, b);
}

function u32w operator>>(u32w values, u32 immediate)
{
   return _mm_srli_epi32(values, immediate);
}

function u32w operator<<(u32w values, u32 immediate)
{
   return _mm_slli_epi32(values, immediate);
}

function u32w set_u32w(u32 value)
{
   return _mm_set1_epi32(value);
}

function f32w set_f32w(float value)
{
   return _mm_set1_ps(value);
}

function u32w convert_to_u32w(f32w values)
{
   return _mm_cvtps_epi32(values);
}

function f32w convert_to_f32w(u32w values)
{
   return _mm_cvtepi32_ps(values);
}

function u32w loadu_u32w(u32w *source)
{
   return _mm_loadu_si128(source);
}

function void storeu_u32w(u32w *destination, u32w value)
{
   _mm_storeu_si128(destination, value);
}

/////////////////////////////////////////////////////////////////////////////////

#elif(SIMD_WIDTH == 8)

#  include <immintrin.h>

typedef __m256 f32w;
typedef __m256i u32w;

function f32w operator+(f32w a, f32w b)
{
   return _mm256_add_ps(a, b);
}

function f32w operator*(f32w a, f32w b)
{
   return _mm256_mul_ps(a, b);
}

function u32w operator&(u32w a, u32w b)
{
   return _mm256_and_si256(a, b);
}

function u32w operator|(u32w a, u32w b)
{
   return _mm256_or_si256(a, b);
}

function u32w operator>>(u32w values, u32 immediate)
{
   return _mm256_srli_epi32(values, immediate);
}

function u32w operator<<(u32w values, u32 immediate)
{
   return _mm256_slli_epi32(values, immediate);
}

function u32w set_u32w(u32 value)
{
   return _mm256_set1_epi32(value);
}

function f32w set_f32w(float value)
{
   return _mm256_set1_ps(value);
}

function u32w convert_to_u32w(f32w values)
{
   return _mm256_cvtps_epi32(values);
}

function f32w convert_to_f32w(u32w values)
{
   return _mm256_cvtepi32_ps(values);
}

function u32w loadu_u32w(u32w *source)
{
   return _mm256_loadu_si256(source);
}

function void storeu_u32w(u32w *destination, u32w value)
{
   _mm256_storeu_si256(destination, value);
}

#else
#   error Unsupported SIMD width.
#endif
