CFLAGS=-lm -o
OUT=ecsrun
FILES=ecsmain.c ecs.c ecs.h Type-Safe-Vector/vector.c Type-Safe-Vector/vector.h Type-Safe-Hash-Map/hashMap.c Type-Safe-Hash-Map/hashMap.h Type-Safe-Queue/queue.h

compile:
	gcc $(FILES) $(CFLAGS) $(OUT) 
debug:
	gcc $(FILES) -g $(CFLAGS) $(OUT)
clean:
	rm -f smite.txt
	rm -f summon.txt
	rm -f add.txt
	rm -f remove.txt
	rm -f valgrind-out.txt
	rm -f ecsrun
	rm -f testOutput.txt
