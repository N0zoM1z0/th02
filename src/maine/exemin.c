/* ReC98
 * -----
 * Rewrites a DOS .EXE with the smallest possible header: the relocation table
 * directly follows the 28-byte header, which is padded to a paragraph. This
 * matches the header of TH02's original MAINE.EXE, which was rewritten in
 * this way after TLINK had linked it. The program image and the order of the
 * relocations stay unchanged.
 */

#include <stdio.h>
#include <string.h>

#define HEADER_WORDS 14
#define HEADER_SIZE (HEADER_WORDS * 2)

enum {
	H_LAST_PAGE_SIZE = 1,
	H_PAGES = 2,
	H_RELOC_COUNT = 3,
	H_HEADER_PARAS = 4,
	H_RELOC_OFFSET = 12,
};

static char buf[4096];

static int copy(FILE *fo, FILE *fi, unsigned long len)
{
	while(len > 0) {
		size_t chunk = ((len > sizeof(buf)) ? sizeof(buf) : (size_t)len);
		if(fread(buf, 1, chunk, fi) != chunk) {
			return 1;
		}
		if(fwrite(buf, 1, chunk, fo) != chunk) {
			return 1;
		}
		len -= chunk;
	}
	return 0;
}

int __cdecl main(int argc, char **argv)
{
	unsigned int h[HEADER_WORDS];
	unsigned long header_old;
	unsigned long header_new;
	unsigned long reloc_offset;
	unsigned long image_size;
	unsigned long file_size;
	FILE *fi;
	FILE *fo;

	if(argc != 3) {
		fprintf(stderr, "Usage: %s in.exe out.exe\n", argv[0]);
		return 1;
	}
	fi = fopen(argv[1], "rb");
	if(!fi) {
		fprintf(stderr, "%s: can't open\n", argv[1]);
		return 1;
	}
	if(fread(h, 2, HEADER_WORDS, fi) != HEADER_WORDS) {
		fprintf(stderr, "%s: too short\n", argv[1]);
		return 1;
	}
	fseek(fi, 0, SEEK_END);
	file_size = ftell(fi);
	header_old = ((unsigned long)h[H_HEADER_PARAS] * 16);
	reloc_offset = h[H_RELOC_OFFSET];
	image_size = (file_size - header_old);
	header_new = (
		(HEADER_SIZE + ((unsigned long)h[H_RELOC_COUNT] * 4) + 15) & ~15ul
	);
	file_size = (header_new + image_size);

	h[H_LAST_PAGE_SIZE] = (unsigned int)(file_size % 512);
	h[H_PAGES] = (unsigned int)((file_size + 511) / 512);
	h[H_HEADER_PARAS] = (unsigned int)(header_new / 16);
	h[H_RELOC_OFFSET] = HEADER_SIZE;

	fo = fopen(argv[2], "wb");
	if(!fo) {
		fprintf(stderr, "%s: can't create\n", argv[2]);
		return 1;
	}
	fwrite(h, 2, HEADER_WORDS, fo);
	fseek(fi, reloc_offset, SEEK_SET);
	if(copy(fo, fi, ((unsigned long)h[H_RELOC_COUNT] * 4))) {
		fprintf(stderr, "%s: truncated relocation table\n", argv[1]);
		return 1;
	}
	memset(buf, 0, sizeof(buf));
	fwrite(
		buf,
		1,
		(size_t)(header_new - HEADER_SIZE - ((unsigned long)h[H_RELOC_COUNT] * 4)),
		fo
	);
	fseek(fi, header_old, SEEK_SET);
	if(copy(fo, fi, image_size)) {
		fprintf(stderr, "%s: truncated image\n", argv[1]);
		return 1;
	}
	fclose(fo);
	fclose(fi);
	return 0;
}
