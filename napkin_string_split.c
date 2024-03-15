
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int split_string_by_delimiters(char *a, size_t n)
{
	char temp[128];
	int word_head = -1, word_tail = -1;
	int slice_now = 0;
	size_t word_count = 0;
	for (int i = 0; i < n; i++) {
		if (word_head == -1)
			word_head = i;
		if (a[i] == ' ' | a[i] == ',' | a[i] == ';' | a[i] == ':' |
		    a[i] == '\n') {
			// if we read a delimeter we reached the possible end of a word
			if (word_tail == -1) {
				// ... except multiple non-word chars
				word_head = -1;
				continue;
			} else {
				// ... signal slicing
				slice_now = 1;
			}
		} else {
			word_tail = i;
		}
		// handle slicing in case of being signaled,
		// or we reached the end of string
		if (slice_now || i + 1 == n) {
			word_count += 1;
			printf("head %d tail %d\n", word_head, word_tail);
			memset(&temp, '\0', 128);
			memcpy(&temp, &a[word_head], word_tail - word_head + 1);
			printf("%s\n", temp);
			word_tail = -1;
			word_head = -1;
			slice_now = 0;
		}
	}
	printf("Done. Word count: %ld\n\n", word_count);
	return word_count;
}

int main()
{
	char s[] = "  , word1, word2 word3 \n word4 , ";
	printf("strlen %ld: \n###\n%s\n###\n", strlen(s), s);
	split_string_by_delimiters(s, strlen(s));	
	char s2[] = "  ,  \n , ";
	printf("strlen %ld: \n###\n%s\n###\n", strlen(s2), s2);
	split_string_by_delimiters(s2, strlen(s2));
}