#include <jdeflate/deflator.h>
#include <stdio.h>


static uint8 source[4096];
static uint8 target[4096];

bool
compressfile(FILE* ihandle, FILE* ohandle)
{
	uintxx final;
	uintxx result;
	uintxx icount;
	uintxx ocount;
	bool done;
	TDeflator* state;

	state = deflator_create(9, NULL);
	if (state == NULL) {
		puts("Failed to create deflator struct");
		return 0;
	}

	final = done = 0;
	do {
		if (feof(ihandle) == 0) {
			icount = fread(source, 1, sizeof(source), ihandle);
			if (ferror(ihandle))
				goto L_ERROR;
			deflator_setsrc(state, source, icount);
		}
		else {
			final = 1;
		}

		do {
			deflator_settgt(state, target, sizeof(target));
			result = deflator_deflate(state, final);

			if ((ocount = deflator_tgtend(state)) != 0) {
				fwrite(target, 1, ocount, ohandle);
				if (ferror(ohandle))
					goto L_ERROR;
			}
		} while (result == DEFLT_TGTEXHSTD);
	} while (result == DEFLT_SRCEXHSTD);

	if (result == DEFLT_OK)
		done = 1;
L_ERROR:
	deflator_destroy(state);
	return done;
}


#if defined(__MSVC__)
	#pragma warning(disable: 4996)
#endif

int
main(int argc, char* argv[])
{
	FILE* ihandle;
	FILE* ohandle;

	if (argc != 3) {
		puts("Usage: thisprogram <input file> <output>");
		return 0;
	}

	ihandle = fopen(argv[1], "rb");
	ohandle = fopen(argv[2], "wb");
	if (ihandle == NULL || ohandle == NULL) {
		if (ihandle)
			fclose(ihandle);
		if (ohandle)
			fclose(ohandle);
		return 0;
	}
	if (compressfile(ihandle, ohandle)) {
		puts("Done");
	}
	fclose(ihandle);
	fclose(ohandle);
	return 0;
}
