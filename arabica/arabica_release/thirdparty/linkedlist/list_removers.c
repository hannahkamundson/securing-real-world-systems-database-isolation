/*
 NAME:			Jon Lovering
 FILE:              list_removers.c
 DESCRIPTION:       this is the implementation of the removers for the list
        library
 NOTES:             none
 DATE:			30/12/05
 CONTACT:			jonlovering@ieee.org
*/

#include "list.h"

/*
The following function are list removers, and are implemented in
list_removers.c
        ListRemove
        ListFree
        ListTrim
*/

/*
 FUNCTION:              ListCheck
 DESCRIPTION:           Check that the list is valid.
 ARGUMENTS:             the list to manipulate
 RETURNS:               an int, the result: 0 if list is invalid, 1 if lst is
 	valid.
 NOTES:                 none
 */
extern int ListCheck(LIST *wList);

extern LIST *_GetListPointer(LIST *wList);

#ifdef DEBUG1
void __DEBUGRepositoryPrint(void)
{
  	int i;
	NODE *chomp;

  	fprintf(stderr, "Repository contains %i nodes, %i in use\n", 
	    repository.node_capacity, repository.listnode_num);
	for (i=0; i<repository.node_capacity; i++)
	{
	  	fprintf(stderr,"NODE %lx\tmaster_next %lx\tin_use %lx\n", 
		    (unsigned long int)(repository.first + i), 
		    (unsigned long int)(repository.first + i)->master_next, 
		    (unsigned long int)(repository.first + i)->in_use);
	}
	for (i=0; i<list_repository.listheader_num ; i++)
	{
	  	fprintf(stderr,"Looking at list %i\n", i);
		fprintf(stderr,"LIST %lx\tfirst %lx\tlast %lx\tcur %lx\tlistcount %i\n",
		    (unsigned long int)(list_repository.first + i),
		    (unsigned long int)(list_repository.first + i)->first,
		    (unsigned long int)(list_repository.first + i)->last,
		    (unsigned long int)(list_repository.first + i)->cur,
		    (list_repository.first + i)->listcount);
		
		chomp = (list_repository.first + i)->first; 
		while (chomp != NULL)
		{
		  	fprintf(stderr,"\tNODE %lx\tprev %lx\tnext %lx\n", 
			    (unsigned long int)chomp, 
			    (unsigned long int)chomp->prev, 
			    (unsigned long int)chomp->next);
			if (chomp == chomp->next)
			{
				break;
			}
			chomp = chomp->next;
		}
	}
}
#endif

void _switchNodes(NODE *free, NODE *used)
{
  	LIST *fixer;
	int i;

#ifdef DEBUG
		fprintf(stderr, "in switchNodes\nswitching %lx to %lx\n", 
		    (unsigned long int) free, 
		    (unsigned long int) used);
#endif
		
	if ((free == NULL) || (used == NULL))
	{
	 	return;
	}
	
	if (used->next == used && used->prev == used)		
	{
		/*
		Moving the only node in a list
		*/
#ifdef DEBUG
		fprintf(stderr, "%lx is only node in it's list\n", 
		    (unsigned long int)used);
#endif
		free->prev = free;
		free->next = free;
	}	
	else if(used->next == used)		
	{
		/*
		Moving the last node in a list
		*/
#ifdef DEBUG
		fprintf(stderr, "%lx is last node in it's list\n", 
		    (unsigned long int)used);
#endif
		used->prev->next = free;
				
		free->prev = used->prev;
		free->next = free;
	}
	else if (used->prev == used)
	{
	  	/*	
	  	Moving the first node in a list
		*/
#ifdef DEBUG
		fprintf(stderr, "%lx is first node in it's list\n", 
		    (unsigned long int)used);
#endif
	  	used->next->prev = free;
			
		free->next = used->next;
		free->prev = free;
	}
	else
	{
		/*
	  	General node
		*/
#ifdef DEBUG
		fprintf(stderr, "%lx is a general node in it's list\n", 
		    (unsigned long int)used);
#endif
		used->prev->next = free;
		used->next->prev = free;
	  		
		free->prev = used->prev;
		free->next = used->next;
	}
		
	/*
	Always move the item, and the in_use flag
	*/
		
	free->item = used->item;
	free->in_use = used->in_use;
			
	used->in_use = FALSE;
	used->prev = NULL;
	used->next = NULL;
	used->item = NULL;

	/*
	If we have swithched a node, then we need to make sure
	that any list pointing to it knows
	Since the node does not know what list it belongs to, then we 
	must iterate.
	*/
	fixer = list_repository.first;
	for(i = 0; i < list_repository.list_capacity; i++)
	{
  		if(fixer[i].first == used)
		{
     		fixer[i].first = free;           
		}
  		if(fixer[i].last == used)
		{
			fixer[i].last = free;             
		}
  		if(fixer[i].cur == used)
		{
			fixer[i].cur = free;
		}
	}
	return;
}

int _deminishRepository()
{
 	/*preconditions: current_Node_Number > MIN_NODE
		       sizeof(current_Node_Number*NODE) < Allocated memory	
		       freelist.first!=Null
		       freelist.last!=Null
	Postcondition: new freelist != Null	*/
 
  	NODE *define, *free, *used;
	NODE *new_first;
	LIST *fixer;
	int offset = 0;
	int old_capacity, new_capacity;
	int i;

#ifdef DEBUG
	fprintf(stderr, "in deminishRepository\n");
#endif

	old_capacity = repository.node_capacity;
	new_capacity = old_capacity / 2;

	/*
		Must condense the list before reallocing.
	*/
	free = repository.first;
	used = (repository.first + new_capacity);


#ifdef DEBUG1
	__DEBUGRepositoryPrint();
#endif
	
	while( free < (repository.first + new_capacity) && used != NULL)
	{
#ifdef DEBUG
	  fprintf(stderr, "finding free/used pair\n");
#endif
	  	while ((free != NULL) && (free->in_use == TRUE))
		{
		  	free = free->master_next;
		}
		while ((used != NULL) && (used->in_use == FALSE))
		{
		  	used = used->master_next;
		}
		/*
			Now we are looking at a Free node inside the new boundary, 
			and a used node outside of the new boundary.
		*/
		_switchNodes(free, used);
	}
	/*
		Now we have moved all the data to within the new_capacity, realloc
	*/
	new_first = realloc(repository.first, (new_capacity * sizeof(NODE)));

	if (new_first == NULL)
	{
	  	return -1;
	}
	
	offset = (int)((char *)new_first - (char*)repository.first);
	repository.first = new_first;
	
	define = repository.first;
	for (i=0; i < new_capacity; i++)
	{
	  	if (define[i].prev != NULL)
		{
		  	define[i].prev = (NODE *)((char *)define[i].prev + offset);
		}
	  	if (define[i].next != NULL)
		{
			define[i].next = (NODE *)((char *)define[i].next + offset);
		}
		define[i].master_next = (define + (i+1));
	}

	define[i-1].master_next = NULL;

	repository.node_capacity = new_capacity;

	/*
		Fix the list_repository, it's pointers are invalid.
	*/
	fixer = list_repository.first;
	for(i = 0; i<list_repository.list_capacity; i++)
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

#ifdef DEBUG1
	__DEBUGRepositoryPrint();
#endif

	return 1;
}

int _deminishList_Repository()
{
  	LIST *define, *used, *free;
	LIST *new_first;
	int offset = 0;
	int old_capacity, new_capacity;
	int i;

#ifdef DEBUG
	fprintf(stderr, "in deminishList_Repository\n");
#endif

	old_capacity = list_repository.list_capacity;
	new_capacity = old_capacity / 2;

	free = list_repository.first;
	used = (list_repository.first + new_capacity);

#ifdef DEBUG1
	for (i = 0; i < MAX_LISTS; i++)
	{
		fprintf(stderr, "HANDLE %i, LIST %lx\n", i,
	    		(unsigned long int) list_repository.handles[i]);
	}
#endif
	
	while( free < (list_repository.first + new_capacity) && used != NULL)
	{
#ifdef DEBUG
	  fprintf(stderr, "finding free/used pair\n");
#endif
	  	while ((free != NULL) && (free->in_use == TRUE))
		{
		  	free = free->master_next;
		}
		while ((used != NULL) && (used->in_use == FALSE))
		{
		  	used = used->master_next;
		}
		/*
			move the pair
		*/

		if (free != NULL && used != NULL)
		{
#ifdef DEBUG
		  	fprintf(stderr,"switching %lx to %lx",
			    (unsigned long int) used,
			    (unsigned long int) free);
#endif
			free->first = used->first;
			free->last = used->last;
			free->cur = used->cur;
			free->listcount = used->listcount;
			free->handle = used->handle;
			free->in_use = TRUE;

			list_repository.handles[used->handle] = free;

			used->first = NULL;
			used->last = NULL;
			used->cur = NULL;
			used->listcount = 0;
			used->handle = -1;
			used->in_use = FALSE;
		}
	}

	new_first = (LIST *) realloc(list_repository.first, new_capacity * 
	    sizeof(LIST));

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
#ifdef DEBUG
	for (i = 0; i < MAX_LISTS; i++)
	{
		fprintf(stderr, "HANDLE %i, LIST %lx\n", i,
	    		(unsigned long int) list_repository.handles[i]);
		fflush(stderr);
	}
#endif
	/*
		fix the repository
	*/
	define = list_repository.first;
	for(i = 0; i< new_capacity ; i++)
	{
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
		define[i].master_next = (define + (i+1));
	}

	define[i-1].master_next = NULL;

	list_repository.list_capacity = new_capacity;
	
	return 1;
}

/*
 FUNCTION:              ListReturn
 DESCRIPTION:          	Returns a node to the free list 
 ARGUMENTS:             the node to return
 RETURNS:               none 
 NOTES:                 none
 */
void ListReturn(NODE *freed)
{
	/*
	NODE *define;
	int i=0;
	*/
	/*
	Reset the pointer to NULL
	*/
#ifdef DEBUG
	fprintf(stderr,"In listReturn, returning %lx\n", (unsigned long int)freed);
#endif
	
	freed->prev = NULL;
	freed->item = NULL;
	freed->next = NULL;
	freed->in_use = FALSE;
	
	repository.listnode_num--;
	
	if (repository.listnode_num == 0)
	{
#ifdef DEBUG
	  	fprintf(stderr, "Repository is empty, liberating\n");
#endif
	  	free(repository.first);
		repository.first = NULL;
		repository.cur = NULL;
		repository.listnode_num = 0;
		repository.node_capacity = 0;
	}
	else if ( (repository.node_capacity / repository.listnode_num) >= 2 )
	{
	  	if (!(repository.node_capacity == MIN_NODES))
		{
	  		if (_deminishRepository() == -1)
			{
				/*
				some error
				*/
			}
		}
	}
	
	return;
}

/*
 FUNCTION:              ListRemove
 DESCRIPTION:           Returns a pointer to the current item in the list, and
        removes it from the list. Makes the next item the current one.
 ARGUMENTS:             the list to manipulate.
 RETURNS:               a void pointer that points to the item that was in the
 	node: NULL if the item could not be removed.
 NOTES:                 none
 */
void *ListRemove(LIST *wList)
{
	NODE *removed;

	void *item;
	
	wList = _GetListPointer(wList);

	#ifdef DEBUG
	fprintf(stderr, "In listRemove\n");
	#endif

	if (!ListCheck(wList))
	{
		return NULL;
	}
	else
	{
		if ((wList->cur->next == wList->cur) &&
		    (wList->cur->prev == wList->cur)) 
		{
			#ifdef DEBUG
			fprintf(stderr, "freeing the only item\n");
			#endif
			removed = wList->cur;

			wList->first = NULL;
			wList->last = NULL;
			wList->cur = NULL;
		}
		/*
		if the current item is the last item, remove it, set the
		pointers correctly, and set cur to point to the previous item
		*/
		else if (wList->cur->next == wList->cur) 
		{
			#ifdef DEBUG
			fprintf(stderr, "freeing the last item\n");
			#endif
			removed = wList->cur;

			/*
			The new current item is the one before the removed one;
			*/
			wList->cur = wList->cur->prev;
			/*
			the item must point at it's self.
			*/
			wList->cur->next = wList->cur;
			/*
			The new last item is the current item
			*/
			wList->last = wList->cur;
		}
		/*
		if the current item is the first item, remove it, set the 
		pointers correctly, and set cur to be the new first list item.
		*/
		else if (wList->cur->prev == wList->cur) 
		{
			#ifdef DEBUG
			fprintf(stderr, "freeing the first item\n");
			#endif
			removed = wList->cur;

			/*
			the new current item is the next item
			*/
			wList->cur = wList->cur->next;
			/*
			The next item after points back to cur
			*/
			wList->cur->prev = wList->cur;
			/*
			the new first item is the current item
			*/
			wList->first = wList->cur;;
		} 
		else 
		{
			#ifdef DEBUG
			fprintf(stderr, "freeing the item\n");
			#endif
			removed = wList->cur;

			wList->cur = wList->cur->next;
			/*
			the previous item must point to cur's next
			*/
			removed->prev->next = removed->next;
			/*
			the next item must point to cur's previous
			*/
			removed->next->prev = removed->prev;
		}

		item = removed->item;

		wList->listcount--;
		
		ListReturn(removed);

		return item;
	}
	return NULL;
}

/*
 FUNCTION:              ListFree
 DESCRIPTION:           free the entire list.
 ARGUMENTS:             the list to manipulate. a function pointer that can
 	free the items in the list
 RETURNS:               none
 NOTES:                 none
 */
void ListFree(LIST *wList, void (*itemFree)(void *item))
{
	void *item;
	
	#ifdef DEBUG
	fprintf(stderr,"In listFree\n");
	#endif
	wList = _GetListPointer(wList);

	if (!ListCheck(wList))
	{
		/*
		ListCheck may identify an invalid list, that is allocated, if
		this is the case, free the list header
		*/
		if ( wList != NULL)
		{
			/*
			Releasing the list
			*/
			list_repository.handles[wList->handle] = NULL;
			wList->first = NULL;
			wList->last = NULL;
			wList->cur = NULL;
			wList->listcount = 0;
			wList->handle = -1;
			wList->in_use = FALSE;
		}
		else if (wList == NULL)
		{
			return;
		}
	}
	else
	{
		/*
		Start at the begining,
		Save the next location,
		free the item.
		return the node.
		continue.

		Get the last item, not covered by the list
		*/
		wList->cur = wList->first;
		
		/*
			Bad API, but this is the only way to make the dynamic sizing
			work.  It seems pointless to alias ListRemove as a private
			function, to then write public list remove as a wrapper.
			This way the list gets free'd and all is happy.

			Nasty hack to make the arguement to list remove acceptable to
			ListRemove we have to hack this since the user list handle is
			+1 over the internal.
		*/
		while ( (item = ListRemove( (LIST *)(wList->handle+1) )) ) 
		{
			(*itemFree)(item);
			wList->listcount--;
		}
		/*
		Releasing the list
		*/
		list_repository.handles[wList->handle] = NULL;
		wList->first = NULL;
		wList->last = NULL;
		wList->cur = NULL;
		wList->listcount = 0;
		wList->handle = -1;
		wList->in_use = FALSE;
	}
	
	/*
	is this the last list in use,
	free the repository
	*/
	list_repository.listheader_num--;
	if (list_repository.listheader_num == 0)
	{
#ifdef DEBUG
	  	fprintf(stderr, "List Repository is empty, liberating\n");
#endif
		free(list_repository.first);
		free(list_repository.handles);
		list_repository.first = NULL;
		list_repository.cur = NULL;
		list_repository.listheader_num = 0;
		list_repository.list_capacity = 0;
		list_repository.handles = NULL;
	}
	/*
	   else the repository needs to shrink
	*/
	else if (list_repository.list_capacity / list_repository.listheader_num 
	    >= 2)
	{
		if(!(list_repository.list_capacity == MIN_LISTS))
		{
			if (_deminishList_Repository() == -1)
			{
			  	/*
					some error
				*/
			}
		}
	}

	return;
}

/*
 FUNCTION:              ListTrim
 DESCRIPTION:           Remove the last item from the list, return it's item
 	and make the new last item the current one.
 ARGUMENTS:             the list to manipulate.
 RETURNS:               a void pointer, to the item just removed: NULL if the
 	item could not be removed
 NOTES:                 none
 */
void *ListTrim(LIST *wList)
{
	NODE *removed;
	void *item;

	wList = _GetListPointer(wList);
	if (!ListCheck(wList) || wList->listcount < 1)
	{
		return NULL;
	}
	else
	{
		/*
		Save the removed node
		*/
		removed = wList->last;

		/*
		Remove the node
		*/
		wList->last->prev->next = wList->last->prev;
		wList->last = wList->last->prev;

		item = removed->item;
		/*
		Return the node to the freelist
		and return the item
		*/
		ListReturn(removed);
		wList->listcount--;
		
		return item;
	}

	return NULL;
}
