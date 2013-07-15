 #include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <SDL.h>
#include <SDL_gfxPrimitives.h>

#define SCREENWIDTH 640
#define SCREENHEIGHT 480
#define SCREENSCALE 10

#define WIDTH SCREENWIDTH*SCREENSCALE
#define HEIGHT SCREENHEIGHT*SCREENSCALE

#define BASEWEIGHT 500
#define BASEPULL 20
#define STEPSIZE 20
#define SIDEPUSH 1000
#define ATTRACTION 2000

enum {
	UP,
	DOWN,
	LEFT,
	RIGHT,
};

struct vect {
	double x, y;
};

struct weight {
	struct weight *next;
	struct vect v, step;
};

struct attract {
	struct attract *next;
	struct node *np;
	double pull, attract;
};

struct node {
	struct node *next;
	struct vect pos;
	struct weight *head_weight;
	struct attract *head_attract;
	double color;
};

SDL_Surface *screen;

struct node *head_node;

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
	vscale (vp1, vp0, 1 / hypot (vp0->x, vp0->y));
}

void
vsub (struct vect *vp2, struct vect *vp0, struct vect *vp1)
{
	vp2->x = vp0->x - vp1->x;
	vp2->y = vp0->y - vp1->y;
}

void
step (struct node *np0)
{
	double r, totalmag, chance, pull;
	/* struct node *np1; */
	struct weight *wp, cardinal[4], *sel, *head_extra;
	struct vect v;
	struct attract *ap;

	sel = NULL;

	totalmag = 0;
	for (wp = np0->head_weight; wp; wp = wp->next) {
		totalmag += hypot (wp->v.x, wp->v.y);
	}

	head_extra = &cardinal[UP];

	cardinal[UP].v.x = 0;
	cardinal[UP].v.y = (-np0->pos.y / HEIGHT) * SIDEPUSH;
	cardinal[UP].step.x = 0;
	cardinal[UP].step.y = -STEPSIZE;
	cardinal[UP].next = &cardinal[DOWN];

	cardinal[DOWN].v.x = 0;
	cardinal[DOWN].v.y = ((HEIGHT - np0->pos.y) / HEIGHT) * SIDEPUSH;
	cardinal[DOWN].step.x = 0;
	cardinal[DOWN].step.y = STEPSIZE;
	cardinal[DOWN].next = &cardinal[LEFT];

	cardinal[LEFT].v.x = (-np0->pos.x / WIDTH) * SIDEPUSH;
	cardinal[LEFT].v.y = 0;
	cardinal[LEFT].step.x = -STEPSIZE;
	cardinal[LEFT].step.y = 0;
	cardinal[LEFT].next = &cardinal[RIGHT];

	cardinal[RIGHT].v.x = ((WIDTH - np0->pos.x) / WIDTH) * SIDEPUSH;
	cardinal[RIGHT].v.y = 0;
	cardinal[RIGHT].step.x = STEPSIZE;
	cardinal[RIGHT].step.y = 0;
	cardinal[RIGHT].next = NULL;

	for (ap = np0->head_attract; ap; ap = ap->next) {
		wp = xcalloc (1, sizeof *wp);

		vsub (&v, &ap->np->pos, &np0->pos);

		if (hypot (v.x, v.y) < 50) {
			pull = -ap->pull;
		} else {
			pull = ap->pull;
		}

		vnorm (&v, &v);
		vscale (&wp->v, &v, ap->attract);
		vscale (&wp->step, &v, pull);

		wp->next = head_extra;
		head_extra = wp;
	}

	for (wp = head_extra; wp; wp = wp->next) {
		totalmag += hypot (wp->v.x, wp->v.y);
	}

	r = (double) rand () / RAND_MAX;
	chance = 0;

	for (wp = np0->head_weight; wp; wp = wp->next) {
		vscale (&v, &wp->v, 1 / totalmag);

		chance += hypot (v.x, v.y);

		if (r < chance) {
			sel = wp;
			break;
		}
	}

	if (!sel) {
		for (wp = head_extra; wp; wp = wp->next) {
			vscale (&v, &wp->v, 1 / totalmag);

			chance += hypot (v.x, v.y);

			if (r < chance) {
				sel = wp;
				break;
			}
		}
	}

	if (sel) {
		np0->pos.x += sel->step.x;
		np0->pos.y += sel->step.y;
	}
}

void
draw (void)
{
	struct node *np;

	for (np = head_node; np; np = np->next) {
		filledCircleColor (screen, np->pos.x / SCREENSCALE,
				   np->pos.y / SCREENSCALE, 5, np->color);
		aacircleColor (screen, np->pos.x / SCREENSCALE,
			       np->pos.y / SCREENSCALE, 5, np->color);
		aacircleColor (screen, np->pos.x / SCREENSCALE,
			       np->pos.y / SCREENSCALE, 50, np->color);
		aacircleColor (screen, np->pos.x / SCREENSCALE,
			       np->pos.y / SCREENSCALE, 200, np->color);
	}
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

void
init_node (int x, int y, double color)
{
	struct weight dirs[4];
	struct node *np;

	np = xcalloc (1, sizeof *np);

	build_weight (&dirs[0], 1000, 0, STEPSIZE);
	build_weight (&dirs[1], 0, -1000, STEPSIZE);
	build_weight (&dirs[2], -1000, 0, STEPSIZE);
	build_weight (&dirs[3], 0, 1000, STEPSIZE);

	np->pos.x = x;
	np->pos.y = y;
	np->color = color;

	add_weight (&dirs[0], np);
	add_weight (&dirs[1], np);
	add_weight (&dirs[2], np);
	add_weight (&dirs[3], np);

	if (head_node) {
		np->next = head_node;
		head_node = np;
	} else {
		head_node = np;
	}
}

void
build_attract (void)
{
	struct node *np0, *np1;
	struct attract *ap;

	for (np0 = head_node; np0; np0 = np0->next) {
		for (np1 = head_node; np1; np1 = np1->next) {
			if (np1 == np0)
				continue;

			ap = xcalloc (1, sizeof *ap);

			ap->np = np1;
			ap->pull = BASEPULL;
			ap->attract = ATTRACTION;

			if (np0->head_attract) {
				ap->next = np0->head_attract;
				np0->head_attract = ap;
			} else {
				np0->head_attract = ap;
			}
		}
	}
}

int
main (int argc, char **argv)
{
	struct node *np;

	srand (time (NULL));

	init_node (WIDTH / 2, HEIGHT / 2, 0x00ff00ff);
	init_node (WIDTH / 2, HEIGHT / 4, 0xff0000ff);
	init_node (WIDTH / 4, HEIGHT / 2, 0x0000ffff);

	build_attract ();

	if (SDL_Init (SDL_INIT_VIDEO) != 0) {
		fprintf (stderr, "unable to initialize SDL: %s\n",
			 SDL_GetError ());
		return (1);
	}

	screen = SDL_SetVideoMode (SCREENWIDTH, SCREENHEIGHT, 32,
				   SDL_HWSURFACE | SDL_DOUBLEBUF);

	while (1) {
		process_input ();
		SDL_FillRect (screen, NULL, 0x000000);

		for (np = head_node; np; np = np->next) {
			step (np);
		}

		draw ();
		SDL_Flip (screen);
		SDL_Delay (10);
	}

	return (0);
}
