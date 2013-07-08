CFLAGS = -g -Wall `sdl-config --cflags`
LIBS = `sdl-config --libs` -lSDL_gfx

fourthkind: fourthkind.o
	$(CC) $(CFLAGS) -o fourthkind fourthkind.o $(LIBS)

clean:
	rm -f *~ *.o
