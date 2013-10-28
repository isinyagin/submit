#include	<stdio.h>
#include	<ctype.h>
#include    <stdlib.h>
#include    <string.h>
#include    <stdbool.h>
#include	"wl.h"

/*
 *	wordlist	client
 *  modified    by Ivan Sinyagin
 *
 *	program reads words from standard input until EOF
 *	then prints all words it saw with the number of times
 *	each word appeared. Order is sorted
 *
 *	definitions and declarations are in wl.h
 */

#define DOUBLE    2
#define WORD_LEN  10

typedef struct string {
    char *data;
    size_t size; 
    size_t capacity;
} string_t;

/** dynamically allocates a string struct */
static string_t* string_init() {
    string_t *s = malloc (sizeof *s);
    if (!s) return NULL;
    s->data = NULL;
    s->size = 0;
    s->capacity = 0;
    return s;
}

/** get the string from the struct */
static char *string_get(string_t *s) {
    return s->data;
}

/** frees data and the string */
static void release_string(string_t *s) {
    free(s->data);
    free(s);
}

/** pushes c to the end of the data, constructing a dynamic string */
static void string_add(string_t *s, char c) {
    if (s->size == s->capacity) {
        // Double the capacity, or create an initial capacity of 20
        size_t new_capacity = s->capacity ? s->capacity * DOUBLE : WORD_LEN;
        s->data = realloc(s->data, sizeof(*s->data) * new_capacity);
        s->capacity = new_capacity;
    }

    s->data[s->size] = c;
    s->size++;
}

/** scanner reads one char and stores it into a dynamic string */
static char *scanner(string_t *str) {
    int c;
    while ((c = fgetc(stdin)) != EOF) {
        if (isspace(c)) {
            string_add(str, '\0');
            str->size = 0;
            str->capacity = 0;
            return string_get(str);
        }
        string_add (str, c);
    }

    return NULL;
}

/*
 * this function converts any string to all lower-case
 * letters.  Right now it is not being used since
 * the script that calls this program converts the text
 * anyway.
 */
static void convert_to_lower_case(char *str) {
	while(*str) {
		*str = tolower((int)*str);
		str++;
	}
}

/** client of wlfilter **/
int main() {
	char	*buf, *wordptr;
    string_t *string_buf = string_init();
    if (!string_buf) {
        fprintf(stderr,"out of memory\n");
        exit(1);
    }

	/* ... set up word filing module ... */
	table_t table = init_table();
    if (!table) {
        fprintf(stderr,"out of memory\n");
        exit(1);
    }

	/* ... read words and store them ... */
    while ((buf = scanner(string_buf))) {
	    convert_to_lower_case(buf);
		if (in_table(table, buf)) {
			if (!update(table, buf, 1 + lookup(table, buf))) {
                fprintf(stderr, "couldn't update the entry\n");
                exit(2);
            }
        } else if (!insert(table, buf, 1)) {
			fprintf(stderr,"wordfreq: out of memory\n");
			exit(1);
		}
	}

#if 0 /** for testing word_delete and destroy_table **/
	for (wordptr = firstword(table); wordptr != NULL; wordptr = nextword(table))
        if ('z' == *wordptr)
           word_delete(table, wordptr);
	for (wordptr = firstword(table); wordptr != NULL; wordptr = nextword(table))
		printf("%5d\t%s\n", lookup(table, wordptr), wordptr);
    destroy_table(table);
#endif
    
	/* ... move cursor down the table printing out results */
	for (wordptr = firstword(table); wordptr != NULL; wordptr = nextword(table))
		printf("%5d\t%s\n", lookup(table, wordptr), wordptr);

    release_string(string_buf);
	return 0;
}
