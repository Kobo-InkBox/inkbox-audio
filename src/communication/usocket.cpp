#include <boost/algorithm/string/classification.hpp> // boost::is_any_of
#include <boost/algorithm/string/split.hpp>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <vector>

#include "../functions.h"
#include "../sound/play.h"
#include "usocket.h"

extern string socketPath;
const string emitter = "usocket";

// this doesn't need mutexes because its writes are controlled
extern bool pausePlay;
extern unsigned int fileOffsetPause;
extern bool continuePlay;
extern string recentFile;
extern bool isPLaying;
extern bool threadToBeJoined;
extern bool canBeContinued;

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

/*
Messages:
// Play command combines the path with "/data/onboard/" so example:
play:"musicFolder/audioFile.wav" play:"/path/to/file"

pause:
continue:
set_volume:69
*/
void listenSocket() {
  thread audio;
  while (true) {
    log("Listening to connections", emitter);
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
    string command = "";
    bool stop_command = false;

    for (int i = 0; i < buffer.size(); i++) {
      message = message + buffer[i];
      if (stop_command == false) {
        if (buffer[i] == ':') {
          stop_command = true;
        } else {
          command = command + buffer[i];
        }
      }
    }
    log("Message is: \"" + message + "\"", emitter);
    log("Command is: \"" + command + "\"", emitter);

    if (command == "play") {
      log("Found play command");
      string musicFilePath = "/data/onboard/";
      musicFilePath = musicFilePath + message.substr(5, message.size());
      log("Music file path: " + musicFilePath);
      ifstream file(musicFilePath.c_str());
      if (file.good() == true) {
        canBeContinued = false;
        if (isPLaying == true) {
          log("File is playing, pausing ( stopping ) it");
          pausePlay = true;
          audio.join();
        }
        if (threadToBeJoined == true) {
          log("File ended playing, now joining threads");
          threadToBeJoined = false;
          audio.join();
        }
        log("File exists");
        audio = thread(playFile, musicFilePath);
        isPLaying = true;
      } else {
        log("File doesn't exist");
      }
    }
    if (command == "pause") {
      if (isPLaying == true) {
        log("Found pause command");
        pausePlay = true;
        audio.join();
        isPLaying = false;
      } else {
        log("Can't pause", emitter);
      }
    }
    if (command == "continue") {
      log("Found continue command");
      if (isPLaying == false and canBeContinued == true) {
        continuePlay = true;
        canBeContinued = false;
        audio = thread(playFile, recentFile);
      } else {
        log("Can't continue", emitter);
      }
    }
    if (command == "set_volume") {
      log("Found set_volume command");
      vector<string> vectorToParse;
      boost::split(vectorToParse, message, boost::is_any_of(":"),
                   boost::token_compress_on);
      setVolumeLevel(atoi(vectorToParse.back().c_str()));
    }
  }
  // :)
  audio.join();
}
