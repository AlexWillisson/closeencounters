#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>
#include <SDL_gfxPrimitives.h>

#define WIDTH 640
#define HEIGHT 480

#define BASEWEIGHT 1000

struct vect {
	int x, y;
	struct vect *next;
};

struct node {
	int x, y;
	struct vect *head_vect;
};

SDL_Surface *screen;

struct node me, you;

void *xcalloc (unsigned int a, unsigned int b);
char *xstrdup (const char *old);
void step (struct node *np);
void draw (void);
void process_input (void);
void add_weight (struct vect *vp, struct node *np);
void init_node (struct node *np, int x, int y);

void *
xcalloc (unsigned int a, unsigned int b)
{
	void *p;

	if ((p = calloc (a, b)) == NULL) {
		fprintf (stderr, "memory error\n");
		exit (1);
	}

	return (p);
}

char *
xstrdup (const char *old)
{
	char *new;

	if ((new = strdup (old)) == NULL) {
		fprintf (stderr, "out of memory\n");
		exit (1);
	}

	return (new);
}

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

void
add_weight (struct vect *vp0, struct node *np)
{
	struct vect *vp1;

	vp1 = xcalloc (1, sizeof *vp1);

	vp1->x = vp0->x;
	vp1->y = vp0->y;

	if (np->head_vect) {
		vp1->next = np->head_vect;
		np->head_vect = vp1;
	} else {
		np->head_vect = vp1;
	}
}

void
init_node (struct node *np, int x, int y)
{
	np->x = x;
	np->y = y;
}

int
main (int argc, char **argv)
{
	struct vect dirs[4];

	srand (time (NULL));

	dirs[0].x = 1000;
	dirs[0].y = 0;
	dirs[1].x = 0;
	dirs[1].y = -1000;
	dirs[2].x = -1000;
	dirs[2].y = 0;
	dirs[3].x = 0;
	dirs[3].y = 1000;

	init_node (&me, WIDTH / 2, HEIGHT / 2);
	add_weight (&dirs[0], &me);
	add_weight (&dirs[1], &me);
	add_weight (&dirs[2], &me);
	add_weight (&dirs[3], &me);

	init_node (&you, WIDTH / 4, HEIGHT / 4);
	add_weight (&dirs[0], &you);
	add_weight (&dirs[1], &you);
	add_weight (&dirs[2], &you);
	add_weight (&dirs[3], &you);

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
