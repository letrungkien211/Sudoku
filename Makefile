TARGET = fast final invent
all: $(TARGET)

%: %.c
	gcc -o $@ $^
clean:
	rm -f fast final invent *#* *~