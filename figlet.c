#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "vtclock.h"
#include "font.h"
#include "figlet.h"

#define CHUNK_SIZE 2048

struct xy { 
	int x;
	int y;
};

void
trim (char *out)
{
	size_t l;
	if (!out) return;
	l = strlen(out);
	while (out[--l] == '\r' || out[l] == '\n')
		out[l] = '\0';
}

char *
get_command_output (const char *cmd)
{
	FILE *pipe;
	char *out;
	size_t out_size;
	char buf[CHUNK_SIZE];
	pipe = popen(cmd, "r");
	if (pipe == NULL)
		return NULL;
	out = NULL;
	while (fgets(buf, CHUNK_SIZE, pipe)) {
		if (out == NULL) {
			out = (char *)malloc(CHUNK_SIZE);
			out_size = CHUNK_SIZE;
			out[0] = '\0';
		}
		else if (strlen(out) + strlen(buf) + 1 >= out_size) {
			out = (char *)realloc(out, out_size += CHUNK_SIZE);
		}
		strcat(out, buf);
	}
	pclose(pipe);
	return out;
}

struct xy
get_character_dimensions (const char *figchar)
{
	struct xy dims;
	int x;
	const char *p;

	x = 0;
	dims.x = 0;
	dims.y = 0;
	for (p = figchar; *p; ++p) {
		if (*p == '\r' || *p == '\n') {
			++dims.y;
			if (dims.x < x) dims.x = x;
			x = 0;
		} else {
			++x;
		}
	}
	if (x) {
		++dims.y;
		if (dims.x < x) dims.x = x;
	}
	++dims.x;
	return dims;
}

char *
recenter_glyph (char *glyph, int width)
{
	struct xy dims;
	int shift;
	int pos;
	int i;
	
	dims = get_character_dimensions(glyph);
	shift = (width - dims.x) / 2;

	if (shift == 0)
		return glyph;
	
	glyph = realloc(glyph, strlen(glyph) + shift * dims.y + 1);

	pos = strlen(glyph) - 1;
	while (1) {
		/* move backward to beginning of string or character immediately following \r or \n, whichever comes first. */
		for (; pos && glyph[pos-1] != '\n' && glyph[pos-1] != '\r'; --pos);
		memmove(glyph + pos + shift, glyph + pos, strlen(glyph + pos) + 1);
		for (i = 0; i < shift; ++i)
			glyph[pos + i] = ' ';
		if (pos < 1) break;
		--pos;
		/* skip past \r and \n characters. */
		for (; pos >= 0 && (glyph[pos] == '\n' || glyph[pos] == '\r'); --pos);
		if (pos < 0) break;
	}
	
	return glyph;
}

vtclock_font *
generate_figlet_font (void) 
{
	int debug;
	vtclock_font *font;
	int i, j;
	char *tempstr;
	size_t tempstr_size = 0;
	char *cp;
	FILE *pipe;
	int char_lines;
	int char_width;
	char figlet_line[CHUNK_SIZE];
	char figlet_cmd[256];
	struct xy dims;

	debug = (getenv("VTCLOCK_DEBUG") != NULL);
	
	font = (vtclock_font *)malloc(sizeof(vtclock_font));
	font->digit_height = 0;
	font->digit_width = 0;
	font->colon_width = 0;
	for (i = 0; i < 10; ++i) {
		font->digits[i] = NULL;
	}
	font->colon = NULL;

	for (i = 0; i <= 10; ++i) { /* 0-9 = digits, 10 = colon */
		sprintf(figlet_cmd, "figlet -W '%c'", (i == 10) ? ':' : (i + '0'));
		tempstr = get_command_output(figlet_cmd);
		if (tempstr == NULL) {
			goto err;
		}
		dims = get_character_dimensions(tempstr);
		if (font->digit_height < dims.y) font->digit_height = dims.y;
		if (i == 10) {
			font->colon = tempstr;
			if (font->colon_width < dims.x) font->colon_width = dims.x;
		} else {
			font->digits[i] = tempstr;
			if (font->digit_width < dims.x) font->digit_width = dims.x;
		}
	}

	for (i = 0; i < 10; ++i) {
		font->digits[i] = recenter_glyph(font->digits[i], font->digit_width);
	}
	
	return font;
 err:
	for (j = 0; j < 10; ++j) {
		if (font->digits[i]) free(font->digits[i]);
	}
	free(font->colon);
	free(font);
	return NULL;
}

vtclock_config *
generate_figlet_config (void)
{
	vtclock_font *font;
	vtclock_config *config;
	font = generate_figlet_font();
	if (font == NULL)
		return NULL;
	config = (vtclock_config *)malloc(sizeof(vtclock_config));
	config->hour   = font;
	config->minute = font;
	config->second = font;
	config->colon1 = font;
	config->colon2 = font;
	return config;
}

