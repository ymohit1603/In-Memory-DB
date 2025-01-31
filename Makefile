flags=-O2 -Wall -std=c2x

all: clean tree redis

tree:tree.o
	cc ${flags} $^ -o $@ ${ldflags}

tree.o: tree.c
	cc ${flags} -c $^

redis:redis.o
	cc ${flags} $^ -o $@ ${ldflags}

redis.o: redis.c
	cc ${flags} -c $^

clean:
	rm -f *.o redis tree