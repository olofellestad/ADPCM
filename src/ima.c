#include "ima.h"

#define CLAMP( x, a, b )	( ( x ) < ( a ) ? ( a ) : ( x ) > ( b ) ? ( b ) : ( x ) )

static const int16 stepTable[] =
{
	7,		8,		9,		10,		11,		12,		13,		14,		16,		17, 
	19, 	21,		23,		25,		28,		31,		34,		37,		41,		45, 
	50, 	55,		60,		66,		73,		80,		88,		97,		107,	118, 
	130,	143,	157,	173,	190,	209,	230,	253,	279,	307,
	337,	371,	408,	449,	494,	544,	598,	658,	724,	796,
	876,	963,	1060,	1166,	1282,	1411,	1552,	1707,	1878,	2066, 
	2272,	2499,	2749,	3024,	3327,	3660,	4026,	4428,	4871,	5358,
	5894,	6484,	7132,	7845,	8630,	9493,	10442,	11487,	12635,	13899, 
	15289,	16818,	18500,	20350,	22385,	24623,	27086,	29794,	32767 
};

static const int8 indexTable[] =
{
	-1, -1, -1, -1, 2, 4, 6, 8,
	-1, -1, -1, -1, 2, 4, 6, 8,
};

#define IMA_MIN_PREDICTOR	-32768
#define IMA_MAX_PREDICTOR	+32767

static uint8
IMA_CompressNibble( const int16 sample, int *predictor, int *stepIndex )
{
	int step	= stepTable[ *stepIndex ];
	int diff	= sample - *predictor;
	uint8 n		= 0;

	if ( diff < 0 )	{ diff = -diff; n = 8; }

	if ( diff >= step )	{ diff -= step; n |= 4; step >>= 1; }
	if ( diff >= step )	{ diff -= step; n |= 2; step >>= 1; }
	if ( diff >= step )	{ diff -= step; n |= 1; step >>= 1; }

	int delta = stepTable[ *stepIndex ] >> 3;
	delta += !!( n & 4 ) * stepTable[ *stepIndex ];
	delta += !!( n & 2 ) * stepTable[ *stepIndex ] >> 1;
	delta += !!( n & 1 ) * stepTable[ *stepIndex ] >> 2;

	if ( n & 8 )	*predictor -= delta;
	else			*predictor += delta;

	*predictor = CLAMP( *predictor, IMA_MIN_PREDICTOR,
									IMA_MAX_PREDICTOR );

	*stepIndex += indexTable[ n ];
	*stepIndex = CLAMP( *stepIndex, 0, 88 );
	return n;
}

int64
IMA_Compress( const int16 *src, const int64 size,
			  uint8 *dst )
{
	int64 compressed = 0;

	int predictor	= 0;
	int stepIndex	= 0;
	for ( int64 i = 0; i < size; i += 2 ) {
		const uint8 n0 = IMA_CompressNibble( src[ i ], &predictor, &stepIndex );
		const uint8 n1 = ( i + 1 ) < size ?
			IMA_CompressNibble( src[ i + 1 ], &predictor, &stepIndex )
			: 0;
		dst[ compressed++ ] = ( n1 << 4 ) | n0;
	}

	return compressed;
}

int64
IMA_Decompress( const uint8 *src, const int64 size,
				int16 *dst )
{
	int64 decompressed = 0;

	int predictor	= 0;
	int stepIndex	= 0;
	for ( int64 i = 0; i < size; i++ ) {
		const uint8 byte = src[ i ];
		for ( int j = 0; j < 2; j++ ) {
			const uint8 n	= ( byte >> ( 4 * j ) ) & 0xF;
			const int step	= stepTable[ stepIndex ];

			int diff = step >> 3;
			diff += !!( n & 4 ) * ( step      );
			diff += !!( n & 2 ) * ( step >> 1 );
			diff += !!( n & 1 ) * ( step >> 2 );
			if ( n & 8 )	predictor -= diff;
			else			predictor += diff;

			predictor = CLAMP( predictor, IMA_MIN_PREDICTOR,
										  IMA_MAX_PREDICTOR );
			dst[ decompressed++ ] = predictor;

			stepIndex += indexTable[ n ];
			stepIndex = CLAMP( stepIndex, 0, 88 );
		}
	}
	
	return decompressed;
}
