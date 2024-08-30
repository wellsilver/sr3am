#ifndef _2dh
#define _2dh

struct sr2am {
  
  void *list;
};

struct fullrect {
  char type; // 1 for rect
  float x,y;
  float width,height;
};

struct sr2am *sr2amNew() {

}

int sr2amRect(struct sr2am list, float x, float y, float width, float height) {

}

void sr2amFree(int id) {

}

#endif