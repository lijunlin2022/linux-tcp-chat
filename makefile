src = $(wildcard ./src/*.c)
obj = $(patsubst ./src/%.c, ./obj/%.o, $(src))

inc_path = ./inc
myArgs = -Wall -g

ALL:server.out client.out

client.out:./obj/client.o ./obj/customsocket.o ./obj/customfile.o
	gcc ./obj/client.o ./obj/customsocket.o ./obj/customfile.o -lpthread -o client.out $(myArgs)
server.out:./obj/server.o ./obj/customsocket.o ./obj/customfile.o
	gcc ./obj/server.o ./obj/customsocket.o ./obj/customfile.o -lpthread -o server.out $(myArgs)

$(obj):./obj/%.o:./src/%.c
	gcc -c $< -o $@ $(myArgs) -I $(inc_path)

clean:
	-rm -rf *.out *.txt

.PHONY:clean ALL
