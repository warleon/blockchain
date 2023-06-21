CC := g++
CFLAGS := -std=c++17 -Wall -Wextra -lX11
LIBS := -lstdc++fs -lpthread

SERVER_SRC := SimpleServer.cpp net_server.h net_common.h net_message.h net_tsqueue.h net_connection.h olc_net.h
CLIENT_SRC := SimpleClient.cpp net_client.h net_common.h net_message.h net_tsqueue.h net_connection.h olc_net.h

SERVER_EXEC := server
CLIENT_EXEC := client

.PHONY: all clean

all: $(SERVER_EXEC) $(CLIENT_EXEC)

$(SERVER_EXEC): $(SERVER_SRC)
	$(CC) $(CFLAGS) $(SERVER_SRC) -o $(SERVER_EXEC) $(LIBS)

$(CLIENT_EXEC): $(CLIENT_SRC)
	$(CC) $(CFLAGS) $(CLIENT_SRC) -o $(CLIENT_EXEC) $(LIBS)

clean:
	rm -f $(SERVER_EXEC) $(CLIENT_EXEC)
