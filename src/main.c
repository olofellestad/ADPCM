#include "ima.h"

#include <stdio.h>
#include <stdlib.h>

int main( void )
{
	uint8 *dst = NULL;
	int16 *wav = NULL;
	uint16 numChannels	= 0;
	uint16 sampleBits	= 0;
	uint32 sampleRate	= 0;
	int64 size = 0;
	{
		FILE *wavFile = fopen( "Untitled(37).wav", "rb" );

		uint32 u = 0;

		fread( &u, 4, 1, wavFile );	if ( u != 'FFIR' )	return 1;
		fread( &u, 4, 1, wavFile );
		fread( &u, 4, 1, wavFile );	if ( u != 'EVAW' )	return 1;
		fread( &u, 4, 1, wavFile );	if ( u != ' tmf' )	return 1;
		fread( &u, 4, 1, wavFile );
		fread( &u, 2, 1, wavFile );	if ( u != 1 )	return 1;

		fread( &numChannels, 2, 1, wavFile );
		fread( &sampleRate, 4, 1, wavFile );
		fread( &u, 4, 1, wavFile );
		fread( &u, 2, 1, wavFile );
		fread( &sampleBits, 2, 1, wavFile );

		if ( sampleBits != 16 )	return 1;
		
		fread( &u, 4, 1, wavFile );	if ( u != 'atad' )	return 1;
		fread( &u, 4, 1, wavFile );
		wav = malloc( u );
		dst = malloc( u );
		fread( wav, u, 1, wavFile );
		fclose( wavFile );

		size = (int64)u / ( sampleBits / 8 );
	}
	printf( "size = %lli (%lli kB)\n", size, ( size*( sampleBits/8 ) ) / 1024 );

	if ( wav == NULL )	return 1;
	if ( dst == NULL )	return 1;

	const int64 n = IMA_Compress( wav, size, dst );
	printf( "n = %lli\n", n );

	int16 *unc = malloc( size * sizeof( int16 ) );
	const int64 m = IMA_Decompress( dst, n, unc );
	printf( "m = %lli\n", m );
	{
		FILE *wavFile = fopen( "adpcm.wav", "wb" );
		uint32 u;

		u = 'FFIR';	fwrite( &u, 4, 1, wavFile );
		u = 2 * m + 44;	fwrite( &u, 4, 1, wavFile );
		u = 'EVAW';	fwrite( &u, 4, 1, wavFile );
		u = ' tmf';	fwrite( &u, 4, 1, wavFile );

		u = 40;		fwrite( &u, 4, 1, wavFile );
		u = 1;		fwrite( &u, 2, 1, wavFile );
		u = numChannels;	fwrite( &u, 2, 1, wavFile );
		u = sampleRate;		fwrite( &u, 4, 1, wavFile );
		u = sampleRate*numChannels*sizeof(int16);	fwrite( &u, 4, 1, wavFile );
		u = numChannels*sizeof(int16);		fwrite( &u, 2, 1, wavFile );
		u = sampleBits;		fwrite( &u, 2, 1, wavFile );
		
		u = 'atad';	fwrite( &u, 4, 1, wavFile );
		u = m;		fwrite( &u, 4, 1, wavFile );
		
		fwrite( unc, 1, m * sizeof( int16 ), wavFile );
		fclose( wavFile );
	}

	free( unc );
	free( dst );
	free( wav );

	return 0;
}
