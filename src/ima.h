#pragma once

#include <stdint.h>

typedef int64_t		int64;
typedef int32_t		int32;
typedef int16_t		int16;
typedef int8_t		int8;
typedef uint64_t	uint64;
typedef uint32_t	uint32;
typedef uint16_t	uint16;
typedef uint8_t		uint8;

int64 IMA_Compress( const int16 *src, const int64 size,
					uint8 *dst );
int64 IMA_Decompress( const uint8 *src, const int64 size,
					int16 *dst );
