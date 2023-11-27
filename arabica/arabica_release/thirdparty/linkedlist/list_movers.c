/*
 NAME:			Jon Lovering
 FILE:			list_movers.c
 DESCRIPTION:       this is the implementation of the movers for the list 
	library
 NOTES:			none
 DATE:			30/12/05
 CONTACT:			jonlovering@ieee.org
*/

#include "list.h"

/*
The following function are list movers:
        ListCount
        ListFirst
        ListLast
        ListNext
        ListPrev
        ListCurr
        ListSearch

	Also implements
	ListCheck
*/

/*
 FUNCTION:              ListCount       
 DESCRIPTION:           return the number of items in the list
 ARGUMENTS:             the list to be counted
 RETURNS:               none
 NOTES:                 a list var is maintained call listcount. 
 */
LIST *_GetListPointer(LIST *wList)
{
	if(list_repository.handles != NULL)
	{ 
	/*
		correct by one since the the user has a handle which is +1 so that
		it does not appear as NULL
	*/
		return (list_repository.handles[(int)wList-1]);
	}
	return NULL;
}

/*
 FUNCTION:              ListCount       
 DESCRIPTION:           return the number of items in the list
 ARGUMENTS:             the list to be counted
 RETURNS:               none
 NOTES:                 a list var is maintained call listcount. 
 */
int ListCount(LIST *wList)
{
  	wList = _GetListPointer(wList);
	#ifdef DEBUG
	fprintf(stderr,"In listCount\n");
	#endif

	if (wList == NULL)
	{
		return -1;
	}

	return (wList->listcount);
}

/*
 FUNCTION:              ListCheck
 DESCRIPTION:           Check that the list is valid.
 ARGUMENTS:             the list to manipulate
 RETURNS:               an int, the result: 0 if list is invalid, 1 if list is
 	valid.
 NOTES:                 none
 */
int ListCheck(LIST *wList)
{
	if (wList == NULL)
	{
		return 0;
	}

	if ( (wList->first == NULL) || (wList->last == NULL) || 
		(wList->cur == NULL) )
	{
		return 0;
	}
	
	else if ( (wList->first != NULL) && (wList->last != NULL) && 
		(wList->cur != NULL) )
	{
		return 1;
	}
	return 0;
}

/*
 FUNCTION:              ListFirst
 DESCRIPTION:           return the first item in the list, and make the first
        node the current one
 ARGUMENTS:             the list to manipulate
 RETURNS:               an void pointer, to the first item: NULL if the last
 	item does not exist.
 NOTES:                 none
 */
void *ListFirst(LIST *wList)
{
	wList = _GetListPointer(wList);

	#ifdef DEBUG
	fprintf(stderr,"In listFirst\n");
	#endif

	if (!ListCheck(wList))
	{
		return NULL;
	}
	else
	{
		wList->cur = wList->first;
		return (wList->cur->item);
	}
	return NULL;
}

/*
 FUNCTION:              ListLast
 DESCRIPTION:           return the last item in the list, and make it the
        current item.
 ARGUMENTS:             the list to manipulate
 RETURNS:               an void pointer, to the last item: NULL if the last
 	item does not exist.
 NOTES:                 none
 */
void *ListLast(LIST *wList)
{
	wList = _GetListPointer(wList);
	
	#ifdef DEBUG
	fprintf(stderr,"In listLast\n");
	#endif

	if (!ListCheck(wList))
	{
		return NULL;
	}
	else
	{
		wList->cur = wList->last;
		return (wList->cur->item);
	}
	return NULL;
}

/*
 FUNCTION:              ListNext
 DESCRIPTION:           make the next item the current item, and return it.
 ARGUMENTS:             the list to manipulate
 RETURNS:               an void pointer, to the next item.
                        NULL if the next item is off the list
 NOTES:                 none
 */
void *ListNext(LIST *wList)
{
	wList = _GetListPointer(wList);
	
	#ifdef DEBUG
	fprintf(stderr,"In listNext\n");
	#endif
	
	if (!ListCheck(wList))
	{
		return NULL;
	}
	else
	{
		/*
		ensure we are looking at a valid item,
		it it the end of the list?
		*/
		if (wList->cur->next == NULL)
		{
			return NULL;
		}
		else if (wList->cur->next == wList->cur)
		{
			return NULL;
		}
		else if (wList->cur != NULL && wList->cur->next != NULL)
		{
			wList->cur = wList->cur->next;
			return (wList->cur->item);
		}
	}
	return NULL;
}

/*
 FUNCTION:              ListPrev
 DESCRIPTION:           make the previous item the current item, and return it
 ARGUMENTS:             the list to manipulate
 RETURNS:               a void pointer to the previous item in the list.
                        NULL if the previous item is off the list
 NOTES:                 none
 */
void *ListPrev(LIST *wList)
{
	wList = _GetListPointer(wList);
	
	#ifdef DEBUG
	fprintf(stderr,"In listPrev\n");
	#endif

	if (!ListCheck(wList))
	{
		return NULL;
	}
	else
	{
		/*
		Ensure we are looking at a valid item
		Are we at the start of the list
		*/
		if (wList->cur->prev ==NULL)
		{
			return NULL;
		}
		else if (wList->cur->prev == wList->cur)
		{
			return NULL;
		}
		else if (wList->cur != NULL && wList->cur->prev != NULL)
		{
			wList->cur = wList->cur->prev;
			return (wList->cur->item);
		}
	}
	return NULL;
}

/*
 FUNCTION:              ListCurr
 DESCRIPTION:           return the current item in the list
 ARGUMENTS:             the list to manipulate
 RETURNS:               a void pointer to the current item in the list: NULL
 	if the item does not exist
 NOTES:                 none
 */
void *ListCurr(LIST *wList)
{
	wList = _GetListPointer(wList);
	
	#ifdef DEBUG
	fprintf(stderr,"In listCurr\n");
	#endif

	if (!ListCheck(wList))
	{
		return NULL;
	}
	else
	{
		return (wList->cur->item);
	}
	return NULL;
}

/*
 FUNCTION:              ListSearch
 DESCRIPTION:           locates an item in the list.  The item is determined
        by a compaarator function passed to the function, and a comparisonArg
        passed to the function.
 ARGUMENTS:             a list, the list to manipulate; a function pointer,
        the comparison opperation; a comparisonArg.
 RETURNS:               a void pointer, to the item searched for.
                        NULL if the item is not found.
 NOTES:                 none
 */
void *ListSearch(LIST *wList, int (*comparator)(void *item1, void *item2), void *compairTo)
{
	wList = _GetListPointer(wList);
	
	#ifdef DEBUG
	fprintf(stderr,"In listSearch\n");
	#endif
	
	if (!ListCheck(wList))
	{
		return NULL;
	}
	else
	{
		/*
		Start at the begining of the list
		Compair each item, it it is the item, break the loop
		Check the last item (not covered by the loop)
		Return the item if it was found.
		*/
		wList->cur = wList->first;
	
		while (wList->cur != wList->last)
		{
			if ((*comparator)(wList->cur->item, compairTo))
			{
				break;
			}
			wList->cur = wList->cur->next;
		}

		if (wList->cur == wList->last)
		{
			if ((*comparator)(wList->cur->item, compairTo))
			{
				return (wList->cur->item);
			}
			else
			{
				return NULL;
			}
		}

		else if (wList->cur != wList->last)
		{
			return (wList->cur->item);
		}
	}
	return NULL;
}
