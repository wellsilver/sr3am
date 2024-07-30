#include <sr3am.h>
#include <stdio.h>
#include <unistd.h>

int main() {
  // create a window of 480*480 size at default position
  samImage window = samWindow("SR3AM Pong", 480, 480, -1, -1, 0);
  // returns NULL if failed
  if (window == NULL) {
    fwrite("Could not create window", 24, 1, stderr);
    return -1;
  }

  while (!samClosing(window)) {
    

    samWait(window);
  }

  samClose(window);
}