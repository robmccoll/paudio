paudio: main.c
	gcc -std=c99 $^ -o $@ -lportaudio -lm 
