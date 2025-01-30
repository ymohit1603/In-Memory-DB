flags=-02 -Wall -std=c2x

all: clean redis

redis:redis.o
	cc ${flags} $^ -o $@ ${ldflags}

redis.o: redis.c
	cc ${flags} -c $^

clean:
	rm -f *.o redis