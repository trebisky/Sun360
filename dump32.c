/* longs.c
 * dump an image as a bunch of 32 bit longs
 * Tom Trebisky
 * 4-30-2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

// char *inpath = "demo.bin";
char *inpath = "sun360.bin";

#define MAXIM	(1*1024*1024)
// #define MAXIM	(2*1024*1024)
unsigned char image[MAXIM];

// #define BASE 0x08000000
// #define BASE 0xfefe0000
#define BASE 0x0fef0000

void
error ( char *msg )
{
	fprintf ( stderr, "%s\n", msg );
	exit ( 1 );
}

void dumper ( int *buf, int count );

int
main ( int argc, char **argv )
{
		int fd;
		int n, nw;

		fd = open ( inpath, O_RDONLY );
		n = read ( fd, image, MAXIM );
		close ( fd );

		printf ( "%d bytes read\n", n );

		nw = sizeof(int);

		printf ( "%d words to dump\n", n/nw );

		dumper ( (int *) image, n/nw );
}

void
swap4 ( int *val )
{
		union {
			unsigned int ii;
			unsigned char cc[4];
		} uu;
		int tt;

		uu.ii = *val;
		tt = uu.cc[0];
		uu.cc[0] = uu.cc[3];
		uu.cc[3] = tt;
		tt = uu.cc[1];
		uu.cc[1] = uu.cc[2];
		uu.cc[2] = tt;
		*val = uu.ii;
}

void
dumper ( int *buf, int count )
{
		int i;
		unsigned int addr = BASE;
		int val;

		for ( i=0; i<count; i++ ) {
			val = buf[i];
			swap4 ( &val );
			printf ( "%08x:  %08x\n", addr, val );
			addr += 4;
		}
}

/* THE END */
