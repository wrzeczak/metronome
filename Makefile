build:
	gcc metronome.c -o met -lraylib -lm -lgdi32 -lwinmm

run: build
	./met