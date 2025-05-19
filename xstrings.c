/* xstrings.c -- pull strings out of our ROM image
 * Tom Trebisky  10-30-2024
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define MAX_ROMSIZE 150000
// #define BIN_PATH "sun380.bin"
#define BIN_PATH "sun360.bin"

char buf[MAX_ROMSIZE];
int romsize;

// unsigned int rombase = 0xfefe0000;
unsigned int rombase = 0x0fef0000;
unsigned int romend;

void
read_rom ( char *path )
{
	int rom;

	rom = open ( path, O_RDONLY );
	if ( rom < 0 ) {
	    fprintf ( stderr, "Cannot access file: %s\n", path );
	    exit ( 1 );
	}

	romsize = read ( rom, buf, MAX_ROMSIZE );
	// printf ( " ... %d bytes read\n", romsize );
	close ( rom );

	romend = rombase + romsize - 1;
}

/* This is used by batch and has special handling
 * of newlines.
 */
int
dump_string ( char *ss, int index )
{
		int newl;
		char *p;
		int c;

		newl = 0;
		p = ss;
		while ( c = buf[index] ) {
			if ( c == '\r' ) {
				*p++ = '\\';
				*p++ = 'r';
			} else if ( c == '\n' ) {
				*p++ = '\\';
				*p++ = 'n';
				newl = 1;
			} else if ( c < ' ' || c > '~' ) {
				*p++ = '.';
			} else
				*p++ = c;
			index++;
			if ( newl && buf[index] ) {
				*p++ = ' ';
				*p++ = '+';
				*p++ = '+';
				break;
			}
		}
		*p = '\0';

		return index;
}

/* This is used by single and prints to a null byte with
 * no hanky panky
 */
void
dump_string_basic ( char *ss, int index )
{
		char *p;
		int c;

		p = ss;
		while ( c = buf[index] ) {
			if ( c == '\r' ) {
				*p++ = '\\';
				*p++ = 'r';
			} else if ( c == '\n' ) {
				*p++ = '\\';
				*p++ = 'n';
			} else if ( c < ' ' || c > '~' ) {
				*p++ = '~';
			} else
				*p++ = c;
			index++;
		}
		*p = '\0';
}

void
batch ( unsigned int start, unsigned int end )
{
	int o_start, o_end;
	int index;
	unsigned int a_str;
	char ss[1024];

	// o_start = start - rombase;
	o_start = start;
	// o_end = end - rombase;
	o_end = end;
	index = o_start;

	for ( ;; ) {
		a_str = rombase + index;
		index = dump_string ( ss, index );
		printf ( "%08x: %s\n", a_str, ss );

		while ( buf[index] == '\0' )
			index++;
		if ( index >= o_end )
			break;
	}
}

void
sorry ( void )
{
		fprintf ( stderr, "Sorry\n" );
		exit ( 1 );
}

void
single ( char *arg_addr )
{
		unsigned int addr;
		char ss[1024];
		int index;

		addr = strtol ( arg_addr, NULL, 16 );
		if ( addr < rombase )
			sorry ();
		if ( addr > romend )
			sorry ();

		index = addr - rombase;
		dump_string_basic ( ss, index );
		// printf ( "%08x: %s\n", addr, ss );
		printf ( "%s\n", ss );
}

/* The sun 3/80 dump is big endian.
 * we want to generate a string anyway,
 * so we loop and read the bytes in order
 * rather than playing games with integers
 */
void
get_long ( char *arg_addr )
{
		unsigned int addr;
		char ss[1024];
		int index;
		int *p;
		int i, val;

		addr = strtol ( arg_addr, NULL, 16 );
		if ( addr < rombase )
			sorry ();
		if ( addr > romend )
			sorry ();

		index = addr - rombase;

		for ( i=0; i<4; i++ ) {
			val = buf[index+i] & 0xff;
			printf ( "%02x", val );
		}
		printf ( "\n" );
}

/* Currently this can do 3 things:
 *
 * 1) without arguments, does a "batch" of strings
 *    using the internal limits (see below)
 * 2) with one argument, prints a single string
 *    at the address given
 * 3) with two arguments, fetches a 4 byte long
 *    from that address and prints it as hex.
 *    i.e.  ./xstrings long fefe7738
 *    the first argument "long" is currently ignored
 */
int
main ( int argc, char **argv )
{
		read_rom ( BIN_PATH );

		argc--;
		argv++;

		/* End address is last address in last string */
		if ( argc == 0 ) {
			// These are from the 3-80 disassembly
#define STR_START	0xfeff3c8c
#define STR_END 	0xfeff4266
			// batch ( STR_START, STR_END );
			// batch ( 0xfeff4468, 0xfeff4c90 );
			// batch ( 0xfeff60d0, 0xfeff6b76 );
			// batch ( 0xfeff542c, 0xfeff5d26 );
			// batch ( 0xfeff5e08, 0xfeff600a );

			// batch ( 0xfefe7640, 0xfefe79e6 );
			// batch ( 0xfefe962c, 0xfefe99ea );
			// batch ( 0xfefe8710, 0xfefe8940 );
			// batch ( 0xfefe7fec, 0xfefe83f9 );
			// batch ( 0xfefe7be2, 0xfefe7c48 );
			// batch ( 0xfefed788, 0xfefede2a );
			// batch ( 0xfefee240, 0xfefee48a );
			// batch ( 0xfefeb31e, 0xfefeb61c );
			// batch ( 0xfefec500, 0xfefec800 );
			// batch ( 0xfefea872, 0xfefeaa50 );
			// batch ( 0xfefeab0c, 0xfefeaf94 );

			// These are from the 3-60 disassembly
			// batch ( 0xe738, 0xf480 );
			// batch ( 0xdb34, 0xdc10 );
			// batch ( 0xc544, 0xda90 );
			batch ( 0xf38e, 0xf480 );
		} else if ( argc == 1 ) {
			single ( argv[0] );
		} else {
			// argv[0] is "long", we cheat
			// and just assume that for now.
			get_long ( argv[1] );
		}

		return 0;
}

/* THE END */
