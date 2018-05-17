#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#include "Connect/Connect.h"

int main(int argc, char* argv[]){
  
  int client_socket, tracker_socket,  port = PORT,
    tracker_port = port + 1, client_number, bytes_remaining, bytes_rcvd;
  size_t bytes_to_receive;
  struct sockaddr_in server_addr, tracker_addr;
  char buffer[BUF], peers[BUF*2], *ip = IP, *ptr;
  FILE *fp;
  
  for(int i = 1; i < argc; i += 2)
    if(!strcmp(argv[i],"-i") && argc >= i + 1)
      ip = argv[i + 1];
    else if(!strcmp(argv[i],"-p") && argc >= i + 1)
      port = atoi(argv[i + 1]);
    else{
      printf("Usage: %s [-i] ip_addr [-p] port\n", argv[0]);
      printf("Default: ip = 127.0.0.0.1, port = 4444\n");
      exit(0);
    }
  
  client_socket = create_socket();
  server_addr = create_socket_address(port, ip);
  connect_to_server(client_socket,(struct sockaddr*)&server_addr);

  tracker_socket = create_socket();
  tracker_addr = create_socket_address(tracker_port, ip);
  connect_to_server(tracker_socket,(struct sockaddr*)&tracker_addr);

  assert(recv(client_socket, buffer, BUF, 0) > 0);
  
  ptr = strtok(buffer, "#");
  ptr = strtok(NULL,  "#");
  ptr = strtok(ptr, ".");
  assert(ptr != NULL);
  client_number = atoi(ptr);

  ptr = strtok(NULL, ".");
  ptr = strtok(ptr, "(");
  ptr = strtok(NULL, "(");
  ptr = strtok(ptr, ")");
  assert(ptr != NULL);
  bytes_to_receive = bytes_remaining = atoi(ptr);

  fprintf(stdout, "[+] You are client %d. Receiving %lu bytes.\n", client_number, bytes_to_receive);

  memset(buffer, 0, sizeof(buffer));
  sprintf(buffer, "%d", client_number);
  
  assert((fp = fopen(buffer, "w")) != NULL);

  while((bytes_rcvd = recv(client_socket, buffer, BUF, 0)) > 0 &&
	bytes_remaining > 0){
    fwrite(buffer, sizeof(char), bytes_rcvd,fp);
    bytes_remaining -= bytes_rcvd;
    fprintf(stdout, "[+] %lf%c: ", 100.0 * (1 - (double)bytes_remaining/bytes_to_receive), '%');
    fprintf(stdout, "Received %lu bytes from server\n", bytes_to_receive - bytes_remaining);
  }

  
  memset(peers, 0, sizeof(peers));
  assert(recv(tracker_socket, peers, sizeof(peers), 0) > 0);
  printf("Received: %s\n", peers);
  
  fclose(fp);
  close(client_socket);
  close(tracker_socket);
  return 0;
}
