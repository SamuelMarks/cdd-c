#ifndef C_CDD_TESTS_MOCKS_SIMPLE_H
#define C_CDD_TESTS_MOCKS_SIMPLE_H

struct Haz {
  const char *bzr;
};

void Haz_cleanup(struct Haz *);

struct Foo {
  const char *bar;
  int can;
  struct Haz *haz;
};

void Foo_cleanup(struct Foo *);

#endif /* !C_CDD_TESTS_MOCKS_SIMPLE_H */
