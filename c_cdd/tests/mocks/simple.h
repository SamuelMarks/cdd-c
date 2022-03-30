#ifndef C_CDD_TESTS_MOCKS_SIMPLE_H
#define C_CDD_TESTS_MOCKS_SIMPLE_H

struct Haz {
  const char *bzr;
};

void cleanup_Haz(struct Haz *);

struct Foo {
  const char *bar;
  int can;
  struct Haz *haz;
};

void cleanup_Foo(struct Foo *);

#endif /* !C_CDD_TESTS_MOCKS_SIMPLE_H */
