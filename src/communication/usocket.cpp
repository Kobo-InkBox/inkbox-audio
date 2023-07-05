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
#include <experimental/filesystem>

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
extern mutex overAllMutex;

int sockfd;
struct sockaddr_un addr;

void createSocket() {
  if(ifstream(socketPath).good() == true) {
    log("Socket already exists. Service should have deleted it", emitter);
    exit(-1);
  }

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
    log("Created socket", emitter);
  }
}

/*
Messages:
// Play command combines the path with "/data/onboard/" so example:
play:"musicFolder/audioFile.wav" play:"/path/to/file" ( on the CLI, "" are ignored unless \"\")

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

    log("Receiving bytes", emitter);
    char buffer_tmp[1];
    vector<char> buffer;
    buffer.reserve(100);

    // This isin't efficient
    while (recv(client_sockfd, buffer_tmp, 1, 0) > 0) {
      // log("While loop going...", emitter);
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
    log("Message is: *" + message + "*", emitter);
    log("Command is: *" + command + "*", emitter);

    if (command == "play") {
      log("Found play command", emitter);
      string musicFilePath = "/data/onboard/";
      musicFilePath = musicFilePath + message.substr(5 + 1, message.size() - 5 - 1 - 1); // " characters are those 1, the last 1 is because size() is not position
      log("Music file path: " + musicFilePath, emitter);
      ifstream file(musicFilePath.c_str());
      if (file.good() == true) {
        overAllMutex.lock();
        bool lockedMutex1 = true;
        canBeContinued = false;
        if (isPLaying == true) {
          log("File is playing, pausing ( stopping ) it", emitter);
          pausePlay = true;
          lockedMutex1 = false;
          overAllMutex.unlock();
          audio.join();
        }
        if (threadToBeJoined == true) {
          log("File ended playing, now joining threads", emitter);
          threadToBeJoined = false;
          lockedMutex1 = false;
          overAllMutex.unlock();
          audio.join();
        }
        
        log("File exists", emitter);
        isPLaying = true; // Doesnt need mutex - thread doesn't exists yet
        if(lockedMutex1 == true) {
          overAllMutex.unlock();
        }
        audio = thread(playFile, musicFilePath);
      } else {
        log("File doesn't exist", emitter);
      }
    }
    if (command == "pause") {
      overAllMutex.lock();
      if (isPLaying == true) {
        log("Found pause command", emitter);
        pausePlay = true;
        overAllMutex.unlock();
        audio.join();
        overAllMutex.lock();
        isPLaying = false;
        overAllMutex.unlock();
      } else {
        log("Can't pause", emitter);
        overAllMutex.unlock();
      }
    }
    if (command == "continue") {
      log("Found continue command");
      overAllMutex.lock();
      if (isPLaying == false and canBeContinued == true) {
        continuePlay = true;
        canBeContinued = false;
        overAllMutex.unlock();
        audio = thread(playFile, recentFile);
      } else {
        log("Can't continue", emitter);
        overAllMutex.unlock();
      }
    }
    if (command == "set_volume") {
      log("Found set_volume command", emitter);
      vector<string> vectorToParse;
      boost::split(vectorToParse, message, boost::is_any_of(":"),
                   boost::token_compress_on);
      setVolumeLevel(atoi(vectorToParse.back().c_str()));
    }
  }
  // :)
  audio.join();
}
