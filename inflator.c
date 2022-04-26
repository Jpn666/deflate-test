#include <inflator.h>
#include <stdio.h>


static uint8 source[4096];
static uint8 target[4096];

bool
decompressfile(FILE* ihandle, FILE* ohandle)
{
	uintxx final;
	uintxx result;
	uintxx icount;
	uintxx ocount;
	bool done;
	TInflator* state;

	state = inflator_create();
	if (state == NULL) {
		return 0;
	}

	final = done = 0;
	do {
		if (feof(ihandle) == 0) {
			icount = fread(source, 1, sizeof(source), ihandle);
			if (ferror(ihandle))
				goto L_ERROR;
			inflator_setsrc(state, source, icount);
		}
		else {
			final = 1;
		}

		do {
			inflator_settgt(state, target, sizeof(target));
			result = inflator_inflate(state, final);

			if ((ocount = inflator_tgtend(state)) != 0) {
				fwrite(target, 1, ocount, ohandle);
				if (ferror(ohandle))
					goto L_ERROR;
			}
		} while (result == INFLT_TGTEXHSTD);
	} while (result == INFLT_SRCEXHSTD);

	if (result == INFLT_OK)
		done = 1;
L_ERROR:
	inflator_destroy(state);
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
	if (decompressfile(ihandle, ohandle)) {
		puts("Done");
	}
	fclose(ihandle);
	fclose(ohandle);
	return 0;
}
