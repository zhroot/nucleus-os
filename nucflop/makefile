CC = gcc

LDFLAGS = -Os -s
CFLAGS = -Wall

OBJ = nucflop.o fat.o

all: nucflop.exe
	@rem echo nucflop wurde erfolgreich compiliert!

clean:
	-rm $(OBJ) 
	-rm nucflop.exe
	@rem echo nucflop wurde geloescht!

nucflop.exe: $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@
	
%o: %c
	$(CC) $(CFLAGS) -c $? -o $@
