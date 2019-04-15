CC = gcc
FILES = sod/sod.c plate_detection.c
OUT = plate_detection

build: $(FILES)
	$(CC) $(FILES) -lm -Ofast -march=native -Wall -std=c99 -o $(OUT)

clean:
	rm -f *.png plate_detection
