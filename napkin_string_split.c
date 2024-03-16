
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORD_BUFF_LEN 8
#define MIN(a,b) ((a) < (b) ? a : b)

int split_string_by_delimiters(char *a, size_t n)
{
	char temp[MAX_WORD_BUFF_LEN];
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
			size_t len = MIN(MAX_WORD_BUFF_LEN, word_tail - word_head + 1);

			printf("head %d tail %d\n", word_head, word_tail);
			memset(&temp, '\0', MAX_WORD_BUFF_LEN);
			memcpy(&temp, &a[word_head], len);
			printf("%s\n", temp);
			word_tail = -1;
			word_head = -1;
			slice_now = 0;
		}
	}
	printf("Done. Word count: %ld\n", word_count);
	return word_count;
}

int main()
{
	char s1[] = "";
	printf("strlen %ld: \n###\n%s\n###\n", strlen(s1), s1);
	split_string_by_delimiters(s1, strlen(s1));
	printf("-------------------------------------------------\n");

	char s2[] = "  ,  \n , 123456789 987654321";
	printf("strlen %ld: \n###\n%s\n###\n", strlen(s2), s2);
	split_string_by_delimiters(s2, strlen(s2));
	printf("-------------------------------------------------\n");

	char s3[] = "  , word1, word2 word3 \n word4 , ";
	printf("strlen %ld: \n###\n%s\n###\n", strlen(s3), s3);
	split_string_by_delimiters(s3, strlen(s3));	
}