#ifndef LIST_H
#define LIST_H

typedef struct link {                     /* data structure for a list node */
    int	value;                            /* the count	*/
    char *word;                           /* the string	*/
    struct link *next;                    /* the next one	*/
    struct link *prev;                    /* the previous one */
} link_t;

typedef struct list {                     /* data structure for the list */
    link_t *first;                        /* the first link */
    link_t *last;                         /* the last link */
    size_t index;                         /* index of the list in the table */
    size_t size;                          /* number of nodes in the list */
} list_t; 

#define TABLE_SIZE  26                    /* number of rows in the table */
typedef list_t (*table_t)[TABLE_SIZE];    /* ptr to an array of 26 list_t */

#define	YES	        1
#define	NO	        0
#define NO_LINK     ((link_t*)0)           /* in case NULL is undefined */
#define NO_LIST     ((list_t*)0)           /* in case NULL is undefined */
#define NO_TABLE    ((table_t)0)          /* in case NULL is undefined */

table_t init_table();
int in_table(table_t, char []);
int insert(table_t, char [], int);
int	lookup(table_t, char []);
int update(table_t, char [], int );
char *firstword(table_t);
char *nextword(table_t);
void word_delete(table_t, char []);
void destroy_table(table_t);

#endif  /* LIST_H */
