#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>
#include <SDL_gfxPrimitives.h>

#define WIDTH 640
#define HEIGHT 480

struct node {
	int x, y;
};

SDL_Surface *screen;

struct node me, you;

void
step (struct node *np)
{
	int r;

	r = rand () % 4000;

	if (r < 1000) {
		np->x += 1;
	} else if (r >= 1000 && r < 2000) {
		np->y += 1;
	} else if (r >= 2000 && r <= 3000) {
		np->x -= 1;
	} else if (r >= 3000 && r <= 4000) {
		np->y -= 1;
	}
}

void
draw (void)
{
	filledCircleColor (screen, me.x, me.y, 5, 0x00ff00ff);
	aacircleColor (screen, me.x, me.y, 5, 0x00ff00ff);

	filledCircleColor (screen, you.x, you.y, 5, 0xff0000ff);
	aacircleColor (screen, you.x, you.y, 5, 0xff0000ff);
}

void
process_input (void)
{
	SDL_Event event;
	int key;

	while (SDL_PollEvent (&event)) {
		key = event.key.keysym.sym;
		switch (event.type) {
		case SDL_QUIT:
			exit (0);
		case SDL_KEYUP:
			if (key == SDLK_ESCAPE || key == 'q') {
				exit (0);
			}
			break;
		}
	}
}

int
main (int argc, char **argv)
{
	srand (time (NULL));

	me.x = WIDTH / 2;
	me.y = HEIGHT / 2;

	you.x = WIDTH / 4;
	you.y = HEIGHT / 4;

	if (SDL_Init (SDL_INIT_VIDEO) != 0) {
		fprintf (stderr, "unable to initialize SDL: %s\n",
			 SDL_GetError ());
		return (1);
	}

	screen = SDL_SetVideoMode (WIDTH, HEIGHT, 32,
				   SDL_HWSURFACE | SDL_DOUBLEBUF);

	while (1) {
		process_input ();
		SDL_FillRect (screen, NULL, 0x000000);
		step (&me);
		step (&you);
		draw ();
		SDL_Flip (screen);
		SDL_Delay (10);
	}

	return (0);
}
