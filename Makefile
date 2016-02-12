SRCS= alt_ssh_auth_sock.c
LIB= libaltsshauthsock.so

PHONY=

PHONY+= all
all: $(LIB)

$(LIB): $(SRCS)
	$(CC) -Wall -fPIC -shared -o $@ $^ -ldl

PHONY+= clean
clean:
	$(RM) $(LIB)

.PHONY: $(PHONY)
