#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include    <stdbool.h>
#include    <ctype.h>
#include	"wl.h"
/*
 *	wordlist table module version 5 (by Ivan Sinyagin)
 *
 *	interface functions are:
 *		init_table()                     - set up this module
 *	    in_table(str)                    - sees if word is in table
 *	  	insert(str, val)                 - insert value into table
 *		lookup(str)                      - retrieve value
 *		update(str, val)                 - update value in table
 *	  	firstword()                      - return first word in table
 *		nextword()                       - return next word in table
 *      destroy_table(table)             - release the table
 * static functions are:
 *      get_list(str)                    - get list from the table based on *str
 *      dummy()                          - make a dummy head/tail node
 *      free_link (link)                 - free a link
 *      check_for_word (str)             - check that str is valid word
 *      search(list, str)                - search for a str in the list
 *      remove_link (list, link)         - remove link from list 
 *      construct_link(str, val)         - make a link with str and val
 *      switch_to_data_list(index)       - get the next data list from table
 *      add_link(list, newlink, current) - add link to the list
 *      destroy_list (list)              - release the list
 */

#define FIRST_LIST        0               /* index of the first list */
#define DUMMY_NODES_ONLY  2               /* number of dummy nodes */
#define ASCII_A           97              /* lower case ascii 'a' int value */

#define NEXT_INDEX(x)     (x+1)           /* macro for readability */

/***                                ***/
/*** Hash Table static functions ***/
/***                                ***/

/** dummy returns a dummy head/tail link */
static link_t* dummy() {
    link_t *dummy = malloc (sizeof *dummy); /* allocate a link */
    if (!dummy)
        return NO_LINK;
    dummy->value = 0;                       
    dummy->word = NULL;
    dummy->next = NO_LINK;
    dummy->prev = NO_LINK;
    return dummy;
}

/** add_link inserts 'newlink' after 'current' link in the 'list' */
static void add_link(list_t *list, link_t *newlink, link_t *current) {
    newlink->prev = current;                
    newlink->next = current->next;
    current->next->prev = newlink;          
    current->next = newlink;
    list->size++;                           /* increment the list size */
}

/** free_link releases memory of the 'link' */
static void free_link (link_t *link) {
    free(link->word);                       /* free the word */
    free(link);                             /* free the link itself */
}

/** remove_link removes 'link' from the 'list' */
static void remove_link (list_t *list, link_t *link) {
    link->prev->next = link->next;
    link->next->prev = link->prev;
    link->next = NO_LINK;                      
    link->prev = NO_LINK;
    list->size--;                           /* decrement the size */
    free_link(link);                        /* free the link */
}

/** destroy_list releases the memory of the 'list' */
static void destroy_list (list_t *list) {
    if (list->size > DUMMY_NODES_ONLY) {    /* list with data */
        link_t *link = list->first->next;   /* point to the first data link */
        while (link->next) {                /* next link exists */
            link = link->next;              /* move ptr to it */
            remove_link(list, link->prev);  /* release the prev link */
        }
    }
    list->first->next = NO_LINK;            /* disconnect dummy head */
    list->last->prev = NO_LINK;             /* disconnect dummy tail */
    free_link(list->first);                 /* release head */
    free_link(list->last);                  /* release tail */
}

/** construct_link returns a created link_t pointer with 'str' and 'val' data */
static link_t* construct_link(char *str, int val) {
	char *newstr;
	link_t *newlink;

	newstr = malloc(strlen(str) + 1);	    /* get memory for str	*/
	if (!newstr) 			                /* or die */
		return NO_LINK;
	strcpy (newstr, str);                   /* copy to mem */

	newlink = malloc(sizeof(link_t));       /* get mem for link	*/
	if (newlink == NO_LINK) {                         
        free(newstr);                       /* free allocated newstr */
		return NO_LINK;                     /* die */
    }

	newlink->word  = newstr;                /* put str in struct */
	newlink->value = val ;                  /* put val in struct */
    newlink->next = NO_LINK;
    newlink->prev = NO_LINK;
    return newlink;
}

/** check_for returns true if 'str' starts with an akphabetic character */
static bool check_for_word (char *str) {
    if (!isalpha(*str))
        return false;
    return true;
}

/** search returns a link_t pointer to the link in the 'list' with 'str' data;
 *  returns NULL or error or not found */
static link_t* search(list_t *list, char *str) {
    if (list == NO_LIST)                    /* bad list ptr */
        return NO_LINK;

    if (!check_for_word(str))               /* not a word */
        return NO_LINK;

    if (list->size == DUMMY_NODES_ONLY)     /* 'empty' list */
        return NO_LINK;

    link_t *link = list->first->next;       /* first data link */
    while (link) {
        if (link->word && !strcmp (link->word, str))  /* found the link */
            break;                                    
        link = link->next;                            /* move the ptr */
    }
    return link;                               
}

/** get_list returns a list_t pointer to the 'table' row based on the first
 *  character of the 'str'. NULL if 'str' is not a word
 */
static list_t* get_list(table_t table, char *str) {
    if (!check_for_word(str))               /* not a word */
        return NO_LIST;                        /* return */
    return (*table) + (*str - ASCII_A);     /* get the row from the table */
}

/** switch_to_data_list returns a pointer to the list_t object to the 'table'
 * row with 'index' index*/
static list_t* switch_to_data_list(table_t table, int index) {
    list_t *current_list = *table + index;  /* next list in the table */
    while (current_list->size == DUMMY_NODES_ONLY && ++index < TABLE_SIZE) 
        current_list = *table + index;      /* move if it has no data links */
    return current_list;                    
}

/***                                ***/
/*** Hash Table interface functions ***/
/***                                ***/

/** init_table returns a ptr to the initialized table_t object or NULL on error */
table_t init_table() {
    table_t table = malloc (sizeof *table); /* allocate space for this table */
    if (!table)
        return NO_TABLE;

    int i;
    for (i = 0; i < sizeof *table /sizeof (list_t); i++) { /* initialize all lists */
        (*table+i)->index = i;                       /* index in the table */
        (*table+i)->size = DUMMY_NODES_ONLY;         /* initial size */
        (*table+i)->first = dummy();                 /* dummy tail */
        (*table+i)->last = dummy();                  /* dummy head */
        if ((*table+i)->first == NO_LINK || (*table+i)->last == NO_LINK)
            return NO_TABLE;
        (*table+i)->first->next = (*table+i)->last;  /* connect dummies */
        (*table+i)->last->prev = (*table+i)->first;  /* connect dummies */
    }
    return table;
}

/** in_table returns true if 'str' is in the 'table', false otherwise or on error */
int in_table(table_t table, char *str) {
    if (table == NO_TABLE || str == NULL)
        return false;

    if (search(get_list(table, str), str))        /* if found  */
        return true;                              /* ret value */
    return false;                                 /* not found */
}

/** lookup searches in the 'table' for the value field associated with 'str' */
int lookup(table_t table, char *str) {
    if (table == NO_TABLE || str == NULL)
        return 0;

	link_t *link;

    if ((link = search(get_list(table, str), str)))     /* if found  */
        return link->value;                             /* ret value */
    return 0;                                           /* not found */
}

/** update updates the value of the 'str' link in the 'table' by 'val' */
int update(table_t table, char *str, int val) {
    if (table == NO_TABLE || str == NULL)
        return false;

	link_t *linkp;

    if ((linkp = search(get_list(table, str), str))) {  /* if found */
        linkp->value = val;                       /* update */
        return true;                              /* and go */
    }                                                              
    return false;                                 /* not found */
}

/** insert returns false(NO) if no more memory, true(YES) if successfully
 * inserted a link with 'str' and 'val' data into 'table' */
int insert(table_t table, char *str, int val) {
    if (table == NO_TABLE || str == NULL)
        return false;

    if (!check_for_word(str))       /* if not a word */
        return true;                /* don't add, return true not to fail */

    link_t *newlink = construct_link(str, val);   /* make a new link */
    if (!newlink)                                 /* if no memory */
        return false;                               
    
    list_t *current_list = get_list(table, str);  /* get the list from table */
    link_t *current = current_list->last->prev;   /* ptr to last real link */
    if (current_list->size > DUMMY_NODES_ONLY && 
        strcmp(current->word, str) < 0) { /* insert at the end without search*/
        add_link(current_list, newlink, current);   
    } else {                                      /* traverse the list */
        int list_mem;                             /* flag to check membership */ 
        current = current_list->first;            /* dummy head node */
        while (current->next && current->next->word &&
               (list_mem = strcmp(current->next->word, str)) < 0) {
            current = current->next;
            if (!list_mem) {          /* word is already in the list */
                free_link(newlink);   /* free the constructed link */
                current->value++;     /* update the value counter */
                return true;          /* don't return 'out of memory' error */
            }
        }
        add_link(current_list, newlink, current); 
    }
    return true;                      /* inserted */
}

/** word_delete removes a link from the 'table' with 'str' object */
void word_delete(table_t table, char *str) {
    if (table == NO_TABLE || str == NULL) return;

    list_t *list = get_list(table, str);     /* get the list */
    if (!list) {                      /* str is not a word */
        fprintf(stderr, "[not a word] %s\n", str);
        return;
    }

    link_t *link;
    if ((link = search(list, str)))   /* str is in the list */
        remove_link(list, link);      /* remove it */
    else                              /* output an error message */
      fprintf(stderr, "[not in the table] %s\n", str);
}

/** destoy_table releases the memory of the 'table' */
void destroy_table (table_t table) {
    if (table == NO_TABLE) return;

    int i;
    for (i = 0; i < sizeof *table / sizeof (list_t); i++)
        destroy_list(*table+i);
    free(table);
}


/** firstword returns nextword from the 'table' */
char *firstword(table_t table) {
    if (table == NO_TABLE)
        return NULL;

	return nextword(table);           /* nextword() does all the work */
}

/** nextword returns word from the next link in the 'table' */
char *nextword(table_t table) {
    if (table == NO_TABLE)
        return NULL;

    static list_t *current_list;      /* save cur_list between calls */
    static link_t *current_link;      /* save cur_link between calls */

    if (!current_list) {              /* first call */
        current_list = switch_to_data_list(table, FIRST_LIST);   /* first list */
        current_link = current_list->first->next;         /* first data link */
    }
    
    if (current_link->value == 0) {   /* dummy tail of the list */
        if (NEXT_INDEX(current_list->index) == TABLE_SIZE) {  /* end of table */
            current_list = NO_LIST;   /* reset for next firstword() call*/
            return NULL;              /* cease traversal */
        } else {                      /* switch to next data list */
            current_list = 
                switch_to_data_list(table, NEXT_INDEX(current_list->index));
            current_link = current_list->first->next;   /* first data link */
            if (NEXT_INDEX(current_list->index) == TABLE_SIZE && /* last list */
                current_link->value == 0) { /* in the table without data*/
                current_list = NO_LIST;     /* reset for next firstword() call*/
                return NULL;                /* cease traversal */
            }
        }
    }

	char *ret_val = current_link->word;     /* save word to return */
	current_link = current_link->next;      /* move the cur_link ptr */
	return ret_val;
}
