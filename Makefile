CFLAGS=-lm -o
CDEBUGFLAGS=-Wall -g
OUT=ecsrun
CLIBS=-LDataContainers -lDataContainers
FILES=ecsmain.c ecs.c ecs.h 

compile:
	gcc $(FILES) $(CLIBS) $(CFLAGS) $(OUT) 
debug:
	gcc $(FILES) $(CLIBS) $(CDEBUGFLAGS) $(CFLAGS) $(OUT)
clean:
	rm -f smite.txt
	rm -f summon.txt
	rm -f add.txt
	rm -f remove.txt
	rm -f valgrind-out.txt
	rm -f ecsrun
	rm -f testOutput.txt
	rm -f query.txt
	rm -f systems.txt
