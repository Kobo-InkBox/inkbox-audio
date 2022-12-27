#include <boost/algorithm/string/classification.hpp> // boost::is_any_of
#include <boost/algorithm/string/split.hpp>
#include <fstream>
#include <iostream>
#include <ostream>
#include <regex>
#include <string>

#include "functions.h"

string emitter = "functions";

// Global variables
bool logEnabled = false;

string socketPath = "/dev/iaudio.socket";

// Sound variables
// Those 2 need to be set just because to be sure its correct.
unsigned int rate;
unsigned int channels;
//

// Functions
void log(string toLog, string emitter) {
  if (logEnabled == true) {
    std::chrono::time_point<std::chrono::system_clock> end =
        std::chrono::system_clock::now();
    std::time_t endTime = std::chrono::system_clock::to_time_t(end);
    std::string message = normalReplace(std::ctime(&endTime), "\n", "\0") +
                          " | " + emitter + ": " + toLog;
    std::cout << message << std::endl;

    // TODO: Improve efficiency (don't close it every time)
    ofstream logFile("/var/log/inkaudio.log", ios::app);
    logFile << message << std::endl;
    logFile.close();
  }
}

string normalReplace(string mainString, string strToLookFor,
                     string replacement) {
  return std::regex_replace(mainString, std::regex(strToLookFor), replacement);
}

/*
Simplest possible config file, with name iaudio.conf, in the same directory as
the executable name of variable=value to set

Order is important, just as in example

so example:
rate=48000
channels=2

*/

void readConfig() {
  ifstream file("iaudio.conf");
  string line;
  int countLine = 0;
  // This could be in a big for loop with a fancy struct, but no reason to over
  // complicate things
  while (getline(file, line)) {
    log(to_string(countLine) + " line contains: " + line, emitter);
    if (countLine == 0) {
      string varToLook = "rate";
      if (normalContains(line, varToLook) == true) {
        vector<string> vectorToParse;
        boost::split(vectorToParse, line, boost::is_any_of("="),
                     boost::token_compress_on);

        rate = stoi(vectorToParse.back());
        log("Variable " + varToLook + " equals: " + to_string(rate));
      } else {
        log("Config misses " + varToLook + " at line " + to_string(countLine) +
                ", exiting",
            emitter);
        exit(-1);
      }
    }
    if (countLine == 1) {
      string varToLook = "channels";
      if (normalContains(line, varToLook) == true) {
        vector<string> vectorToParse;
        boost::split(vectorToParse, line, boost::is_any_of("="),
                     boost::token_compress_on);

        channels = stoi(vectorToParse.back());
        log("Variable " + varToLook + " equals: " + to_string(channels), emitter);
      } else {
        log("Config misses " + varToLook + " at line " + to_string(countLine) +
            ", exiting");
        exit(-1);
      }
    }
    countLine = countLine + 1;
  }
}

bool normalContains(string stringToCheck, string stringToLookFor) {
  if (stringToCheck.find(stringToLookFor) != std::string::npos) {
    return true;
  } else {
    return false;
  }
}
