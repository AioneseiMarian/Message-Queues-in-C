LDFLAGS = -ljson-c
SOURCE = server/queue.c server/rbtree.c server/hash_table.c server/json_utils.c
.PHONY: all
all:: client/publisher.c server/server.c client/dbwriter.c
	make clean
	make server
	make publisher
	make db
.PHONY: publisher
publisher: client/publisher.c
	cc -Wall -g client/publisher.c $(SOURCE) -o publisher $(LDFLAGS)
.PHONY: server
server:: server/server.c
	cc server/server.c $(SOURCE) -o serv $(LDFLAGS)
.PHONY: db
db: client/dbwriter.c
	cc client/dbwriter.c $(SOURCE) -o dbwriter $(LDFLAGS)
.PHONY: clean
clean:
	-rm _publisher
	-rm _server
	-rm _dbwriter
	-rm tmp.txt
.PHONY: cleantests
cleantests:
	-rm test*
