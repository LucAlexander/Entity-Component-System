CC=gcc
CFLAGS=-lm -o
CDEBUGFLAGS=-Wall -g
OUT=ecsrun
CLIBS=-L../DataContainers -lDataContainers
FILES=ecsmain.c ecs.c ecs.h 
LNAME=libEntityComponentSystem.a

compile:
	$(CC) $(FILES) $(CLIBS) $(CFLAGS) $(OUT) 
debug:
	$(CC) $(FILES) $(CLIBS) $(CDEBUGFLAGS) $(CFLAGS) $(OUT)

linkedCompile:
	$(CC) ecsmain.c -L. -lEntityComponentSystem $(CLIBS) $(CFLAGS) $(OUT)

build:
	mkdir object
	$(CC) -c ecs.c -o ./object/ecs.o
	ar rcs $(LNAME) ./object/*.o

destruct:
	rm -rf ./object
	rm -rf $(LNAME)

rebuild:
	make destruct
	make build

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
