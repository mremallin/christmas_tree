#include <stdio.h>

#include <SDL2/SDL.h>

int
main (int argc, char *argv[])
{
	int rc = 0;

	rc = SDL_Init(SDL_INIT_VIDEO);
	if (rc != 0) {
		fprintf(stderr, "SDL_Init failed (%u): %s\n", rc, SDL_GetError());
		exit(rc);
	}

	printf("Christmas tree goes here!\n");

	SDL_Quit();
	return 0;
}
