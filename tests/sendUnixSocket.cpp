#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[])
{
  // Create the Unix domain socket
  int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sockfd < 0) {
    // Handle error
  }

  // Connect to the socket
  struct sockaddr_un addr;
  addr.sun_family = AF_UNIX;
  strcpy(addr.sun_path, "/dev/iaudio.socket");
  int res = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
  if (res < 0) {
    // Handle error
  }

  res = send(sockfd, std::string(argv[1]).c_str(), strlen(std::string(argv[1]).c_str()), 0);
  if (res < 0) {
    // Handle error
  }

  // Close the socket
  close(sockfd);

  return 0;
}
