#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>

#include <SDL2/SDL.h>
#define GL_GLEXT_PROTOTYPES 1
#include <SDL2/SDL_opengl.h>

#include "shader.h"

/* Variables related to SDL window and rendering */
static SDL_Window 		*main_window = NULL;
static SDL_GLContext 	*main_opengl_context = NULL;

/* Variables related to GPU data */
static GLuint 			 vbo_id_light_point[1] = {0};
static GLuint 			 vao_id_light_point[1] = {0};

/* Using perspective projection */
static GLfloat 			 projection_matrix[4*4] = {0};

/* Just the identity matrix for now. */
static GLfloat 			 modelview_matrix[4*4] = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f,
};

/* The vertex where the light point originates from */
static GLfloat			 light_point[] = {
	0.0f, 0.0f, 1.0f, 1.0f,
	0.5f, 0.0f, 1.0f, 1.0f,
	0.5f, 0.5f, 1.0f, 1.0f,
};

#define ERROR_LOG(...) (fprintf(stderr, __VA_ARGS__))
#define ELEMENTS_IN_ARRAY(_array) (sizeof((_array))/sizeof((_array[0])))

static SDL_Window *
get_window (void)
{
	return main_window;
}

static SDL_GLContext *
get_gl_context (void)
{
	return main_opengl_context;
}

static void
at_exit (void)
{
	deinit_shaders();

	glDeleteBuffers(ELEMENTS_IN_ARRAY(vbo_id_light_point), vbo_id_light_point);

	if (get_gl_context()) {
		SDL_GL_DeleteContext(get_gl_context());
	}

	if (get_window()) {
		SDL_DestroyWindow(get_window());
	}

	SDL_Quit();
}

static void
set_opengl_attributes (void)
{
	/* Taken from http://headerphile.com/sdl2/opengl-part-1-sdl-opengl-awesome/ */
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	/* Synchronize buffer swap with monitor V-Sync */
	SDL_GL_SetSwapInterval(1);
}

static void
allocate_opengl_objects (void)
{
	/* These calls generate the Vertex Buffer Object (VBO) for GPU usage */
	glGenBuffers(ELEMENTS_IN_ARRAY(vbo_id_light_point),
				 vbo_id_light_point);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_id_light_point[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(light_point), light_point, GL_STATIC_DRAW);

	glGenVertexArrays(ELEMENTS_IN_ARRAY(vao_id_light_point),
					  vao_id_light_point);
	glBindVertexArray(vao_id_light_point[0]);

	glEnableClientState(GL_VERTEX_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_id_light_point[0]);
	glVertexAttribPointer(get_vertex_attribute(),
						  4,
					      GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(get_vertex_attribute());

	/* Upload constant data to the shader */
	glUniformMatrix4fv(get_vertex_uniform_projection(),
					   1, GL_FALSE, projection_matrix);
	glUniformMatrix4fv(get_vertex_uniform_modelview(),
					   1, GL_FALSE, modelview_matrix);
}

static void
dump_matrix (GLfloat *matrix)
{
	int i, j;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			printf("%f ", matrix[4*i + j]);
		}
		printf("\n");
	}
	printf("\n");
}

/*
 * https://www.3dgep.com/understanding-the-view-matrix/#The_View_Matrix
 * https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/opengl-perspective-projection-matrix 
 * https://solarianprogrammer.com/2013/05/22/opengl-101-matrices-projection-view-model/
 */


#define FOV_DEGREES 90.0f

static void
generate_projection_matrix(void)
{
	GLfloat near = -100.0f;
	GLfloat far = 100.0f;
	GLfloat scale = tan((M_PI / 180.0f) * FOV_DEGREES * 0.5f) * near;
	GLfloat aspect_ratio = (640.0f / 480.0f);

	GLfloat top = scale;
	GLfloat bottom = -top;
	GLfloat right = top * aspect_ratio;
	GLfloat left = -right;

	GLfloat local_projection_matrix[4*4] = {
		(2 * near) / (right - left), 0.0f, 						  (right + left) / (right - left), 0.0f,
		0.0f, 						 (2 * near) / (top - bottom), (top + bottom) / (top - bottom), 0.0f,
		0.0f, 						 0.0f, 						  (-(far + near) / (far - near)),  ((-2.0f * far * near) / (far - near)),
		0.0f, 						 0.0f, 						  -1.0f, 						   0.0f,
	};

	memcpy(projection_matrix, local_projection_matrix, sizeof(projection_matrix));
}

static void
init_opengl (void)
{
	set_opengl_attributes();

	main_opengl_context = SDL_GL_CreateContext(get_window());
	if (main_opengl_context == NULL) {
		ERROR_LOG("SDL_GL_CreateContext failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	glEnable(GL_PROGRAM_POINT_SIZE);

	generate_projection_matrix();
	dump_matrix(projection_matrix);
}

static void
clear_window (void)
{
	/* Clear both rendering buffers to black */
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapWindow(get_window());

    glClear(GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapWindow(get_window());
}

static void
render_frame (void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	/* Render here */
	glEnableVertexAttribArray(get_vertex_attribute());
	glBindVertexArray(vao_id_light_point[0]);
	glDrawArrays(GL_TRIANGLES, 0, ELEMENTS_IN_ARRAY(light_point)/4);

	glDisableVertexAttribArray(get_vertex_attribute());
}

static void
run_main_event_loop (void)
{
	bool loop = true;

	printf("Entering main loop\n");

	while (loop) {
		SDL_Event event;

		/* Process incoming events */
		if (SDL_WaitEvent(&event) != 0) {
			if (event.type == SDL_QUIT) {
				loop = false;
			}
		}

		/* Render Frame */
		render_frame();

		/* Move the rendered buffer to the screen */
		SDL_GL_SwapWindow(get_window());
	}

	printf("Exiting...\n");
}

static void
init_sdl (void)
{
	int rc = 0;

	rc = SDL_Init(SDL_INIT_VIDEO);
	if (rc != 0) {
		ERROR_LOG("SDL_Init failed (%u): %s\n", rc, SDL_GetError());
		exit(rc);
	}

	main_window = SDL_CreateWindow("Christmas Tree", 100, 100, 640, 480,
								   SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	if (main_window == NULL) {
		ERROR_LOG("SDL_CreateWindow failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
}

int
main (int argc, char *argv[])
{
	/* Setup program exit cleanup routines */
	atexit(at_exit);

	init_sdl();
	init_opengl();
	initialize_shaders();
	allocate_opengl_objects();

	clear_window();
    run_main_event_loop();

	return EXIT_SUCCESS;
}
