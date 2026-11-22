# Makefile

CC = gcc
CFLAGS = -Wall -c
SRC = src

all: server client other

server: server.o utente.o item.o
	$(CC) -Wall utente.o item.o server.o -o server

client: client.o partita.o
	$(CC) -Wall client.o partita.o -o client

other: client.o partita.o
	$(CC) -Wall client.o partita.o -o other


server.o: $(SRC)/server.c $(SRC)/item.h 
	$(CC) $(CFLAGS) $(SRC)/server.c -o server.o

client.o: $(SRC)/client.c 
	$(CC) $(CFLAGS) $(SRC)/client.c -o client.o

utente.o: $(SRC)/utente.c $(SRC)/utente.h
	$(CC) $(CFLAGS) $(SRC)/utente.c -o utente.o

partita.o: $(SRC)/partita.c $(SRC)/partita.h
	$(CC) $(CFLAGS) $(SRC)/partita.c -o partita.o

item.o: $(SRC)/item.c $(SRC)/item.h
	$(CC) $(CFLAGS) $(SRC)/item.c -o item.o


clean:
	rm *.o server client

run: all
	gnome-terminal --tab --title="Server Terminal" --command="bash -c './server 4242; exec bash'" \
                   --tab --title="Client Terminal" --command="bash -c './client 6000; exec bash'" \
				   --tab --title="Other Terminal" --command="bash -c './other 6100; exec bash'"