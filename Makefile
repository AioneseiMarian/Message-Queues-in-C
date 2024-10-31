all: source/publisher.c source/server.c source/dbwriter.c
	make clean
	make server
	make publisher
	make db
publisher: source/publisher.c
	cc source/publisher.c -o _publisher
server: source/server.c
	cc source/server.c source/heap.c -o _server
db: source/dbwriter.c
	cc source/dbwriter.c -o _dbwriter
clean:
	-rm _publisher
	-rm _server
	-rm _dbwriter
	-rm tmp.txt
