#include "common_types.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXRCVLEN 500
#define PORTNUM 2300

typedef int socket_id_t;
typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;

socket_id_t openTCP(int ip, int port) {
  sockaddr_in address = {};
  socket_id_t conn;
  conn = socket(AF_INET, SOCK_STREAM, 0);

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(ip);
  address.sin_port = htons(port);

  connect(conn, (sockaddr *)&address, sizeof(sockaddr_in));

  return conn;
}
inline char *createBuffer(size_t max_size) {
  return (char *)malloc(max_size + 1);
}
inline void destroyBuffer(char *buff) { free((void *)buff); }

int main(int argc, char *argv[]) {
  char *buffer = createBuffer(MAXRCVLEN);
  int len, mysocket = openTCP(INADDR_LOOPBACK, 3000);

  len = recv(mysocket, buffer, MAXRCVLEN, 0);

  /* We have to null terminate the received data ourselves */
  buffer[len] = '\0';

  printf("Received %s (%d bytes).\n", buffer, len);

  close(mysocket);
  destroyBuffer(buffer);
  return EXIT_SUCCESS;
}