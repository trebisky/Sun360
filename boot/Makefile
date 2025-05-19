# disassemble Sun 3 second stage boot

# I add the -z options because I would
# rather see blocks of zeros verbatim
# than the compression to "..."
#DUMP = m68k-linux-gnu-objdump -m68030 -z -d
DUMP = m68k-linux-gnu-objdump -m68020 -z -d

all:	wrap

dis: boot.dis boot.odx

wrap: wrap.c
	cc -o wrap wrap.c

dump32: dump32.c
	cc -o dump32 dump32.c

# --------------------

xstrings: xstrings.c
	cc -o xstrings xstrings.c

boot.elf: boot.bin
	../wrap boot.bin boot.elf 04000

boot.dis: boot.elf
	$(DUMP) boot.elf >boot.dis

boot.odx: boot.bin
	odx boot.bin >boot.odx

# ============================================================

clean:
	rm -f *.elf
	rm -f *.dis
	rm -f *.odx
