publisher: publisher.c
	cc publisher.c -o publisher
server: server.c
	cc server.c -o server
db: db_writer.c
	cc db_writer.c -o dbwriter
clean:
	rm publisher
	rm server
	rm dbwriter
