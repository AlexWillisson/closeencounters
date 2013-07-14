 #include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <SDL.h>
#include <SDL_gfxPrimitives.h>

#define WIDTH 640
#define HEIGHT 480

#define BASEWEIGHT 1000
#define STEPSIZE 5

struct vect {
	double x, y;
};

struct weight {
	struct weight *next;
	struct vect v, step;
};

struct node {
	int x, y;
	struct weight *head_weight;
};

SDL_Surface *screen;

struct node me, you;

void *xcalloc (unsigned int a, unsigned int b);
char *xstrdup (const char *old);
void step (struct node *np);
void draw (void);
void process_input (void);
void add_weight (struct weight *wp, struct node *np);
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
vscale (struct vect *vp1, struct vect *vp0, double s)
{
	vp1->x = vp0->x * s;
	vp1->y = vp0->y * s;
}

void
vnorm (struct vect *vp1, struct vect *vp0)
{
	double mag;

	mag = hypot (vp0->x, vp0->y);

	vscale (vp1, vp0, 1 / mag);
}

void
step (struct node *np)
{
	double r, totalmag, chance;
	struct weight *wp;
	struct vect v;

	totalmag = 0;
	for (wp = np->head_weight; wp; wp = wp->next) {
		totalmag += hypot (wp->v.x, wp->v.y);
	}

	r = (double) rand () / RAND_MAX;
	chance = 0;

	for (wp = np->head_weight; wp; wp = wp->next) {
		vscale (&v, &wp->v, 1 / totalmag);

		chance += hypot (v.x, v.y);

		if (r < chance)
			break;
	}

	np->x += wp->step.x;
	np->y += wp->step.y;
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
add_weight (struct weight *wp0, struct node *np)
{
	struct weight *wp1;

	wp1 = xcalloc (1, sizeof *wp1);

	wp1->v.x = wp0->v.x;
	wp1->v.y = wp0->v.y;
	wp1->step.x = wp0->step.x;
	wp1->step.y = wp0->step.y;

	if (np->head_weight) {
		wp1->next = np->head_weight;
		np->head_weight = wp1;
	} else {
		np->head_weight = wp1;
	}
}

void
init_node (struct node *np, int x, int y)
{
	np->x = x;
	np->y = y;
}

void
build_weight (struct weight *wp, double x, double y, double str)
{
	struct vect v;

	wp->v.x = x;
	wp->v.y = y;

	vnorm (&v, &wp->v);
	vscale (&v, &v, str);

	wp->step.x = v.x;
	wp->step.y = v.y;
}

int
main (int argc, char **argv)
{
	struct weight dirs[4];

	srand (time (NULL));

	build_weight (&dirs[0], 1000, 0, STEPSIZE);
	build_weight (&dirs[1], 0, -1000, STEPSIZE);
	build_weight (&dirs[2], -1000, 0, STEPSIZE);
	build_weight (&dirs[3], 0, 1000, STEPSIZE);

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
