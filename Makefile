CC=gcc
CFLAGS=-Wall -O3 -flto -funroll-loops -fomit-frame-pointer -Iinclude `pkg-config --cflags glib-2.0` -g
LDFLAGS=`pkg-config --libs glib-2.0`
#flto
all: folders dserver dclient

dserver: bin/dserver
dclient: bin/dclient

folders:
	@mkdir -p src include obj bin tmp indice

bin/dserver: obj/servidor.o obj/cache.o
	$(CC) $(LDFLAGS) $^ -o $@

bin/dclient: obj/cliente.o
	$(CC) $(LDFLAGS) $^ -o $@

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f obj/* tmp/* bin/*
	make cleanData

cleanData:
	rm -f indice/*