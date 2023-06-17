CC := g++
CFLAGS := -std=c++17 -Wall -Wextra
LIBS := -lstdc++fs -lpthread

SERVER_SRC := net_server.cpp net_common.h net_message.cpp net_tsqueue.cpp net_connection.cpp
CLIENT_SRC := net_client.cpp net_common.h net_message.cpp net_tsqueue.cpp net_connection.cpp

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
