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
#include <time.h>

#define BUFFER_SIZE 16384

// print error message and exit program
void err(char* err_msg) {
  fputs(err_msg, stdout);
  exit(-1);
}

// tcp: send file
void tcp_send(in_addr_t ip, int port, char* file_name) {
  // socket
  int sock, cli_sock;
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    err("[ERR] Socket error\n");
  }

  // buffer
  char buf[BUFFER_SIZE] = {0};
  memset(buf, 0, sizeof(buf));

  // initialization
  struct sockaddr_in serv_addr, cli_addr;
  memset(&serv_addr, 0, sizeof(serv_addr));
  socklen_t cli_len = sizeof(cli_addr);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port);

  // binding
  if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    err("[ERR] Bind error\n");
  }

  // listen
  listen(sock, 5);

  // accept
  if((cli_sock = accept(sock, (struct sockaddr *)&cli_addr, &cli_len)) < 0)
    err("[ERR] Accept error\n");

  // open file
  FILE* fp = fopen(file_name, "r");
  if (fp == NULL) {
    err("[ERR] Failed to open file\n");
  }

  // get file size
  struct stat file_stat;
  stat(file_name, &file_stat);
  uint64_t file_size = file_stat.st_size;

  // send file size to client
  write(cli_sock, &file_size, sizeof(file_size));

  // progress record
  uint64_t curr_progress = 0;
  int partition_25 = file_size * 0.25;
  int partition_50 = file_size * 0.5;
  int partition_75 = file_size * 0.75;
  printf("[RECORD] 0%%  ");

  // time
  time_t timer;
  clock_t start, end;
  start = clock();
  struct tm* Now;

  time(&timer);
  Now = localtime(&timer);
  printf("%s", asctime(Now));

  // start sending file content to client
  while (!feof(fp)) {
    // clear buffer
    memset(buf, 0, sizeof(buf));

    // read data from file
    int send_len = fread(buf, sizeof(char), sizeof(buf), fp);

    // send to client
    int write_len = write(cli_sock, buf, send_len);

    // progress record
    curr_progress += write_len;
    if (partition_25 - BUFFER_SIZE / 2 <= curr_progress && curr_progress <= partition_25 + BUFFER_SIZE / 2) {
      printf("[RECORD] 25%%  ");
      time(&timer);
      Now = localtime(&timer);
      printf("%s", asctime(Now));
    }
    if (partition_50 - BUFFER_SIZE / 2 <= curr_progress && curr_progress <= partition_50 + BUFFER_SIZE / 2) {
      printf("[RECORD] 50%%  ");
      time(&timer);
      Now = localtime(&timer);
      printf("%s", asctime(Now));
    }
    if (partition_75 - BUFFER_SIZE / 2 <= curr_progress && curr_progress <= partition_75 + BUFFER_SIZE / 2) {
      printf("[RECORD] 75%%  ");
      time(&timer);
      Now = localtime(&timer);
      printf("%s", asctime(Now));
    }

  }

  // finish sending file content
  fclose(fp);
  printf("[RECORD] 100%%  ");
  time(&timer);
  Now = localtime(&timer);
  printf("%s", asctime(Now));
  printf("[INFO] File transfer finished\n");
  end = clock();

  // print info
  printf("[INFO] File size = %.2f MB\n", (double) file_size / 1048576);
  printf("[INFO] Total time = %.2lf ms\n", (double) (end - start) / (CLOCKS_PER_SEC / 1000));

  // close socket
  close(sock);
  close(cli_sock);

}

// tcp: receive file
void tcp_recv(in_addr_t ip, int port) {
  // socket
  int sock;
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    err("[ERR] Socket error\n");
  }

  // initialize
  struct sockaddr_in serv_addr;
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = ip;
  serv_addr.sin_port = htons(port);

  // connect
  if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    err("[ERR] Connect error\n");
  }

  // buffer
  char buf[BUFFER_SIZE] = {0};
  memset(buf, 0, sizeof(buf));

  // get file size from server
  uint64_t file_size;
  read(sock, &file_size, sizeof(file_size));

  // open file
  FILE* fp = fopen("recv.txt", "w");
  if (fp == NULL) {
    err("[ERR] File open error\n");
  }

  // progress record
  uint64_t curr_progress = 0;
  int partition_25 = file_size * 0.25;
  int partition_50 = file_size * 0.5;
  int partition_75 = file_size * 0.75;
  printf("[RECORD] 0%%  ");

  // time
  time_t timer;
  clock_t start, end;
  start = clock();
  struct tm* Now;

  time(&timer);
  Now = localtime(&timer);
  printf("%s", asctime(Now));

  while (1) {
    // clear buffer
    memset(buf, 0, sizeof(buf));

    // get file content from server
    int recv_len = read(sock, buf, sizeof(buf));
    if (recv_len == -1) {
      err("[ERR] Receive error\n");
    }

    // if received data length = 0
    if (recv_len == 0) {
      fclose(fp);
      break;
    }

    // write to file
    int write_len = fwrite(buf, sizeof(char), recv_len, fp);

    // progress record
    curr_progress += write_len;
    if (partition_25 - BUFFER_SIZE / 2 <= curr_progress && curr_progress <= partition_25 + BUFFER_SIZE / 2) {
      printf("[RECORD] 25%%  ");
      time(&timer);
      Now = localtime(&timer);
      printf("%s", asctime(Now));
    }
    if (partition_50 - BUFFER_SIZE / 2 <= curr_progress && curr_progress <= partition_50 + BUFFER_SIZE / 2) {
      printf("[RECORD] 50%%  ");
      time(&timer);
      Now = localtime(&timer);
      printf("%s", asctime(Now));
    }
    if (partition_75 - BUFFER_SIZE / 2 <= curr_progress && curr_progress <= partition_75 + BUFFER_SIZE / 2) {
      printf("[RECORD] 75%%  ");
      time(&timer);
      Now = localtime(&timer);
      printf("%s", asctime(Now));
    }

  }

  // print info
  printf("[RECORD] 100%%  ");
  time(&timer);
  Now = localtime(&timer);
  printf("%s", asctime(Now));
  printf("[INFO] File transfer fininshed\n");
  end = clock();

  // print info
  printf("[INFO] File size = %.2f MB\n", (double) file_size / 1048576);
  printf("[INFO] Total time = %.2lf ms\n", (double) (end - start) / (CLOCKS_PER_SEC / 1000));

  close(sock);

}

// udp: send file
void udp_send(in_addr_t ip, int port, char* file_name) {
  // create udp socket
  int sock;
  if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
    err("[ERR] Socket error\n");
  }

  // initialize client info
  struct sockaddr_in cli_addr;
  memset(&cli_addr, 0, sizeof(cli_addr));
  socklen_t cli_len = sizeof(cli_addr);
  cli_addr.sin_family = AF_INET;
  cli_addr.sin_port = htons(port);
  cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);

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
  printf("[INFO] Client request success\n");

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
  sendto(sock, &file_size, sizeof(file_size), 0, (struct sockaddr *)&cli_addr, cli_len);

  // progress record
  uint64_t curr_progress = 0;
  int partition_25 = file_size * 0.25;
  int partition_50 = file_size * 0.5;
  int partition_75 = file_size * 0.75;
  printf("[RECORD] 0%%  ");

  // time
  time_t timer;
  clock_t start, end;
  start = clock();
  struct tm* Now;

  time(&timer);
  Now = localtime(&timer);
  printf("%s", asctime(Now));

  // start sending file content
  while (!feof(fp)) {
    // clear buffer
    memset(buf, 0, sizeof(buf));

    // read data from file
    int send_len = fread(buf, sizeof(char), sizeof(buf), fp);

    // send to client
    sendto(sock, buf, send_len, 0, (struct sockaddr *)&cli_addr, cli_len);

    // progress record
    curr_progress += send_len;
    if (partition_25 - BUFFER_SIZE / 2 <= curr_progress && curr_progress <= partition_25 + BUFFER_SIZE / 2) {
      printf("[RECORD] 25%%  ");
      time(&timer);
      Now = localtime(&timer);
      printf("%s", asctime(Now));
    }
    if (partition_50 - BUFFER_SIZE / 2 <= curr_progress && curr_progress <= partition_50 + BUFFER_SIZE / 2) {
      printf("[RECORD] 50%%  ");
      time(&timer);
      Now = localtime(&timer);
      printf("%s", asctime(Now));
    }
    if (partition_75 - BUFFER_SIZE / 2 <= curr_progress && curr_progress <= partition_75 + BUFFER_SIZE / 2) {
      printf("[RECORD] 75%%  ");
      time(&timer);
      Now = localtime(&timer);
      printf("%s", asctime(Now));
    }
  }

  // finish sending file content
  fclose(fp);
  sendto(sock, "EOF", 3, 0, (struct sockaddr *)&cli_addr, cli_len);
  printf("[RECORD] 100%%  ");
  time(&timer);
  Now = localtime(&timer);
  printf("%s", asctime(Now));
  printf("[INFO] File transfer fininshed\n");
  end = clock();

  // close socket
  close(sock);

  // print info
  printf("[INFO] File size = %.2f MB\n", (double) file_size / 1048576);
  printf("[INFO] Total time = %.2lf ms\n", (double) (end - start) / (CLOCKS_PER_SEC / 1000));

}

// udp: receive file
void udp_recv(in_addr_t ip, int port) {
  // create udp socket
  int sock;
  if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
    err("[ERR] Socket error\n");
  }

  // initialize server info
  struct sockaddr_in srv_addr;
  memset(&srv_addr, 0, sizeof(srv_addr));
  socklen_t srv_len = sizeof(srv_addr);
  srv_addr.sin_family = AF_INET;
  srv_addr.sin_port = htons(port);
  srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  // buffer
  char buf[BUFFER_SIZE] = {0};
  memset(buf, 0, sizeof(buf));

  // send request to server
  sendto(sock, "REQUEST", sizeof("REQUEST"), 0, (struct sockaddr *)&srv_addr, srv_len);
  printf("[INFO] Send request to client\n");

  // get file size from server
  uint64_t file_size;
  recvfrom(sock, &file_size, sizeof(file_size), 0, (struct sockaddr *)&srv_addr, &srv_len);

  // open file
  FILE* fp = fopen("recv.txt", "w");
  if (fp == NULL) {
    err("[ERR] File open error\n");
  }

  // progress record
  uint64_t curr_progress = 0;
  int partition_25 = file_size * 0.25;
  int partition_50 = file_size * 0.5;
  int partition_75 = file_size * 0.75;
  printf("[RECORD] 0%%  ");

  // time
  time_t timer;
  clock_t start, end;
  start = clock();
  struct tm* Now;

  time(&timer);
  Now = localtime(&timer);
  printf("%s", asctime(Now));

  // for loss
  uint64_t recv_size = 0;


  while (1) {
    // clear buffer
    memset(buf, 0, sizeof(buf));

    // get file content from server
    int recv_len = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&srv_addr, &srv_len);
    if (recv_len == -1) {
      err("[ERR] Receive error\n");
    }

    // if file transfer fininshed
    if (strcmp("EOF", buf) == 0) {
      fclose(fp);
      break;
    }

    // write to file
    int write_len = fwrite(buf, sizeof(char), recv_len, fp);
    recv_size += write_len;

    // progress record
    curr_progress += recv_len;
    if (partition_25 - BUFFER_SIZE / 2 <= curr_progress && curr_progress <= partition_25 + BUFFER_SIZE / 2) {
      printf("[RECORD] 25%%  ");
      time(&timer);
      Now = localtime(&timer);
      printf("%s", asctime(Now));
    }
    if (partition_50 - BUFFER_SIZE / 2 <= curr_progress && curr_progress <= partition_50 + BUFFER_SIZE / 2) {
      printf("[RECORD] 50%%  ");
      time(&timer);
      Now = localtime(&timer);
      printf("%s", asctime(Now));
    }
    if (partition_75 - BUFFER_SIZE / 2 <= curr_progress && curr_progress <= partition_75 + BUFFER_SIZE / 2) {
      printf("[RECORD] 75%%  ");
      time(&timer);
      Now = localtime(&timer);
      printf("%s", asctime(Now));
    }

  }

  // close socket
  printf("[RECORD] 100%%  ");
  time(&timer);
  Now = localtime(&timer);
  printf("%s", asctime(Now));
  end = clock();

  printf("[INFO] File transfer fininshed\n");
  printf("[INFO] Loss rate = %.2f%%\n", 100 * ((double) (file_size - recv_size) / (double) file_size));
  printf("[INFO] Loss size = %.2f MB\n", (double) (file_size - recv_size) / 1048576);
  printf("[INFO] File size = %.2f MB\n", (double) file_size / 1048576);
  printf("[INFO] Total time = %.2lf ms\n", (double) (end - start) / (CLOCKS_PER_SEC / 1000));
  close(sock);

}

// main function
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