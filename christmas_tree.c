#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>

#include <SDL2/SDL.h>
#define GL_GLEXT_PROTOTYPES 1
#include <SDL2/SDL_opengl.h>

#include <cglm/cglm.h>
#include <cglm/io.h>

#include "shader.h"
#include "spiral.h"

/* Variables related to SDL window and rendering */
static SDL_Window 		*main_window = NULL;
static SDL_GLContext 	main_opengl_context = NULL;

static mat4 projection_matrix;
static mat4 view_matrix;

#define ERROR_LOG(...) (fprintf(stderr, __VA_ARGS__))
#define ELEMENTS_IN_ARRAY(_array) (sizeof((_array))/sizeof((_array[0])))

#define WINDOW_WIDTH 1280.0f
#define WINDOW_HEIGHT 800.0f

#define NUM_SPIRALS		4
static spiral rendered_spirals[NUM_SPIRALS];

static SDL_Window *
get_window (void)
{
	return main_window;
}

static SDL_GLContext
get_gl_context (void)
{
	return main_opengl_context;
}

static void
free_spirals(void)
{
	size_t i;

	for (i = 0; i < ELEMENTS_IN_ARRAY(rendered_spirals); i++) {
		spiral_free(rendered_spirals[i]);
	}
}

static void
init_spirals(void)
{
	size_t i;
	spiral_init_ctx init_ctx = {
		.num_slices = 400,
		.num_rotations = 6,
		.cycle_time_ms = 100000,
		.y_max = 2.0f,
		.slope = -3.0f,
		.starting_angle_offset = 0.0f,
	};

	vec4 colors[ELEMENTS_IN_ARRAY(rendered_spirals)] = {
		{0.0f, 0.5f, 0.4f, 1.0f},
		{0.0f, 1.0f, 0.8f, 1.0f},
		{0.535f, 0.1f, 0.1f, 1.0f},
		{1.0f, 0.2f, 0.2f, 1.0f}
	};

	for (i = 0; i < ELEMENTS_IN_ARRAY(rendered_spirals); i += 2) {
		memcpy(init_ctx.color, colors[i], sizeof(init_ctx.color));
		rendered_spirals[i] = spiral_init(&init_ctx);

		init_ctx.slope += 0.1f;
		memcpy(init_ctx.color, colors[i+1], sizeof(init_ctx.color));
		rendered_spirals[i+1] = spiral_init(&init_ctx);

		init_ctx.starting_angle_offset += GLM_PI;
	}
}

static void
at_exit (void)
{
	if (get_window()) {
		deinit_shaders();

		free_spirals();

		if (get_gl_context()) {
			SDL_GL_DeleteContext(get_gl_context());
		}

		SDL_DestroyWindow(get_window());
		main_window = NULL;
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
upload_shader_constants (void)
{
	glUniformMatrix4fv(get_vertex_uniform_projection(),
					   1, GL_FALSE, (GLfloat *)projection_matrix);
	glUniformMatrix4fv(get_vertex_uniform_modelview(),
					   1, GL_FALSE, (GLfloat *)view_matrix);
}

/*
 * https://www.3dgep.com/understanding-the-view-matrix/#The_View_Matrix
 * https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/opengl-perspective-projection-matrix 
 * https://solarianprogrammer.com/2013/05/22/opengl-101-matrices-projection-view-model/
 * https://www.opengl-tutorial.org/beginners-tutorials/tutorial-3-matrices/#transformation-matrices
 */
static void
generate_projection_matrix(void)
{
	/* Default perspective in 3D space is that the camera is
	 * looking down the Z-Axis (-Z is further into the screen).
	 * 
	 * glm_lookat takes the following vectors: Eye, Center, Up
	 */
	glm_lookat((vec3){0.0f, 1.0f, 3.0f}, (vec3){0, 0.92f, 0}, (vec3){0, 1.0f, 0}, view_matrix);
	glm_perspective_default((WINDOW_WIDTH/WINDOW_HEIGHT),  projection_matrix);
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
	glEnable(GL_POINT_SMOOTH);

	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
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
update_frame (uint32_t delta_ms)
{
	size_t i;

	for (i = 0; i < ELEMENTS_IN_ARRAY(rendered_spirals); i++) {
		spiral_update(rendered_spirals[i], delta_ms);
	}
}

static void
render_frame (void)
{
	size_t i;

	glClear(GL_COLOR_BUFFER_BIT);

	for (i = 0; i < ELEMENTS_IN_ARRAY(rendered_spirals); i++) {
		spiral_render(rendered_spirals[i]);
	}
}

static void
run_main_event_loop (void)
{
	bool loop = true;
	uint32_t frame_start_ticks = 0;
	uint32_t frame_end_ticks = 0;
	uint32_t frame_delta_ticks = 0;

	printf("Entering main loop\n");

	while (loop) {
		SDL_Event event;

		/* Bump the delta in case the framerate is too fast */
		if (frame_delta_ticks == 0) {
			frame_delta_ticks = 1;
		}

		printf("FPS: %3.3f\r", 1/(frame_delta_ticks*0.001f));

		frame_start_ticks = SDL_GetTicks();

		/* Process incoming events.
		 * NOTE: This will chew up 100% CPU.
		 * Would be nice to have a better way to wait between drawing frames */
		if (SDL_PollEvent(&event) != 0) {
			if (event.type == SDL_QUIT) {
				loop = false;
			}
		}

		update_frame(frame_delta_ticks);
		render_frame();

		/* Move the rendered buffer to the screen */
		SDL_GL_SwapWindow(get_window());

		frame_end_ticks = SDL_GetTicks();
		frame_delta_ticks = frame_end_ticks - frame_start_ticks;
	}

	printf("\nExiting...\n");
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

	main_window = SDL_CreateWindow("Christmas Tree",
								   SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
								   WINDOW_WIDTH, WINDOW_HEIGHT,
								   SDL_WINDOW_OPENGL);
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
	generate_projection_matrix();
	initialize_shaders();
	init_spirals();
	upload_shader_constants();

	clear_window();
    run_main_event_loop();

    /* On Arch Linux, calling all the cleanup via at_exit was causing a
     * double free due to GPU driver cleanup occurring before the
     * registered at_exit. at_exit is called in the success path to fix this.
     */
    at_exit();

	return EXIT_SUCCESS;
}
