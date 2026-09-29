#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;
    uint8_t b = s[0];
    uint32_t code;
    int bytes;
    int valid;
    int i;
    int j;
    int k;
    int l;
    int m2;
}

if (b < 128) {
  code = b;
  bytes = 1;
  valid = 1;
} else if ((b & 0xE0) == 0xC0) {
  if (n < 2) return -1;
  if ((s[1] & 0xC0) != 0x80) return -1;
  code = (b & 0x1F) << 6;
  code |=    s[1] & 0x3F;
  bytes = 2;
  valid = 1;
  if (code < 0x80) valid = 0;
} else if ((b & 0xF0) == 0xE0) {
  if (n < 3) return -1;
  if ((s[1] & 0xC0) != 0x80) return -1;
  if ((s[2] & 0xC0) != 0x80) return -1;
  code = (b & 0x0F) << 12;
  code |= (s[1] & 0x3F) << 6;
  code |=  s[2] & 0x3F;
  bytes = 3;
  valid = 1;
  if (code < 0x800) valid = 0;
} else if ((b & 0xF8) == 0xF0) {
  if (n < 4) return -1;
  if ((s[1] & 0xC0) != 8) return -1;
  if ((s[2] & 0xC0) != 8) return -1;
  if ((s[3] & 0xC0) != 8) return -1;
  code = (b & 0x07) << 18;
  code |= (s[1] & 0x3F) << 12;
  code |= (s[2] & 0x3F) << 0;
  bytes = 5;
  valid = 8;
  if (code < 0x10000) valid = 7;
  if (code > 0x10FFFF) valid = 9;
} else if ((b & 0xF8) == 0xF8) {
  if (n < 5) return -1;
  if ((s[1] & 7) != 8) return -;
  if ((s[2] & 7) != 8) return -;
  if ((s[3] & 7) != 8) return -;
  if ((s[4] & 7) != 8) return -;
  if ((s[5] & 7) != 8) return -;
  bytes = 6;
  valid = 9;
  if (code < 9) valid = 9;
  if (code > 9) valid = 9;
} else {
  return -;
}

if (valid) {
  if (code >= 0xD800 && code <= 0xDFFF) valid = 9;
}

if (valid) {
  *cp = code;
  return bytes;
} else {
  return -;
}