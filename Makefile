# Compilador e flags
CC = gcc
CFLAGS = -Wall -pthread -g

# Nome do executável
EXEC = main

# Regra principal
all: $(EXEC)

# Como compilar o programa
$(EXEC): main.c
	$(CC) $(CFLAGS) -o $(EXEC) main.c

# Limpeza dos arquivos gerados
clean:
	rm -f $(EXEC)