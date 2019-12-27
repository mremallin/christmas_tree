#include "shader.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static char *vertex_shader_source;
static char *fragment_shader_source;

static char *
file_to_buf (const char *file_name)
{
	FILE *f;
	size_t length;
	char *buffer;

	f = fopen(file_name, "rb");
	if (f == NULL) {
		return NULL;
	}

	/* Grab length of the file */
	fseek(f, 0, SEEK_END);
	length = ftell(f);
	fseek(f, 0, SEEK_SET);

	/* Allocate new buffer and read */
	buffer = malloc(length + 1);
	assert(buffer);

	fread(buffer, length, 1, f);
	fclose(f);
	buffer[length] = '\0';

	return buffer;
}

void
initialize_shaders (void)
{
	vertex_shader_source = file_to_buf("christmas_tree.vert");
	if (vertex_shader_source == NULL) {
		fprintf(stderr, "Failed to load vertex shader\n");
		exit(EXIT_FAILURE);
	}

	fragment_shader_source = file_to_buf("christmas_tree.frag");
	if (fragment_shader_source == NULL) {
		fprintf(stderr, "Failed to load fragment shader\n");
		exit(EXIT_FAILURE);
	}
}

void
deinit_shaders (void)
{
	if (fragment_shader_source) {
		free(fragment_shader_source);
	}

	if (vertex_shader_source) {
		free(vertex_shader_source);
	}
}
