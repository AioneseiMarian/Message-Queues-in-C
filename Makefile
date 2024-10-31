all: source/publisher.c source/server.c source/dbwriter.c
	-make clean
	cc source/publisher.c -o _publisher
	cc source/server.c -o _server
	cc source/dbwriter.c -o _dbwriter
publisher: source/publisher.c
	cc source/publisher.c -o _publisher
server: source/server.c
	cc source/server.c -o _server
db: source/db_writer.c
	cc source/dbwriter.c -o _dbwriter
clean:
	-rm _publisher
	-rm _server
	-rm _dbwriter
	-rm tmp.txt
