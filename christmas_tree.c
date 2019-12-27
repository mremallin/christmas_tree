#include <stdio.h>

#include <SDL2/SDL.h>

static SDL_Window 		*main_window = NULL;
static SDL_Renderer 	*main_renderer = NULL;
static SDL_GLContext 	*main_opengl_context = NULL;

static SDL_Window *
get_window(void)
{
	return main_window;
}

static SDL_Renderer *
get_renderer(void)
{
	return main_renderer;
}

static void
set_opengl_attributes(void)
{
	/* Taken from http://headerphile.com/sdl2/opengl-part-1-sdl-opengl-awesome/ */
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
}

int
main (int argc, char *argv[])
{
	int rc = 0;

	rc = SDL_Init(SDL_INIT_VIDEO);
	if (rc != 0) {
		fprintf(stderr, "SDL_Init failed (%u): %s\n", rc, SDL_GetError());
		exit(rc);
	}

	main_window = SDL_CreateWindow("Christmas Tree", 100, 100, 640, 480,
								   SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	if (main_window == NULL) {
		fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	main_renderer = SDL_CreateRenderer(get_window(),
									   -1 /* Use the first one that matches the given flags */,
									   SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (main_renderer == NULL) {
		fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
		SDL_DestroyWindow(get_window());
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	set_opengl_attributes();

	main_opengl_context = SDL_GL_CreateContext(get_window());
	if (main_opengl_context == NULL) {
		fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
		SDL_DestroyRenderer(get_renderer());
		SDL_DestroyWindow(get_window());
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	printf("Christmas tree goes here!\n");
	SDL_Delay(5000);

	SDL_GL_DeleteContext(main_opengl_context);
	SDL_DestroyRenderer(get_renderer());
	SDL_DestroyWindow(get_window());
	SDL_Quit();
	return 0;
}
