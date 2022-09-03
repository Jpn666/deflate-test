#include <stdio.h>
#include <stdlib.h>
#include <zstrm.h>


uint8 iobuffer[4096];

intxx
rcallback(uint8* buffer, uintxx size, void* user)
{
	uintxx r;

	r = fread(buffer, 1, size, (FILE*) user);
	if (r != size) {
		if (ferror((FILE*) user)) {
			return -1;
		}
	}

	return r;
}

intxx
wcallback(uint8* buffer, uintxx size, void* user)
{
	uintxx r;

	r = fwrite(buffer, 1, size, (FILE*) user);
	if (r != size) {
		if (ferror((FILE*) user)) {
			return -1;
		}
	}
	return r;
}


bool
inflate(TZStrm* z, FILE* source, FILE* target)
{
	uintxx r;

	zstrm_setiofn(z, rcallback, source);
	do {
		r = zstrm_r(z, iobuffer, sizeof(iobuffer));

		if (fwrite(iobuffer, 1, r, target) != r || ferror(target)) {
			puts("Error: IO error while writing file");
			return 0;
		}
	} while (z->state != ZSTRM_END);

	if (z->error) {
		puts("Error: Zstream error");
		return 0;
	}

	fflush(target);
	if (ferror(target)) {
		puts("Error: IO error");
		return 0;
	}
	return 1;
}

bool
deflate(TZStrm* z, FILE* source, FILE* target)
{
	uintxx r;

	zstrm_setiofn(z, wcallback, target);
	do {
		r = fread(iobuffer, 1, sizeof(iobuffer), source);
		if (ferror(source)) {
			puts("Error: IO error while reading file");
			return 0;
		}

		zstrm_w(z, iobuffer, r);
		if (z->error) {
			puts("Error: Zstream error");
			return 0;
		}
	} while (feof(source) == 0);
	zstrm_flush(z, 1);

	if (z->error) {
		puts("Error: Zstream error");
		return 0;
	}

	fflush(target);
	if (ferror(target)) {
		puts("Error: IO error");
		return 0;
	}
	return 1;
}

void
showusage(void)
{
	puts("Usage:");
	puts("thisprogram <0...9> <input file> <compressed file>");
	puts("thisprogram <compressed file> <output file>");
	exit(0);
}


#if defined(__MSVC__)
	#pragma warning(disable: 4996)
#endif


static void*
reserve(void* user, uintxx amount)
{
	(void) user;
	return malloc(amount);
}

static void
release(void* user, void* memory)
{
	(void) user;
	free(memory);
}

int
main(int argc, char* argv[])
{
	intxx level;
	char* lvend;
	FILE* source;
	FILE* target;
	TZStrm* z;
	TAllocator allocator[1];

	if (argc != 3 && argc != 4) {
		showusage();
	}

	allocator[0].reserve = reserve;
	allocator[0].release = release;

	level = 0;
	if (argc == 4) {
		lvend = argv[1];
		level = strtoll(argv[1], &lvend, 0);
		if (lvend == argv[1] || level < 0 || level > 9) {
			puts("Error: Invalid compression level...");
			showusage();
		}

		source = fopen(argv[2], "rb");
		target = fopen(argv[3], "wb");
		z = zstrm_create(ZSTRM_WMODE | ZSTRM_GZIP, level, allocator);
	}
	else {
		source = fopen(argv[1], "rb");
		target = fopen(argv[2], "wb");
		/* level is ignored here */
		z = zstrm_create(ZSTRM_RMODE | ZSTRM_AUTO, level, allocator);
	}

	if (z == NULL) {
		puts("Error: Failed to create Zstream struct");
		goto L_ERROR;
	}
	if (source == NULL || target == NULL) {
		puts("Error: Failed to open or create file");
		goto L_ERROR;
	}

	if (argc == 4) {
		deflate(z, source, target);
	}
	else {
		inflate(z, source, target);
	}

L_ERROR:
	if (source) fclose(source);
	if (target) fclose(target);
	if (z)
		zstrm_destroy(z);
	return 0;
}
