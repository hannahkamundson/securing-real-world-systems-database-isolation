/*
 NAME:			Jon Lovering
 FILE:			list.h
 DESCRIPTION:   	this is the header for the list library
 NOTES:			none
 DATE:			30/12/05
 CONTACT:			jonlovering@ieee.org
*/


#include <stdio.h>
#include <stdlib.h>

#ifndef LISTH
#define LISTH

#ifndef MIN_NODES
#define MIN_NODES 20
#endif

#ifndef MIN_LISTS
#define MIN_LISTS 2
#endif

#ifndef MAX_NODES
#define MAX_NODES 5120
#endif

#ifndef MAX_LISTS
#define MAX_LISTS 100
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/*
 STRUCTURE:		_node (alias NODE)		
 DESCRIPTION:		this is the structure of every linklist node
 NOTES:			none
*/
struct _node {
	void *item;
	struct _node *next;
	struct _node *prev;

	struct _node *master_next;
	char in_use;
};

typedef struct _node NODE;

/*
 STRUCTURE:		_LIST (alias LIST)		
 DESCRIPTION:		this is the structure of every linklist header (list
 	type)
 NOTES:			none
*/
struct _LIST {
	NODE *first;
	NODE *last;
	NODE *cur;
	int listcount;

	int handle;
	struct _LIST *master_next;
	char in_use;
};

#ifndef _LIBROBUSTA_JVM_H_
typedef struct _LIST LIST;
#endif

/*
 STRUCTURE:		_fnode
 DESCRIPTION:		this is repository of nodes
 			listnode_num keeps track of the total number of listnode used
			node_capacity keeps track of the number of nodes allowed
 NOTES:			none
*/
struct _fnode {
	NODE *first;
	NODE *cur;
	int listnode_num;
	int node_capacity;
} repository;

/*
 STRUCTURE:		_fLIST
 DESCRIPTION:		this is repository of List headers
 			listheader_num keeps track of the total number of listheader used
			list_capacity keeps track of the number of lists allowed
 NOTES:			none
*/
struct _fLIST {
	LIST *first;
	LIST *cur;
	int listheader_num;
	int list_capacity;
	LIST **handles;
} list_repository;

/*
The following function are list movers, and are implemented in list_movers.c
	ListCount
	ListFirst
	ListLast
	ListNext
	ListPrev
	ListCurr
	ListSearch
*/

/*
 FUNCTION:		ListCount	
 DESCRIPTION:		return the number of items in the list
 ARGUMENTS:		the list to be counted
 RETURNS:		an int, the count of the list: -1 if the list is NULL
 NOTES:			none
 */
extern int ListCount(LIST *wList);

/*
 FUNCTION:		ListFirst
 DESCRIPTION:		return the first item in the list, and make the first
 	node the current one
 ARGUMENTS:		the list to manipulate
 RETURNS:		an void pointer, to the first item: NULL if the first
 	item does not exist
 NOTES:			none
 */
extern void *ListFirst(LIST *wList);

/*
 FUNCTION:		ListLast
 DESCRIPTION:		return the last item in the list, and make it the
 	current item.
 ARGUMENTS:		the list to manipulate
 RETURNS:		an void pointer, to the last item: NULL if the last
 	item does not exist.
 NOTES:			none
 */
extern void *ListLast(LIST *wList);

/*
 FUNCTION:		ListNext
 DESCRIPTION:		make the next item the current item, and return it.
 ARGUMENTS:		the list to manipulate
 RETURNS:		an void pointer, to the next item.
 			NULL if the next item is off the list
 NOTES:			none
 */
extern void *ListNext(LIST *wList);

/*
 FUNCTION:		ListPrev
 DESCRIPTION:		make the previous item the current item, and return it
 ARGUMENTS:		the list to manipulate
 RETURNS:		a void pointer to the previous item in the list.
 			NULL if the previous item is off the list
 NOTES:			none
 */
extern void *ListPrev(LIST *wList);

/*
 FUNCTION:		ListCurr
 DESCRIPTION:		return the current item in the list
 ARGUMENTS:		the list to manipulate
 RETURNS:		a void pointer to the current item in the list: 
 			NULL if the current item does not exist.
 NOTES:			none
 */
extern void *ListCurr(LIST *wList);

/*
 FUNCTION:		ListSearch
 DESCRIPTION:		locates an item in the list.  The item is determined
 	by a compaarator function passed to the function, and a comparisonArg
	passed to the function.
 ARGUMENTS:		a list, the list to manipulate; a function pointer,
 	the comparison opperation; a comparisonArg.
 RETURNS:		a void pointer, to the item searched for.
 			NULL if the item is not found.
 NOTES:			none
 */
extern void *ListSearch(LIST *wList, int (*comparator)(void *item1, void
*item2), void *compareTo);
 
/*
The following functiona are list adders, and are implemented in list_adders.c
	ListCreate
	ListGetNode
	ListAdd
	ListInsert
	ListAppend
	ListPrepend
	ListConcat
*/

/*
 FUNCTION:		ListCreate
 DESCRIPTION:		creates a new list
 ARGUMENTS:		none
 RETURNS:		a list structure, the base of the new list	
 NOTES:			this function will also define the node repository for
 	this list
 */
extern LIST *ListCreate(void);

/*
 FUNCTION:		ListAdd
 DESCRIPTION:		inserts an item directly after the current item	
 ARGUMENTS:		the list to manipulate; an item, the item to add.
 RETURNS:           an int, the success of the function: -1 for failure,
 	0 for success
 NOTES:			none
 */
extern int ListAdd(LIST *wList, void *item);

/*
 FUNCTION:		ListInsert
 DESCRIPTION:		inserts an item directly before the current item	
 ARGUMENTS:		the list to manipulate; an item, the item to add.
 RETURNS:           an int, the success of the function: -1 for failure,
 	0 for success
  NOTES:			none
 */
 extern int ListInsert(LIST *wList, void *item);

/*
 FUNCTION:		ListAppend
 DESCRIPTION:		inserts an item at the end of the list
 ARGUMENTS:		the list to manipulate; an item, the item to add.
 RETURNS:           an int, the success of the function: -1 for failure,
 	0 for success
 NOTES:			none
 */
extern int ListAppend(LIST *wList, void *item);

/*
 FUNCTION:		ListPrepend
 DESCRIPTION:		inserts an item at the begining of the list (before
 	the first item)
 ARGUMENTS:		the list to manipulate; an item, the item to add.
 RETURNS:           an int, the success of the function: -1 for failure,
 	0 for success
 NOTES:			none
 */
extern int ListPrepend(LIST *wList, void *item);

/*
 FUNCTION:		ListConcat
 DESCRIPTION:		concatinates two lists together.  The second list will
 	no longer exist, and the current item will be the current item of the 
	first list
 ARGUMENTS:		a list, the base list; a list, the list to be added to
 	the base list
 RETURNS:		none	
 NOTES:			none
 */
extern void ListConcat(LIST *list1, LIST *list2);
 
/*
The following function are list removers, and are implemented in
list_removers.c
	ListRemove
	ListFree
	ListTrim
*/


/*
 FUNCTION:		ListRemove
 DESCRIPTION:		Returns a pointer to the current item in the list, and
 	removes it from the list. Makes the next item the current one.
 ARGUMENTS:		the list to manipulate.
 RETURNS:		a void pointer to the item removed: NULL if the item
 	could not be removed
 NOTES:			a list with NULL items may cause listRemove to appear
 	to fail, when it actually returns the item.
 */
extern void *ListRemove(LIST *wList);

/*
 FUNCTION:		ListFree
 DESCRIPTION:		Returns a pointer to the current item in the list, and
 	removes it from the list. Makes the next item the current one.
 ARGUMENTS:		the list to manipulate.
 RETURNS:		none
 NOTES:			none
 */
extern void ListFree(LIST *wList, void (*itemFree)(void *item));

/*
 FUNCTION:		ListRemove
 DESCRIPTION:		Returns a pointer to the current item in the list, and
 	removes it from the list. Makes the next item the current one.
 ARGUMENTS:		the list to manipulate.
 RETURNS:		a void pointer to the item trimed: NULL if the item
 	could not be removed.
 NOTES:			Same as list Remove
 */
extern void *ListTrim(LIST *wList);

#endif
