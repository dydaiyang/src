#include <stdio.h>

static struct sss{
  char f;
  struct {char * m;} snd;
} sss;

#define _offsetof(st,f) ((char *)&((st *) 16)->f - (char *) 16)

int main (void) {
  printf ("+++Struct pointer inside struct starting with char:\n");
  printf ("size=%d,align=%d\n", sizeof (sss), __alignof__ (sss));
  printf ("offset-char=%d,offset-sss-pointer=%d,\nalign-char=%d,align-sss-pointer=%d\n",
          _offsetof (struct sss, f), _offsetof (struct sss, snd),
          __alignof__ (sss.f), __alignof__ (sss.snd));
  return 0;
}
