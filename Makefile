all: source/publisher.c source/server.c source/db_writer.c
	make clean
	cc source/publisher.c -o publisher
	cc source/server.c -o server
	cc source/db_writer.c -o dbwriter
publisher: source/publisher.c
	cc source/publisher.c -o publisher
server: source/server.c
	cc source/server.c -o server
db: source/db_writer.c
	cc source/db_writer.c -o dbwriter
clean:
	rm publisher
	rm server
	rm dbwriter
