// Socket things
#include <cstring>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <vector>

#include "../functions.h"
#include "usocket.h"

extern string socketPath;
const string emitter = "usocket";

int sockfd;
struct sockaddr_un addr;

void createSocket() {
  // Create the socket.
  sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sockfd < 0) {
    log("Error creating socket", emitter);
  }

  // Set up the sockaddr_un structure.
  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, socketPath.c_str(), sizeof(addr.sun_path) - 1);

  // Bind the socket to the sockaddr_un structure.
  if (bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) < 0) {
    log("Error binding socket", emitter);
  } else {
    log("Created socket");
  }
}

void listenSocket() {
  log("Listening to connections");
  int result = listen(sockfd, 1);
  if (result < 0) {
    log("Failed to listen to socket", emitter);
  } else {
    log("New connection", emitter);
  }

  int client_sockfd = accept(sockfd, nullptr, nullptr);
  if (client_sockfd < 0) {
    log("Failed to accept connection to socket", emitter);
  } else {
    log("New client", emitter);
  }

  log("Receiving bytes");
  char buffer_tmp[1];
  vector<char> buffer;

  // This isin't efficient
  ssize_t n;
  while ((n = recv(client_sockfd, buffer_tmp, 1, 0)) > 0) {
    buffer.push_back(buffer_tmp[0]);
  }

  log("Received bytes: " + to_string(buffer.size()), emitter);
  string message = "";
  for(int i = 0; i < buffer.size(); i++) {
    message = message + buffer[i];
  }
  log("Message is: \"" + message + "\"", emitter);
}
