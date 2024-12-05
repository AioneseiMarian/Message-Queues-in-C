LDFLAGS = -ljson-c
SOURCE = server/queue.c server/rbtree.c server/hash_table.c server/json_utils.c
.PHONY: all
all:: client/publisher.c server/server.c client/dbwriter.c client/subscriber.c
	make clean
	make server
	make publisher
	make subscriber
	make db
publisher: client/publisher.c
	cc -Wall -g client/publisher.c $(SOURCE) -o publisher $(LDFLAGS)
subscriber: client/subscriber.c
	cc -Wall -g client/subscriber.c ${SOURCE} -o subscriber ${LDFLAGS}
.PHONY: server
server:: server/server.c
	cc server/server.c $(SOURCE) -o serv $(LDFLAGS)
.PHONY: db
db: client/dbwriter.c
	cc client/dbwriter.c $(SOURCE) -o dbwriter $(LDFLAGS)
.PHONY: clean
clean:
	-rm subscriber
	-rm publisher
	-rm serv
	-rm dbwriter
.PHONY: cleantests
cleantests:
	-rm test*
