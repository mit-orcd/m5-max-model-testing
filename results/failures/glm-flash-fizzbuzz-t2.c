#include <stddef.h>

const char *fizzbuzz(int n) {
	static char buffer[16];
	if (n % 15 == 0) {
		return "FizzBuzz";
	    } else if (n % 3 == 0) {
		return "Fizz";
	    } else if (n % 5 == 0) {
		return "Buzz";
	    } else {
		int length;
		if (n < 100000000) {
			length = (n < 10) ? 1 : (n < 100) ? 2 : (n < 10000) ? 5 : 9;
			snprintf(buffer, sizeof(buffer), "%d", n);
		} else {
			snprintf(buffer, sizeof(buffer), "%d", n);
		}
		return buffer;
	}
}