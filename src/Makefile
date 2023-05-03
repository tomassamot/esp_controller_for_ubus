SRCS:=$(wildcard *.c)

.PHONY:all clean

all:ubus_esp

ubus_esp: $(SRCS)
# ubus_esp: counter.c
	$(CC) -o $@ $^ -lblobmsg_json -lubox -lubus

clean:
	rm -rf ubus_esp
