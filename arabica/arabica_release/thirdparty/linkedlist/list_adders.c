/*
 NAME:			Jon Lovering
 FILE:			list_adders.c
 DESCRIPTION:   	this file impliments the adders for the list
 NOTES:			none
 DATE:			30/12/05
 CONTACT:			jonlovering@ieee.org
*/

#include "list.h"

/*
The following functiona are list adders, and are implemented in list_adders.c
        ListCreate
        ListAdd
        ListInsert
        ListAppend
        ListPrepend
        ListConcat

	Also implemented:
	ListSetPointers
	ListGetNode
*/

/*
 FUNCTION:              ListCheck
 DESCRIPTION:           Check that the list is valid.
 ARGUMENTS:             the list to manipulate
 RETURNS:               an int, the result: 0 if list is invalid, 1 if list is
 	valid.
 NOTES:                 none
 */
extern int ListCheck(LIST *wList);

extern LIST *_GetListPointer(LIST *wList);

int getHandle(LIST *wList)
{
  	int i = 0;

	for(i = 0; i < list_repository.list_capacity; i++)
	{
		if(list_repository.handles[i] == NULL)	
		{
		  	break;
		}
	}

	if (i < list_repository.list_capacity)
	{
	  	list_repository.handles[i] = wList;
		return i;
	}
	return -1;
}

int _expandRepository()
{	
	NODE *define;
	NODE *new_first;
	int offset = 0;
	int old_capacity, new_capacity;
	int i;

	LIST *fixer;
	
#ifdef DEBUG
	fprintf(stderr, "in expandRepository\n");
#endif
	
	/*Preconditions: freelist is full 
		       freelist.first != Null
		       freelist.last != Null
	Postcondition: new freelist != Null*/
	old_capacity = repository.node_capacity;
	new_capacity = old_capacity * 2;
	
	new_first = (NODE *) realloc(repository.first, 
	    (new_capacity * sizeof(NODE)) );
	
	if (new_first == NULL)
	{
	  	return -1;
	}

	offset = (int)((char *)new_first - (char *)(repository.first));
	
	repository.first = new_first;
	define = repository.first;

	/*
		Fix the old repository, block may have moved, internal pointers are
		invalid.
	*/
	for( i = 0; i<(old_capacity); i++)
	{
	  	if(define[i].prev != NULL)
		{
		  	define[i].prev = (NODE *)((char *)define[i].prev + offset);
		}
	  	if(define[i].next != NULL)
		{
	  		define[i].next = (NODE *)((char *)define[i].next + offset);
		}
	  	define[i].master_next = (define + (i+1));
	}

	/*
		Formate the new nodes.
	*/
	for (i = old_capacity; i<new_capacity; i++)
	{
		define[i].prev = NULL;
		define[i].item = NULL;
		define[i].next = NULL;
		define[i].master_next = (define + (i+1));
		define[i].in_use = FALSE;
	}
	define[i-1].master_next = NULL;

	repository.node_capacity = new_capacity;
	
	/*
		Fix the list_repository, it's it's pointers are now invalid.
	*/
	fixer = list_repository.first;
	for (i = 0; i<list_repository.list_capacity; i++)
	{
	  	if(fixer[i].in_use == TRUE)
		{
		  	if(fixer[i].first != NULL)
			{
		  		fixer[i].first = (NODE *)((char *)fixer[i].first + offset);
			}
		  	if(fixer[i].last != NULL)
			{
	  			fixer[i].last = (NODE *)((char *)fixer[i].last + offset);
			}
		  	if(fixer[i].cur != NULL)
			{
	  			fixer[i].cur = (NODE *)((char *)fixer[i].cur + offset);
			}
		}
	}
	return 1;
}

int _expandList_Repository()
{
	LIST *define;
	LIST *new_first;
	int offset = 0;
	int old_capacity, new_capacity;
	int i;

#ifdef DEBUG
	fprintf(stderr, "in expandList_Repository\n");
#endif
	
	old_capacity = list_repository.list_capacity;
	new_capacity = old_capacity * 2;
	
	new_first = (LIST *) realloc(list_repository.first, 
	    (new_capacity *  sizeof(LIST)) );

	if (new_first == NULL)
	{
	  	return -1;
	}

	offset = (int)((char *)new_first - (char *)list_repository.first);
	
	list_repository.first = new_first;
	
	/*
		Fix the list handles
	*/
	for( i = 0; i < MAX_LISTS; i++)
	{
	  	if(list_repository.handles[i] != NULL)
		{
		  	list_repository.handles[i] = 
			  (LIST *)((char *)list_repository.handles[i] + offset); 
		}
	}
	/*
		Fix the old repository, block may have moved, internal pointers are
		invalid.
	*/
	define = list_repository.first;
	for( i = 0; i < old_capacity; i++)
	{
	  /*
	  	if(define[i].first != NULL)
		{
	  		define[i].first = (NODE *)((char *)define[i].first + offset);
		}
	  	if(define[i].last != NULL)
		{
	  		define[i].last = (NODE *)((char *)define[i].last + offset);
		}
	  	if(define[i].cur != NULL)
		{
			define[i].cur = (NODE *)((char *)define[i].cur + offset);
		}
	*/
	  	define[i].master_next = (define + (i+1));
	}

	/*
		Formate the new nodes.
	*/
	for (i = old_capacity; i < new_capacity; i++)
	{
		define[i].first = NULL;
		define[i].last = NULL;
		define[i].cur = NULL;
		define[i].listcount = 0;
		define[i].master_next = (define + (i+1));
		define[i].in_use = FALSE;
	}
	define[i-1].master_next = NULL;

	list_repository.list_capacity = new_capacity;
	return 1;
}

/*
 FUNCTION:              listCreate
 DESCRIPTION:           creates a new list
 ARGUMENTS:             none
 RETURNS:               a list structure, the base of the new list      
 NOTES:                 none
 */
LIST *ListCreate(void)
{
	LIST *newList;
	LIST *define_list;
	int i;
	
	#ifdef DEBUG
	fprintf(stderr,"List Create\n");
	#endif

	if (list_repository.list_capacity == 0)
	{
		/*
		Grab a chunk of memory, and point the start of the list_repository
		at it.
		define is the initializer, it sets all the pointers in the
		nodes, and the next pointers.
		*/
#ifdef DEBUG
	  	fprintf(stderr, "Populating the list repository\n");
#endif
		list_repository.listheader_num = 0;
		list_repository.list_capacity = MIN_LISTS;
		list_repository.first = (LIST *) malloc(MIN_LISTS * sizeof(LIST));
		
		if(list_repository.first == NULL)
		{
		  	return NULL;
		}
		
		list_repository.cur = list_repository.first;
		
		define_list = list_repository.first;
		for (i=0; i<MIN_LISTS; i++)
		{
			define_list[i].first = NULL;
			define_list[i].last = NULL;
			define_list[i].cur = NULL;
			define_list[i].listcount = 0;
			define_list[i].handle = -1;
			define_list[i].master_next = (define_list + (i+1));
			define_list[i].in_use = FALSE;
		}
		define_list[i-1].master_next = NULL;

		/*
		create an array of list handles.
		*/
		list_repository.handles = (LIST **) 
		  malloc (MAX_LISTS * sizeof(LIST *));
		if (list_repository.handles == NULL)
		{
		  	return NULL;
		}
		
		for (i=0; i < MAX_LISTS; i++)
		{
			list_repository.handles[i] = NULL;
		}
	}
	
	if (list_repository.listheader_num >= MAX_LISTS)
	{
		return NULL;
	}
	else if (list_repository.listheader_num == list_repository.list_capacity)
	{
	 	if (_expandList_Repository() == -1)
		{
		  	return NULL;
		}
	}
	
	for(list_repository.cur = list_repository.first ; 
	    list_repository.cur != NULL ; 
	    list_repository.cur =  list_repository.cur->master_next)
	{
		if (list_repository.cur->in_use == FALSE)
		{
			newList = list_repository.cur;
			newList->in_use = TRUE;
			newList->handle = getHandle(newList);
			if(newList->handle == -1)
			{
				return NULL;
			}
			newList = (LIST *)(newList->handle + 1);
			break;
		}	
	}
	
	list_repository.listheader_num++;

	return newList;
}

/*
 FUNCTION:              listSetPointer (internal)
 DESCRIPTION:           sets the pointers for the newnode, and the surrounding
	nodes.  Behaviour is goverend by the values of prev, and next, special
	case special values
 ARGUMENTS:            	a node pointer, the previous node; a node pointer, the
	next node; a node pointer, the new node; a void pointer, the item
	being added. 
 RETURNS:               none
 NOTES:                 none
 */
 void ListSetPointers(NODE *prev, NODE *next, NODE *newnode, void *item)
{
	newnode->prev = prev;
	newnode->next = next;
	newnode->item = item;
}


/*
 FUNCTION:              listGetNode
 DESCRIPTION:           gets a new node from the repository.
 ARGUMENTS:             none 
 RETURNS:               a node pointer, to the new node 
 NOTES:                 none
 */
 NODE *ListGetNode(void)
 {
 	NODE *newnode;
	NODE *define;
	int i=0;

#ifdef DEBUG
	fprintf(stderr, "In ListGetNode\n");
#endif	
	
	if (repository.node_capacity == 0)
	{
		/*
		Grab a chunk of memory, and point the start of the repository
		at it.
		define is the initializer, it sets all the pointers in the
		nodes, and the next pointers.
		*/
		
#ifdef DEBUG
	  	fprintf(stderr, "Populating the node repository\n");
#endif
		repository.listnode_num = 0;
		repository.node_capacity = MIN_NODES;
		repository.first = (NODE *) malloc(MIN_NODES * sizeof(NODE));
		repository.cur = repository.first;
		
		define = repository.first;
		for (i=0; i<MIN_NODES; i++)
		{
			define[i].prev = NULL;
			define[i].item = NULL;
			define[i].next = NULL;
			define[i].master_next = (define + (i+1));
			define[i].in_use = FALSE;
		}
		define[i-1].master_next = NULL;
	}
	
	if ( !(repository.listnode_num <= MAX_NODES))
	{
		return NULL;
	}
	else if (repository.listnode_num >= repository.node_capacity)
	{
		if (_expandRepository() != 1)
		{
		  	return NULL;
		}
	}

	for(repository.cur = repository.first;
	    repository.cur != NULL;
	    repository.cur = repository.cur->master_next)
	{
		if (repository.cur->in_use == FALSE)
		{
			repository.cur->in_use = TRUE;
			newnode = repository.cur;
			break;
		}	
	}
	
	repository.listnode_num++;

	return newnode;
	
}
 
/*
 FUNCTION:              listAdd
 DESCRIPTION:           inserts an item directly after the current item
 	and makes it the current item.
 ARGUMENTS:             the list to manipulate; an item, the item to add.
 RETURNS:               an int, the success of the function: -1 for failure,
 	1 for success
 NOTES:                 none
 */
int ListAdd(LIST *wList, void *item)
{
	NODE *newnode = NULL;
	wList = _GetListPointer(wList);
	
	#ifdef DEBUG
	fprintf(stderr,"In listAdd\n");
	#endif

	if (wList == NULL)
	{
		return -1;
	}

	if (item == NULL)
	{
	  	return -1;
	}

	newnode = ListGetNode();
	if (newnode == NULL)
	{
		return -1;
	}
	/*
	First node in the list
	*/
	if ( (wList->first == NULL) || (wList->last == NULL) || 
		(wList->cur == NULL) )
	{

		#ifdef DEBUG
		fprintf(stderr,"Adding first node to list\n");
		#endif
		
		ListSetPointers(newnode, newnode, newnode, item);
		
		wList->first = newnode;
		wList->last = newnode;
	}
	/*
	not the first node in the list
	*/
	else if ( (wList->first != NULL) && (wList->last != NULL) && 
		(wList->cur != NULL) )
	{
		/*
		being addeded as the last node of the list?
		*/
		if (wList->cur == wList->last)
		{

		#ifdef DEBUG
		fprintf(stderr,"Adding last item to the existing list\n");
		#endif
			/*
			set the node pointers
			*/
			
			ListSetPointers(wList->cur, newnode, newnode, item);
			
			/*
			make the previous item point at the new one	
			*/
			wList->cur->next = newnode;
			wList->last = newnode;
		}
		else
		{
			/*
			set the node pointers.
			insert the node
			*/
			#ifdef DEBUG
			fprintf(stderr,"Adding item to existing list\n");
			#endif

			ListSetPointers(wList->cur, wList->cur->next, newnode, 
				item);
			/*
			make the current item point at the new node
			make the old next item point at the new node
			*/
			wList->cur->next->prev = newnode;
			wList->cur->next = newnode;
		}
	}
	
	wList->cur = newnode;
	wList->listcount++;
	return 0;
}

/*
 FUNCTION:              ListInsert
 DESCRIPTION:           inserts an item directly before the current item        
 ARGUMENTS:             the list to manipulate; an item, the item to add.
 RETURNS:               an int, the success of the function: -1 for failure,
 	0 for success
 NOTES:                 listInsert functions the same as listAdd in an empty
	list situation. 
 */
int ListInsert(LIST *wList, void *item)
{

	NODE *newnode;
	
	wList = _GetListPointer(wList);
	#ifdef DEBUG
	fprintf(stderr,"In listInsert\n");
	#endif

	if (wList == NULL)
	{
		return -1;
	}
	
	if (item == NULL)
	{
	  	return -1;
	}
	
	newnode = ListGetNode();
	if (newnode == NULL)
	{
		return -1;
	}
	/*
	First node in the list
	*/
	if ( (wList->first == NULL) || (wList->last == NULL) || 
		(wList->cur == NULL) )
	{

		#ifdef DEBUG
		fprintf(stderr,"Adding first node to list\n");
		#endif
		
		ListSetPointers(newnode, newnode, newnode, item);
		
		wList->first = newnode;
		wList->last = newnode;
	}
	/*
	not the first node in the list
	*/
	else if ( (wList->first != NULL) && (wList->last != NULL) && 
		(wList->cur != NULL) )
	{
		/*
		being inserted as the first node of the list?
		*/
		if ( wList->cur == wList->first)
		{

		#ifdef DEBUG
		fprintf(stderr,"Adding first item to existing list\n");
		#endif
			/*
			set the node pointers
			*/
			
			ListSetPointers(newnode, wList->cur, newnode, item);

			/*
			make the current node point back at the new node
			make the list see the new node as the first node
			*/
			wList->cur->prev = newnode;
			wList->first = newnode;
		}
		else
		{
			/*
			set the node pointers.
			*/
			#ifdef DEBUG
			fprintf(stderr,"Adding item to existing list\n");
			#endif
			
			ListSetPointers(wList->cur->prev, wList->cur, newnode, 
				item);
			/*
			make the old previous node point at the new node
			make the current node point back at the new node
			*/
			wList->cur->prev->next = newnode;
			wList->cur->prev = newnode;
			
		}
	}
	
	/*
	make the new node the current node
	increment list count
	*/
	wList->cur = newnode;
	wList->listcount++;
	return 0;
}

/*
 FUNCTION:              listAppend
 DESCRIPTION:           inserts an item at the end of the list
 ARGUMENTS:             the list to manipulate; an item, the item to add.
 RETURNS:               an int, the success of the function: -1 for failure,
 	0 for success
 NOTES:                 listAppend handle first item insertion like other
	routines
 */
int ListAppend(LIST *wList, void *item)
{

        NODE *newnode;

	wList = _GetListPointer(wList);
        #ifdef DEBUG
        fprintf(stderr,"In listAppend\n");
        #endif

	if (wList == NULL)
	{
		return -1;
	}
	
	if (item == NULL)
	{
	  	return -1;
	}

        newnode = ListGetNode();
        if (newnode == NULL)
        {
                return -1;
        }
        /*
        First node in the list
        */
        if ( (wList->first == NULL) || (wList->last == NULL) || 
		(wList->cur == NULL) )
        {

                #ifdef DEBUG
                fprintf(stderr,"Adding first node to list\n");
                #endif

                ListSetPointers(newnode, newnode, newnode, item);

                wList->first = newnode;
                wList->last = newnode;
                wList->cur = newnode;
        }
        /*
        not the first node in the list
        */
        else if ( (wList->first != NULL) && (wList->last != NULL) && 
		(wList->cur != NULL) )
        {
                #ifdef DEBUG
                fprintf(stderr,"Appending item to the existing list\n");
                #endif
                /*
                set the node pointers
                */
                ListSetPointers(wList->last, newnode, newnode, item);
		
		/*
		make the last node point at the new node
		make the new node the last node
		*/
		wList->last->next = newnode;
		wList->last = newnode;
        }
	
	   wList->cur = newnode;
        wList->listcount++;
        return 0;
}

/*
 FUNCTION:              ListPrepend
 DESCRIPTION:           inserts an item at the begining of the list (before
        the first item)
 ARGUMENTS:             the list to manipulate; an item, the item to add.
 RETURNS:               an int, the success of the function: -1 for failure,
 	0 for success
 NOTES:                 listPrepend handles first item insertion like other
	adder functions.
 */
int ListPrepend(LIST *wList, void *item)
{
	NODE *newnode;

	wList = _GetListPointer(wList);
        #ifdef DEBUG
        fprintf(stderr,"In listPrepend\n");
        #endif

	if (wList == NULL)
	{
		return -1;
	}
	
	if (item == NULL)
	{
	  	return -1;
	}

        newnode = ListGetNode();
        if (newnode == NULL)
        {
                return -1;
        }
        /*
        First node in the list
        */
        if ( (wList->first == NULL) || (wList->last == NULL) || 
		(wList->cur == NULL) )
        {

                #ifdef DEBUG
                fprintf(stderr,"Adding first node to list\n");
                #endif

                ListSetPointers(newnode, newnode, newnode, item);

                wList->first = newnode;
                wList->last = newnode;
                wList->cur = newnode;
        }
        /*
        not the first node in the list
        */
        else if ( (wList->first != NULL) && (wList->last != NULL) &&
		(wList->cur != NULL) )
        {
                #ifdef DEBUG
                fprintf(stderr,"Prepending item to the existing list\n");
                #endif
                /*
                set the node pointers
                */
                ListSetPointers(newnode, wList->first, newnode, item);

		/*
		make the first node point back at the new node
		make the new node the first node
		*/
                wList->first->prev = newnode;
                wList->first = newnode;
        }

        wList->cur = newnode;
        wList->listcount++;
        return 0;
}

/*
 FUNCTION:              ListConcat
 DESCRIPTION:           concatinates two lists together.  The second list will
        no longer exist, and the current item will be the current item of the 
        first list
 ARGUMENTS:             a list, the base list; a list, the list to be added to
        the base list
 RETURNS:               none    
 NOTES:                 after use, list2 is no longer valid
 */
void ListConcat(LIST *list1, LIST *list2)
{
#ifdef DEBUG
  	fprintf(stderr,"in ListConcat\n");
#endif
	list1 = _GetListPointer(list1);
	list2 = _GetListPointer(list2);
	/*
	if either list is invalid, then special case.
	No resolution of special case
	if both lists are invalid, then we can't do this.
	*/
	if( (!ListCheck(list1) || !ListCheck(list2)) )
	{
	  	return;
	}
	else
	/*
	make the last node of list1 point at the first nod of list2
	make the last node of list1 the last node of list2.
	*/
		list1->last->next = list2->first;
		list2->first->prev = list1->last;
		list1->last = list2->last;
		list1->listcount += list2->listcount;
		/*
			Releasing the list
		*/
		list2->first = NULL;
		list2->last = NULL;
		list2->cur = NULL;
		list2->listcount = 0;
		list2->in_use = FALSE;
		list_repository.handles[list2->handle] = NULL;
		list2->handle = -1;
		list_repository.listheader_num--;
}
