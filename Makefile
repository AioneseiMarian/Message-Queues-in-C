LDFLAGS = -ljson-c
.PHONY: all publisher server db clean cleantests

all:: client/publisher.c server/server.c client/dbwriter.c
	make clean
	make server
	make publisher
	make db
publisher: client/publisher.c
	cc client/publisher.c -o _publisher $(LDFLAGS)
server: server/server.c
	cc server/server.c server/heap.c server/queue.c -o _server $(LDFLAGS)
db: client/dbwriter.c
	cc client/dbwriter.c -o _dbwriter $(LDFLAGS)
clean:
	-rm _publisher
	-rm _server
	-rm _dbwriter
	-rm tmp.txt
cleantests:
	-rm test*
