//
// Created by Jonathan on 08.11.2021.
//

#ifndef AUDIOFY_LIB_MEMUTIL_H
#define AUDIOFY_LIB_MEMUTIL_H
#include <immintrin.h>
#include "types.h"

#ifdef _WIN32
#include <intrin.h>
#define cpuid(info, x) __cpuidex(info,x ,0)
#else
void cpuid(int info[4], int infoType) {
    __cpuid_count(infoType, 0, info[0], info[1], info[2], info[3]);
}
#endif
/*
    * MISC
    */

/*
 * This may be faster than memset if the dst buffer is properly aligned.
 * However: Most compilers already generate code like this
 */
/// Utility replacement for memset. Please only use this if you know why.
/// \param dst Destination
/// \param val Value
/// \param bytes Number of bytes of val to write to destination buffer
template <bool align=false>
void mmMemSet(_Out_writes_bytes_all_(bytes) u8* __restrict dst,
              u32 val,
              size_t bytes)  {

    auto reg = _mm256_set1_epi32(val);
    auto* __restrict p = reinterpret_cast<__m256i*>(dst);

    for(auto i = bytes / sizeof(*p); i>0 ; i--, p++) {
        if constexpr(align) {_mm256_store_si256(reinterpret_cast<__m256i*>(p), reg);}
        else {_mm256_storeu_si256(reinterpret_cast<__m256i*>(p), reg);}
    }
}

/*
 * As a general rule of thumb: Memcpy will most likely outperform this function unless
 * you use non-temporal writes and know that you'll be copying memory to a destination which you will not actually
 * read from immediately. AVX2 may incur penalties like clocking down the cpu and shit like this
 * so please DON'T use this unless you know what you're doing
 */
/// Utility replacement for memcpy. Please only use this if you know why
/// \tparam stream if true use non temopral writes
/// \param src src
/// \param dst dst
/// \param bytes number of bytes to copy
template <bool stream=false, bool align=false>
void mmMemCopy(_In_reads_bytes_(bytes) const void*__restrict src,
               _Out_writes_bytes_all_(bytes) u8* __restrict dst,
               size_t bytes) {
    const auto* __restrict srcPtr = reinterpret_cast<const __m256i*>(src);

    auto* __restrict dstPtr = reinterpret_cast<__m256i*>(dst);

    i64 i = bytes / sizeof(*dstPtr);

    for(; i > 0; i--, srcPtr++, dstPtr++) {
        if constexpr(!stream) {
            const __m256i source = _mm256_load_si256(srcPtr);
            if constexpr(align) {
                _mm256_storeu_si256(dstPtr, source);
            } else {
                _mm256_store_si256(dstPtr, source);
            }

        } else {
            const __m256i source = _mm256_stream_load_si256(srcPtr);
            _mm256_stream_si256(dstPtr, source);
        }
    }
    if constexpr(stream) {
        _mm_sfence();
    }
}

#endif //AUDIOFY_LIB_MEMUTIL_H
