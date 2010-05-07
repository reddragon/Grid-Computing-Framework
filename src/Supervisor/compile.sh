g++ -c  main.cpp $(pkg-config --cflags --libs gtk+-2.0 gthread-2.0) -lpthread -lm -lstatgrab
g++ -c  callbacks.c $(pkg-config --cflags --libs gtk+-2.0 gthread-2.0) -lpthread -lm -lstatgrab
g++ -c  interface.cpp $(pkg-config --cflags --libs gtk+-2.0 gthread-2.0) -lpthread -lm -lstatgrab
g++ -c  support.c $(pkg-config --cflags --libs gtk+-2.0 gthread-2.0) -lpthread -lm -lstatgrab
g++ -o supervisor callbacks.o support.o interface.o  main.o $(pkg-config --cflags --libs gtk+-2.0 gthread-2.0) -lpthread -lm -lstatgrab
