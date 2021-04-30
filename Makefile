CC = g++ -w
serverA: serverA.cpp
	$(CC) serverA.cpp -o serverA
serverB: serverB.cpp
	$(CC) serverB.cpp -o serverB
client: client.cpp
	$(CC) client.cpp -o client
servermain: servermain.cpp
	$(CC) servermain.cpp -o servermain

all: servermain serverA serverB client 
clean:
	$(RM) servermain serverA serverB client 



