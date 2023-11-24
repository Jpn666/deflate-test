#include <jdeflate/zstrm.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


uint8 iobuffer[4096];


static intxx
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

static intxx
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


static bool
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
		puts("Error: ZStream error");
		return 0;
	}

	fflush(target);
	if (ferror(target)) {
		puts("Error: IO error");
		return 0;
	}
	return 1;
}

static bool
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
			puts("Error: ZStream error");
			return 0;
		}
	} while (feof(source) == 0);
	zstrm_flush(z, 1);

	if (z->error) {
		puts("Error: ZStream error");
		return 0;
	}

	fflush(target);
	if (ferror(target)) {
		puts("Error: IO error");
		return 0;
	}
	return 1;
}

static void
showusage(void)
{
	puts("Usage:");
	puts("thisprogram [options] <input> <output>");
	puts("");
	puts("To compress a file:");
	puts("thisprogram -f <format> -<compression level: 0-9> <input> <output>");
	puts("");
	puts("To decompress a file:");
	puts("thisprogram <input> <output>");
	puts("");
	puts("Valid formats are:");
	puts("    - deflate");
	puts("    - gzip");
	puts("    - zlib");
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


struct TArguments {
	uintxx format;
	uintxx level;
};

static bool
parsearguments(char* argv[], struct TArguments result[1])
{
	uintxx j;
	bool hasformat;
	bool haslevel;

	argv++;
	hasformat = 0;
	haslevel  = 0;
	for (j = 0; j < 2; j++) {
		if (argv[0][0] ^ '-') {
			goto L_ERROR;
		}

		if (argv[0][1] == 'f') {
			if (hasformat)
				goto L_ERROR;
			result[0].format = -1;
			if (strcmp(argv[1],    "gzip") == 0) result[0].format = ZSTRM_GZIP;
			if (strcmp(argv[1],    "zlib") == 0) result[0].format = ZSTRM_ZLIB;
			if (strcmp(argv[1], "deflate") == 0) result[0].format = ZSTRM_DFLT;
			if (result[0].format == -1) {
				goto L_ERROR;
			}
			argv++;
			argv++;
			hasformat = 1;
		}
		else {
			if (argv[0][1] >= 0x30 && argv[0][1] <= 0x39) {
				if (haslevel)
					goto L_ERROR;
				result[0].level = argv[0][1] - 0x30;
				argv++;
				haslevel = 1;
			}
			else {
				goto L_ERROR;
			}
		}
	}
	return 1;

L_ERROR:
	puts("Error: Invalid argument");
	return 0;
}

int
main(int argc, char* argv[])
{
	struct TArguments a[1];
	intxx level;
	FILE* source;
	FILE* target;
	TZStrm* z;
	TAllocator allocator[1];

	if (argc ^ 3 && argc ^ 6) {
		showusage();
		return 0;
	}

	if (argc == 6) {
		if (parsearguments(argv, a) == 0) {
			showusage();
			return 0;
		}
	}

	allocator[0].reserve = reserve;
	allocator[0].release = release;

	level = 0;
	if (argc == 6) {
		source = fopen(argv[4], "rb");
		target = fopen(argv[5], "wb");
		z = zstrm_create(ZSTRM_WMODE | a[0].format, a[0].level, allocator);
	}
	else {
		source = fopen(argv[1], "rb");
		target = fopen(argv[2], "wb");
		/* level is ignored here */
		z = zstrm_create(ZSTRM_RMODE | ZSTRM_AUTO, level, allocator);
	}

	if (z == NULL) {
		puts("Error: Failed to create ZStream struct");
		goto L_ERROR;
	}
	if (source == NULL || target == NULL) {
		puts("Error: Failed to open or create file");
		goto L_ERROR;
	}

	if (argc == 6) {
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
