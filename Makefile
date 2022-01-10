CFLAGS=-lm -o
OUT=ecsrun
FILES=ecsmain.c ecs.c ecs.h vector.h hashMap.c hashMap.h queue.h

compile:
	gcc $(FILES) $(CFLAGS) $(OUT) 
debug:
	gcc $(FILES) -g $(CFLAGS) $(OUT)
