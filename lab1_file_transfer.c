#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#define BUFFER_SIZE 1024

// print error message and exit program
void err(char* err_msg[]) {
  fputs(err_msg, stdout);
  exit(-1);
}

void tcp_send(in_addr_t ip, int port, char* file_name[]) {

}

void tcp_recv(in_addr_t ip, int port) {

}

void udp_send(in_addr_t ip, int port, char* file_name[]) {
  // create udp socket
  int sock;
  if ((sock = socket(PF_INET, SOCK_DGRAM, 0) < 0)) {
    err("[ERR] Socket error\n");
  }

  // initialize client info
  struct sockaddr_in cli_addr;
  memset(&cli_addr, 0, sizeof(cli_addr));
  socklen_t cli_len = sizeof(cli_addr);
  cli_addr.sin_family = AF_INET;
  cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  cli_addr.sin_port = htons(port);

  // binding
  if (bind(sock, (struct sockaddr*)&cli_addr, sizeof(cli_addr)) < 0) {
    err("[ERR] Bind error\n");
  }

  // buffer
  char buf[BUFFER_SIZE] = {0};
  memset(buf, 0, sizeof(buf));

  // catch request messsage from client
  int recv_len = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&cli_addr, &cli_len);
  if (recv_len == -1) {
    err("[ERR] Receive error\n");
  }
  if (strcmp("REQUEST", buf) != 0) {
    err("[ERR] Wrong request message from client\n");
  }

  // variables for tracking sending progress
  int percent = 0;

  // open file
  FILE* fp = fopen(file_name, "r");
  if (fp == NULL) {
    err("[ERR] File open error\n");
  }

  // get file size
  struct stat file_stat;
  stat(file_name, &file_stat);
  uint64_t file_size = file_stat.st_size;

  // send file size to client
  sendto(sock, file_size, sizeof(file_size), 0, (struct sockaddr *)&cli_addr, cli_len);

  // start sending file content
  while (!feof(fp)) {
    // clear buffer
    memset(buf, 0, sizeof(buf));

    // read data from file
    int send_len = fread(buf, sizeof(char), sizeof(buf), fp);

    // send to client
    sendto(sock, buf, send_len, 0, (struct sockaddr *)&cli_addr, cli_len);

    // print percentage
    percent += ((double) send_len / (double) file_size) * 100;
    printf("[INFO] %d%%\n", percent);
  }

  // finish sending file content
  sendto(sock, "EOF", 3, 0, (struct sockaddr *)&cli_addr, cli_len);
  printf("[INFO] File transfer fininshed\n");

  // close socket
  close(sock);

}

void udp_recv(in_addr_t ip, int port) {

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
      tcp_recv(inet_addr(argv[3]), atoi(argv[4]));
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
      // params: ip, port
      udp_recv(inet_addr(argv[3]), atoi(argv[4]));
    }

  }

}