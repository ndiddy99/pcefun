#include <stdio.h>
#include <string.h>

#include "debug.h"
// #include "sprite.h"
#include "tile.h"

#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define BUFFER_LEN (256)
#define ASCII_NUMBER_BASE (0x30)

int main(int argc, char **argv) {
	if (argc < 2) {
		printf("Usage: pceconv [asset list]\n");
		return -1;
	}
	
	FILE *asset_list = fopen(argv[1], "r");
	if (asset_list == NULL) {
		printf("Error: couldn't open asset list.\n");
		return -2;
	}
	
	char line[BUFFER_LEN];
	while (fgets(line, BUFFER_LEN - 1, asset_list)) {
		line[strcspn(line, "\r\n")] = 0;
		DBG_PRINTF("%s\n", line);
		char *infile;
		switch(line[0]) {
			// sprite directory
			// case 's':
				// infile = &line[2];
				// if (snprintf(outfile, OUTFILE_LEN, "%s.spr", infile) > OUTFILE_LEN) {
					// printf("Error: overran output file buffer, increase OUTFILE_LEN define in pceconv.c\n");
					// fclose(asset_list);
					// return -3;
				// }
				// else {
					// sprite_process(infile, outfile);
				// }
				// break;
			
			// tile graphics
			case 't':
				infile = &line[2];
				tile_process(infile);
				break;
			
			default:
				printf("Error: Invalid asset list format.\n");
				fclose(asset_list);
				return -4;
				
		}
	}
	
	fclose(asset_list);
	return 0;
}