flags=-O2 -Wall -std=c2x
ldflags=  # Define this if needed (e.g., `-lpthread` for threading support)

all: clean redis

tree.o: tree.c tree.h  # Recompile if tree.h changes
	cc ${flags} -c tree.c -o tree.o

redis.o: redis.c redis.h tree.h  # Ensure tree.h is included if used
	cc ${flags} -c redis.c -o redis.o

redis: redis.o tree.o  # Link redis with tree.o
	cc ${flags} redis.o tree.o -o redis ${ldflags}

clean:
	rm -f *.o redis
