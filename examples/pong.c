#include <sr3am.h>
#include <stdio.h>
#include <unistd.h>

int main() {
  // create a window of 480*480 size at default position
  samImage window = samWindow("SR3AM Pong", 480, 480, -1, -1, 0);
  // returns NULL if failed
  if (window == NULL) {
    printf("Could not create window");
    return -1;
  }

  while (!samClosing(window)) {
    
    
  }

  sleep(5000);

  samClose(window);
}