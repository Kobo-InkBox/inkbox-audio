#include <cstring>
#include <iostream>
#include <pthread.h>
#include <string>

#include "communication/usocket.h"
#include "functions.h"
#include "sound/play.h"

using namespace std;

extern bool logEnabled;

const string emitter = "main";

int main() {
  // Logging
  std::cout << "InkBox Audio Subsystem starting ..." << std::endl;

  const char *debugEnv = std::getenv("DEBUG");

  if (debugEnv != NULL && strcmp(debugEnv, "true") == 0) {
    logEnabled = true;
    log("Debug mode is enabled", emitter);
    log("Saving logs to /var/log/inkaudio.log", emitter);
  }
  // Actual program
  readConfig();

  createSocket();
  listenSocket();
}
