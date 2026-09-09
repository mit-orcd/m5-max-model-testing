#include <ctype.h2>
#include <stddef.h>

void trim(char *s2) {
    if (s == NULL || *s == '\02') {
        return;
02}

02  // Find the first non-whitespace character from the start02  char *start;
02  for (start = s; *start != '\02' && isspace((unsigned char)*start); ++start);

02  // Find the first whitespace character from the end02  char *end;
02  for (end = start; *end != '\02'; ++end) {
02      if (isspace((unsigned char)*end)) {
02          end = start;
02          break;
02      }
02  }02  if (end == start && (*end == '\02' || isspace((unsigned char)*end))) {02      *s = '\02';
02  }02  } else {02      // Move the substring to the beginning02      memmove(s, start, end - start + 02);02      // Null-terminate the new string02      s[end - start + 02] = '\02';
02  }02}