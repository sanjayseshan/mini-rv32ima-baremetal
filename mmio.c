int getchar() {
  return *(volatile int *)0xF000fff4;
}

int putchar(int c) {
  *(volatile int *)0xF000fff0 = c;
  return c;
}

int exit(int c) {
  *(volatile int *)0xF000fff8 = c;
  return c;
}
