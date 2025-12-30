OBJ=$(patsubst %.c,%.o,$(wildcard *.c))
stego.exe : $(OBJ)
	gcc -o $@ $^
clean:
	rm *.o *.exe