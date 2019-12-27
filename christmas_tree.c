#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include <SDL2/SDL.h>
#define GL_GLEXT_PROTOTYPES 1
#include <SDL2/SDL_opengl.h>

/* Variables related to SDL window and rendering */
static SDL_Window 		*main_window = NULL;
static SDL_Renderer 	*main_renderer = NULL;
static SDL_GLContext 	*main_opengl_context = NULL;

/* Variables related to GPU data */
static GLuint 			 vbo_id_light_point[1] = {0};
static GLuint		     vao_id_light_point[1] = {0};
static GLuint			 vap_id_light_point[1] = {0};

/* The vertex where the light point originates from */
static GLfloat			 light_point[] = {
	0.0f, 0.0f, 0.0f
};

#define ERROR_LOG(...) (fprintf(stderr, __VA_ARGS__))
#define ELEMENTS_IN_ARRAY(_array) (sizeof((_array))/sizeof((_array[0])))

static SDL_Window *
get_window (void)
{
	return main_window;
}

static SDL_Renderer *
get_renderer (void)
{
	return main_renderer;
}

static SDL_GLContext *
get_gl_context (void)
{
	return main_opengl_context;
}

static void
at_exit (void)
{
	if (get_gl_context()) {
		SDL_GL_DeleteContext(get_gl_context());
	}

	if (get_renderer()) {
		SDL_DestroyRenderer(get_renderer());
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

	/* These calls generate the Vertex Array Object (VAO), made up of VBOs */
	glGenVertexArrays(ELEMENTS_IN_ARRAY(vao_id_light_point),
					  vao_id_light_point);
	glBindVertexArray(vao_id_light_point[0]);

	/* These calls generate the vertex attribute pointer data */
	glVertexAttribPointer(vap_id_light_point[0], ELEMENTS_IN_ARRAY(light_point),
						  GL_FLOAT, GL_FALSE /* Normalized data */,
						  0 /* Stride, spacing between vertex attributes */,
						  NULL /* Extra data pointer */);

	glEnableVertexAttribArray(vap_id_light_point[0]);
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
}

static void
clear_window (void)
{
    SDL_SetRenderDrawColor(get_renderer(), 0, 0, 0, 255);
    SDL_RenderClear(get_renderer());
    SDL_GL_SwapWindow(get_window());
    SDL_RenderPresent(get_renderer());
}

static void
render_frame (void)
{

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
		SDL_RenderPresent(get_renderer());
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

	main_renderer = SDL_CreateRenderer(get_window(),
									   -1 /* Use the first one that matches the given flags */,
									   SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (main_renderer == NULL) {
		ERROR_LOG("SDL_CreateRenderer failed: %s\n", SDL_GetError());
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
	allocate_opengl_objects();

	clear_window();
    run_main_event_loop();

	return EXIT_SUCCESS;
}
