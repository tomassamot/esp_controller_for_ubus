BIN:=ubus_esp
SRCS:=$(wildcard *.c)
CPPFLAGS:= 
LDFLAGS:= -lblobmsg_json -lubox -lubus -lserialport
.PHONY:all clean

all:$(BIN)

$(BIN): $(SRCS)
	$(CC) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) 

clean:
	rm -rf ubus_esp
