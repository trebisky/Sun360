/* wrap.c -- wrap a ROM image as an ELF file so
 * that it can be disassembled
 * Part of the Convergent Technologies "Mightyframe/S80" effort.
 * Tom Trebisky  9-5-2014
 * From /u1/tom/Mightyframe/Roms/wrap.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>	// for ntohs/htons

#include <elf.h>

#define PGSIZE 8192	/* 0x2000 */

/* Mightyframe ROM is 27256 (32K, 4 pages) */
// #define MAX_ROMSIZE 50000
/* Sun380 rom is 128K */
// #define MAX_ROMSIZE 131072
#define MAX_ROMSIZE 150000

#ifdef notdef
/* S80_boot_72-01231.BIN */
unsigned int rombase = 0x80000000;
unsigned int entry = 0x80000000;

/* heurikon.bin */
unsigned int rombase = 0x00fc0000;
unsigned int entry = 0x00fc0082;
#endif

/* now we get the image and elf names from the command line */
char *rom_image;
char *elf_image;

#ifdef notdef
/* For the Callan, trial and error */
/* first pair */
unsigned int rombase = 0x00200000;
unsigned int entry = 0x002000b0;

/* second pair */
unsigned int rombase = 0x00400000;
unsigned int entry = 0x00400000;
#endif

/* XXX these get overwritten from command line */
unsigned int rombase = 0x00400000;
unsigned int entry = 0x00400000;

char buf[MAX_ROMSIZE];
int romsize;

/* String table gets built here */
char sbuf[PGSIZE];
int string_off = 0;

int mk_string ( int );
int add_string ( char * );

void mk_shdr_null ( int );
void mk_shdr_t ( int, int );
void mk_shdr_g ( int, int );
void mk_shdr_s ( int, int, int );

int mk_phdr ( int );

char gnu_extra[] = "GCC: (GNU) 4.7.2";
int gnusize;

/*
 * Here is what an elf file looks like:
 *
 *  Elf header (52 bytes)
 *  Elf phdr (32 bytes) "program header"
 *  zero padding to get to 8k (8192) boundary
 *  binary image
 *  gnu extra string
 *  ELF string table
 *  ELF segment headers
 */

int main ( int argc, char **argv )
{
	Elf32_Ehdr hdr;
	int rom, of;
	int shnum;
	int ph_off;
	int sh_off;
	char pad[PGSIZE];
	int npad;
	int s_index;
	int g_index;
	int t_index;
	int st_size;

	argc--;
	argv++;

	if ( argc != 3 ) {
	    fprintf ( stderr, "Usage: wrap image-name elf-name addr\n" );
	    exit ( 1 );
	}

	rom_image = argv[0];
	elf_image = argv[1];

	rombase = strtol ( argv[2], NULL, 16 );
	entry = rombase;

	printf ( "Reading image: %s\n", rom_image );
	rom = open ( rom_image, O_RDONLY );
	if ( rom < 0 ) {
	    fprintf ( stderr, "Cannot access file: %s\n", rom_image );
	    exit ( 1 );
	}

	of = creat ( elf_image, 0664 );
	if ( of < 0 ) {
	    fprintf ( stderr, "Cannot create file: %s\n", elf_image );
	    exit ( 1 );
	}

	romsize = read ( rom, buf, MAX_ROMSIZE );
	printf ( " ... %d bytes read\n", romsize );
	close ( rom );

	gnusize = strlen(gnu_extra) + 1;

	shnum = 4;

	/* program table immediately follows elf header */
	ph_off = sizeof(hdr);

	/* build string table now */
	add_string ( "" );
	s_index = add_string ( ".shstrtab" );
	t_index = add_string ( ".text" );
	g_index = add_string ( ".comment" );

	/* May need to add a pad byte to string table so
	 * that segment headers get even alignment.
	 */
	sh_off = PGSIZE + romsize + gnusize + string_off;
	if ( (sh_off % 2) == 1 ) {
	    sbuf[string_off++] = 0;
	    sh_off++;
	}

	/* Fill in the elf header (52 bytes) */
	strncpy ( hdr.e_ident, "-ELF", 4 );
	hdr.e_ident[0] = 0x7F;
	/* note there are 16 bytes in the ident array */

	hdr.e_ident[4] = 1;
	hdr.e_ident[5] = 2;
	hdr.e_ident[6] = 1;
	hdr.e_ident[7] = 0;

	hdr.e_ident[8] = 0;
	hdr.e_ident[9] = 0;
	hdr.e_ident[10] = 0;
	hdr.e_ident[11] = 0;

	hdr.e_ident[12] = 0;
	hdr.e_ident[13] = 0;
	hdr.e_ident[14] = 0;
	hdr.e_ident[15] = 0;

	hdr.e_type = ntohs(2);

	/* XXX - how do we control 68010 versus 68020 disassembly ? */
	hdr.e_machine = ntohs(4);	/* mc680x0 */

	hdr.e_version = ntohl(1);
	hdr.e_entry = ntohl ( entry );
	hdr.e_phoff = ntohl ( ph_off );
	hdr.e_shoff = ntohl ( sh_off );
	hdr.e_flags = ntohl ( 0x01000000 );
	hdr.e_ehsize = ntohs ( 52 );
	hdr.e_phentsize = ntohs ( 0x20 );
	hdr.e_phnum = ntohs ( 1 );
	hdr.e_shentsize = ntohs ( shnum * 10 );
	hdr.e_shnum = ntohs ( shnum );
	hdr.e_shstrndx = ntohs ( 3 );

	write ( of, &hdr, sizeof(hdr) );

	memset ( pad, 0, PGSIZE );
	npad = PGSIZE - sizeof(hdr);

	npad -= mk_phdr ( of );

	write ( of, pad, npad );

	/* The ROM image !!! */
	write ( of, buf, romsize );

	/* The gnu signature */
	write ( of, gnu_extra, gnusize );

	/* write string table */
	st_size = mk_string ( of );

	mk_shdr_null ( of );
	mk_shdr_t ( of, t_index );
	mk_shdr_g ( of, g_index );
	mk_shdr_s ( of, s_index, st_size );

	close ( of );
}

void swap_shdr ( int *a, int count )
{
	int i;

	/*
	printf ( "int is %d bytes, %d of em\n", sizeof(int), count );
	*/

	for ( i=0; i<count; i++ ) {
	    /*
	    printf ( "preswap %d: %08x\n", i, a[i] );
	    */
	    a[i] = ntohl ( a[i] );
	    /*
	    printf ( "post    %d: %08x\n", i, a[i] );
	    */
	}
}

void
mk_shdr_null ( int of )
{
	Elf32_Shdr shdr;

	memset ( (char *) &shdr, 0, sizeof(shdr) );

	write ( of, &shdr, sizeof(shdr) );
}

void
mk_shdr_t ( int of, int t_index )
{
	Elf32_Shdr shdr;

	shdr.sh_name = t_index;
	shdr.sh_type = 1;
	shdr.sh_flags = 6;
	shdr.sh_addr = rombase;
	shdr.sh_offset = PGSIZE;
	shdr.sh_size = romsize;
	shdr.sh_link = 0;
	shdr.sh_info = 0;
	shdr.sh_addralign = 4;
	shdr.sh_entsize = 0;

	swap_shdr ( (int *)&shdr, sizeof(shdr) / sizeof(int) );

	write ( of, &shdr, sizeof(shdr) );
}

void
mk_shdr_g ( int of, int index )
{
	Elf32_Shdr shdr;

	shdr.sh_name = index;
	shdr.sh_type = 1;
	shdr.sh_flags = 0x30;
	shdr.sh_addr = 0;
	shdr.sh_offset = PGSIZE + romsize;
	shdr.sh_size = gnusize;
	shdr.sh_link = 0;
	shdr.sh_info = 0;
	shdr.sh_addralign = 1;
	shdr.sh_entsize = 1;

	swap_shdr ( (int *)&shdr, sizeof(shdr) / sizeof(int) );

	write ( of, &shdr, sizeof(shdr) );
}

void
mk_shdr_s ( int of, int s_index, int size )
{
	Elf32_Shdr shdr;
	int n;

	shdr.sh_name = s_index;
	shdr.sh_type = 3;
	shdr.sh_flags = 0;
	shdr.sh_addr = 0;
	shdr.sh_offset = PGSIZE + romsize + gnusize;
	shdr.sh_size = size;
	shdr.sh_link = 0;
	shdr.sh_info = 0;
	shdr.sh_addralign = 1;
	shdr.sh_entsize = 0;

	/*
	printf ( "type: %08x\n", shdr.sh_type );
	printf ( "sizeof name thingie = %d\n", sizeof(shdr.sh_name) );
	*/
	swap_shdr ( (int *)&shdr, sizeof(shdr) / sizeof(int) );
	/*
	printf ( "type: %08x\n", shdr.sh_type );
	printf ( "s type: %08x\n", shdr.sh_type );
	*/

	n = write ( of, &shdr, sizeof(shdr) );
}

int
mk_phdr ( int of )
{
	Elf32_Phdr phdr;

	phdr.p_type = ntohl ( 1 );
	phdr.p_offset = ntohl ( 0x2000 );
	phdr.p_vaddr = ntohl ( rombase );
	phdr.p_paddr = ntohl ( rombase );
	phdr.p_filesz = ntohl ( romsize );
	phdr.p_memsz = ntohl ( romsize );
	phdr.p_flags = ntohl ( 5 );
	phdr.p_align = ntohl ( 0x2000 );

	write ( of, &phdr, sizeof(phdr) );

	return sizeof(phdr);
}

int add_string ( char *s )
{
	int rv = string_off;

	strcpy ( &sbuf[string_off], s );
	string_off += strlen(s) + 1;

	return rv;
}

int mk_string ( int of )
{
	write ( of, sbuf, string_off );
	return string_off;
}

/* THE END */
