DEBUG = 

lc3-vm: main.c lc3.c
	gcc -std=c99 -Wall -Wextra $(DEBUG) main.c lc3.c -o lc3-vm

debug: DEBUG = -g
debug: lc3-vm

clean: 
	rm lc3-vm

.PHONY: clean debug
