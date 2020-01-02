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

/* Variables related to SDL window and rendering */
static SDL_Window 		*main_window = NULL;
static SDL_GLContext 	*main_opengl_context = NULL;

/* Variables related to GPU data */
static GLuint 			 vbo_id_light_point[1] = {0};
static GLuint 			 vao_id_light_point[1] = {0};

static mat4 projection_matrix;
static mat4 view_matrix;

/* The vertex where the light point originates from */
#define NUM_SLICES 		(50)
/* Where to stop the tree at the top */
#define Y_MAX			(2.0f)
/* Number of rotations around the tree before hitting y-max */
#define NUM_ROTATIONS 	(4)
/* Time taken for a single point to go from bottom of the tree
 * to the top */
#define Y_TIME_MS		(10000)
/* delta y per unit time */
#define Y_SPEED			((Y_MAX) / Y_TIME_MS)

static vec4 *light_points;

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

	if (light_points) {
		free(light_points);
	}

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
	glBufferData(GL_ARRAY_BUFFER, sizeof(light_points[0]) * NUM_SLICES, light_points, GL_DYNAMIC_DRAW);

	glGenVertexArrays(ELEMENTS_IN_ARRAY(vao_id_light_point),
					  vao_id_light_point);
	glBindVertexArray(vao_id_light_point[0]);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_id_light_point[0]);
	glVertexAttribPointer(get_vertex_attribute(),
						  4,
					      GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(get_vertex_attribute());

	/* Upload constant data to the shader */
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
	 * looking down the Z-Axis (-Z is further into the screen)
	 */
	glm_lookat((vec3){0.0f, 2.0f, 5.0f}, (vec3){0, 1.0f, 0}, (vec3){0, 1.0f, 0}, view_matrix);
	glm_perspective_default((640.0f/480.0f),  projection_matrix);
}

/* This represents the bounding cone for the tree */
static float
get_r_for_point (float y)
{
	/* ( y - b ) / m = r */
	return (y - 2) / -2.0f;
}

static void
update_point (vec4 point)
{
	float r_clamp = get_r_for_point(point[1]);

	/* Each one also gets rotated around the trunk.
	 * 3 rotations from start to finish should make a decent tree.
	 * The radius calculated above is the distance from the trunk
	 * the point must be. Need to turn the radius into an (x, z)
	 * coordinate to have it around the tree. Starting from
	 * y=0, want the spiral to also start from x=1 -> cos(3x) to
	 * know the x-component of the vertex.
	 */
	point[0] = r_clamp * cosf((NUM_ROTATIONS * 2.0f * point[1]));
	point[2] = r_clamp * sinf((NUM_ROTATIONS * 2.0f * point[1]) + GLM_PI);
}

static void
generate_points(void)
{
	size_t i;

	light_points = malloc(sizeof(light_points[0]) * NUM_SLICES);
	if (light_points == NULL) {
		ERROR_LOG("Failed to allocate space for points\n");
		exit(EXIT_FAILURE);
	}

	/* Initialize each poin making up the spiral */
	for (i = 0; i < NUM_SLICES; i++) {
		/* Start each one at a given y-value */
		light_points[i][1] = (Y_MAX/NUM_SLICES) * i;
		light_points[i][3] = 1.0f;

		update_point(light_points[i]);
	}
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
	/* To render the christmas tree:
	 * 1) Slice up the y-axis into discrete steps. Each one has a single point
	 *	  in it's plane (or n points eqidistant from each other)
	 * 2) Based on which slice this is, that determines how far away (r)
	 *		from the y-axis the point is (r = (y-b)/m) to find the bounding cone
	 *      of the spiral.
	 *    Each slice also is at a different position on the circle from the
	 *      ones surrounding it.
	 * 3) Each time step, update the points to move along the y-axis. Once
	 *      a given point has surpassed a y-max, reset it to y=0 and continue.
	 */
	size_t i;

	for (i = 0; i < NUM_SLICES; i++) {
		light_points[i][1] += Y_SPEED * delta_ms;
		if (light_points[i][1] > Y_MAX) {
			light_points[i][1] = 0.0f;
		}
		update_point(light_points[i]);
	}
}

static void
render_frame (void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	/* Update the GPU with latest vertex data */
	glBindVertexArray(vao_id_light_point[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(light_points[0]) * NUM_SLICES, light_points, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(get_vertex_attribute());
	glDrawArrays(GL_POINTS, 0, NUM_SLICES);

	glDisableVertexAttribArray(get_vertex_attribute());
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

		frame_start_ticks = SDL_GetTicks();

		/* Process incoming events.
		 * NOTE: This will chew up 100% CPU, need to find a better way
		 * in order to not be busy while waiting to update. */
		if (SDL_PollEvent(&event) != 0) {
			if (event.type == SDL_QUIT) {
				loop = false;
			}
		}

		/* Render Frame */
		update_frame(frame_delta_ticks);
		render_frame();

		/* Move the rendered buffer to the screen */
		SDL_GL_SwapWindow(get_window());

		frame_end_ticks = SDL_GetTicks();
		frame_delta_ticks = frame_end_ticks - frame_start_ticks;
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
	generate_projection_matrix();
	generate_points();
	initialize_shaders();
	allocate_opengl_objects();

	clear_window();
    run_main_event_loop();

	return EXIT_SUCCESS;
}
