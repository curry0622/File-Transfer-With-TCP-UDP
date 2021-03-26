#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define BUFFER_SIZE 1024

void tcp_send() {

}

void tcp_recv() {

}

void udp_send() {

}

void udp_recv() {

}

int main(int argc, char* argv[]) {
  // tcp
  if (strcmp(argv[1], "tcp") == 0) {
    // send
    if (strcmp(argv[2], "send") == 0) {
      // params: ip, port, file_name
      tcp_send(inet_addr(argv[3]), atoi(argv[4]), argv[5]);
    }

    // recv
    else {
      // params: ip, port
      tcp_recvinet_addr((argv[3]), atoi(argv[4]));
    }

  }

  // udp
  else {
    // send
    if (strcmp(argv[2], "send") == 0) {
      // params: ip, port, file_name
      udp_send(inet_addr(argv[3]), atoi(argv[4]), argv[5]);
    }

    // recv
    else {
      // params: ip, port, file_name
      udp_recv(inet_addr(argv[3]), atoi(argv[4]));
    }

  }

}