#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qdbmp.h"
#include "tile.h"

#define FILENAME_LEN (64)

int tile_read(BMP *image, uint8_t *buffer, uint8_t *ref_buf, int start_x, int start_y) {
	uint8_t value;
	int palette = -1;
	for (int y = 0; y < 8; y++) {
		uint8_t plane0 = 0;
		uint8_t plane1 = 0;
		uint8_t plane2 = 0;
		uint8_t plane3 = 0;
		for (int x = 0; x < 8; x++) {
			BMP_GetPixelIndex(image, x + start_x, y + start_y, &value);
			if (palette == -1) {
				palette = value >> 4;
			}
			else if ((value >> 4) != palette) {
				printf("Error: Tile ranges across multiple palettes\n");
				return 0;
			}
			
			if (value & 1) {
				plane0 |= 1;
			}
			if (value & 2) {
				plane1 |= 1;
			}
			if (value & 4) {
				plane2 |= 1;
			}
			if (value & 8) {
				plane3 |= 1;
			}
			plane0 <<= 1;
			plane1 <<= 1;
			plane2 <<= 1;
			plane3 <<= 1;
		}
		// the bitplanes are interleaved such that bits 1 & 2 are together
		// but bits 3 & 4 are after them
		buffer[y * 2] = plane0;
		buffer[y * 2 + 1] = plane1;
		buffer[(y + 8) * 2] = plane2;
		buffer[(y + 8) * 2 + 1] = plane3;
	}
	*ref_buf = (uint8_t)palette;
	return 1;
}

int tile_process(char *file) {
	char in_filename[FILENAME_LEN];
	char tle_filename[FILENAME_LEN];
	char pal_filename[FILENAME_LEN];
	char ref_filename[FILENAME_LEN];
	
	snprintf(in_filename, FILENAME_LEN, "%s.bmp", file);
	snprintf(tle_filename, FILENAME_LEN, "%s.tle", file);
	snprintf(pal_filename, FILENAME_LEN, "%s.pal", file);
	snprintf(ref_filename, FILENAME_LEN, "%s.ref", file);

	BMP *infile = BMP_ReadFile(in_filename);
	BMP_CHECK_ERROR(stdout, 0);
	int width = BMP_GetWidth(infile);
	int height = BMP_GetHeight(infile);
	uint8_t bpp = BMP_GetDepth(infile);
	
	if (bpp != 8) {
		printf("Error: Source image not 8bpp indexed!\n");
		BMP_Free(infile);
		return 0;
	}
	
	if ((width % 8 != 0) || (height % 8 != 0)) {
		printf("Error: Image width %d or height %d not a multiple of 8.\n", width, height);
		BMP_Free(infile);
		return 0;
	}
	uint16_t *palette = malloc(256 * sizeof(uint16_t));
	uint16_t color;
	uint8_t r, g, b;
	uint8_t palette_count = 0;
	for (int i = 0; i < 16; i++) {
		int all_black = 1;
		for (int j = 0; j < 16; j++) {
			BMP_GetPaletteColor(infile, (i * 16) + j, &r, &g, &b);
			// pc engine color format: 0000000gggrrrbbb
			r >>= 5;
			g >>= 5;
			b >>= 5;
			color = (g << 6) | (r << 3) | b;
			if (color != 0) {
				all_black = 0;
			}
			palette[(i * 16) + j] = color;
		}
		// only save the first n palettes with data
		if (all_black) {
			break;
		}
		palette_count++;
	}
	
	uint16_t image_size = (width / 2) * height;
	uint8_t *image_data = malloc(image_size);
	// keeps track of which tile has which palette
	uint8_t *ref_buf = malloc((width / 8) * (height / 8));
	uint8_t *ref_curs = ref_buf;
	
	int image_cursor = 0;
	uint8_t single_tile[32];
	for (int y = 0; y < height; y += 8) {
		for (int x = 0; x < width; x += 8) {
			if (!tile_read(infile, single_tile, ref_curs, x, y)) {
				goto err_exit;
			}
			ref_curs++;
			for (int i = 0; i < sizeof(single_tile); i++) {
				image_data[image_cursor + i] = single_tile[i];
			}
			image_cursor += sizeof(single_tile);
		}
	}
		
	FILE *tlefile = fopen(tle_filename, "wb");
	if (!tlefile) {
		printf("Error: Couldn't open %s for writing!\n", tle_filename);
		goto err_exit;
	}
	fwrite(image_data, sizeof(uint8_t), image_size, tlefile);
	fclose(tlefile);
	
	FILE *palfile = fopen(pal_filename, "wb");
	if (!palfile) {
		printf("Error: Couldn't open %s for writing!\n", pal_filename);
		goto err_exit;
	}
	
	fwrite(palette, sizeof(uint16_t), 16 * palette_count, palfile);
	fclose(palfile);
	
	FILE *reffile = fopen(ref_filename, "wb");
	if (!palfile) {
		printf("Error: Couldn't open %s for writing!\n", ref_filename);
		goto err_exit;
	}
	
	fwrite(ref_buf, sizeof(uint8_t), (width / 8) * (height / 8), reffile);
	fclose(reffile);
	
	free(ref_buf);
	free(palette);
	free(image_data);
	BMP_Free(infile);
	return 1;
	
err_exit:
	free(ref_buf);
	free(palette);
	free(image_data);
	BMP_Free(infile);
	return 0;
}