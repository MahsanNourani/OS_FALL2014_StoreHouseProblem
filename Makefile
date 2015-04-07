SERVER:server.o stuck.o item.o
	g++ -o server server.o stuck.o item.o
server.o:server.cpp stuck.o item.o
	g++ -c server.cpp
stuck.o:stuck.cpp stuck.h item.o
	g++ -c stuck.cpp
item.o:item.cpp item.h
	g++ -c item.cpp
clean:
	rm *.o