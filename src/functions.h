#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string>
#include <mutex>

using namespace std;

void log(string message, string emitter = "undefined");
string normalReplace(string mainString, string strToLookFor, string replacement);
void readConfig();
bool normalContains(string stringToCheck, string stringToLookFor);

#endif
