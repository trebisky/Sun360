# disassemble Sun 3/80 boot rom

# I add the -z options because I would
# rather see blocks of zeros verbatim
# than the compression to "..."
#DUMP = m68k-linux-gnu-objdump -m68030 -z -d
DUMP = m68k-linux-gnu-objdump -m68020 -z -d

all:	wrap dump32 xstrings

dis: sun360.dis sun360.odx

wrap: wrap.c
	cc -o wrap wrap.c

dump32: dump32.c
	cc -o dump32 dump32.c

# --------------------

xstrings: xstrings.c
	cc -o xstrings xstrings.c

sun360.elf: sun360.bin
	./wrap sun360.bin sun360.elf 0fef0000

sun360.dis: sun360.elf
	$(DUMP) sun360.elf >sun360.dis

sun360.odx: sun360.bin
	odx sun360.bin >sun360.odx

# I did this needlessly to compare the the disassembly from
# Dan's image, then discovered that the images are identical
alt.elf: Roms/sun3_60_v1.9.bin
	./wrap Roms/sun3_60_v1.9.bin alt.elf 0fef0000

alt.dis: alt.elf
	$(DUMP) alt.elf >alt.dis

fix: sun360.elf
	$(DUMP) --start-address=0xfefe2b04 --stop-address=0xfefe2b10 sun360.elf

strings: xstrings
	./xstrings >zzz
	./xstrings

# ============================================================

clean:
	rm -f wrap
	rm -f *.elf
	rm -f *.dis
	rm -f *.odx
