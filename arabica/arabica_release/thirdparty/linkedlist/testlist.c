/*
 NAME:			Jon Lovering
 FILE:			testlist.c 
 DESCRIPTION:		this is the test program for the list library
 NOTES:			none
 DATE:			30/12/05
 CONTACT:			jonlovering@ieee.org
 */

#include "list.h"
#include <stdio.h>

int comparator(void *item1, void *item2);
void itemFree(void *item);
void itemFree1(void *item);

int PrintForward(LIST *myList);
int PrintReverse(LIST *myList);

int stageOne(void);
int stageTwo(void);
int stageThree(void);
int stageFour(void);

int main(int argc, char **argv)
{
	if (!stageOne())
	{
		return 1;
	}
	if (!stageTwo())
	{
		return 1;
	}
	if (!stageThree())
	{
		return 1;
	}

	printf("Tests Completed\n");

	printf("Stage 4 tests maximum node creation:\n\
	WARNING!!\n\
do not run on a dynamically allocated list, \
as it will run to stack overflow!\n");
	printf("Proceed?? ");
	char go = getchar();
	if ( (go == 'y') || (go == 'Y') )
	{
		if(!stageFour())
		{
			return 1;
		}
		printf("Stage 4 complete\n");
	}
	
	return 0;
}

int stageOne(void)
{
	LIST *myList, *myList2;	
	int i=0, *tester, *testy;

	printf("\n\n ===== STAGE 1 ===== \n\n");
	
	myList = ListCreate();
	myList2 = ListCreate();

	if (myList == NULL)
	{
		return 0;
	}
	printf("List created\n\n");
	
	/*
	Add 5 items
	*/
	for (i=1; i<=5; i++)
	{
		tester = (int *) malloc(1 * sizeof(int));
		*tester = i;
		if (ListAdd(myList, tester) == -1)
		{
			return 0;
		}
		if ( (testy = (int *) ListCurr(myList)) == NULL )
		{
			return 0;
		}
		printf("item %i: %i\t\
		list count: %i\n", i, *testy, ListCount(myList));
	}

	printf("Insertion with Add completed\n\n");
	
	/*
	Append 5 items
	*/
	for(i=6; i<=10; i++)
	{
		tester = (int *) malloc(1 * sizeof(int));
		*tester = i;
		if (ListAppend(myList, tester) == -1)
		{
			return 0;
		}
		if ( (testy = (int *) ListLast(myList)) == NULL )
		{
			return 0;
		}
		printf("item %i: %i\t\
		list count: %i\n", i, *testy, ListCount(myList));
	}
	
	printf("List Appendation with Append completed\n\n");
	
	/*
	Prepend 5 items
	*/
	for(i=11; i<=15; i++)
	{
		tester = (int *) malloc(1 * sizeof(int));
		*tester = i;
		if (ListPrepend(myList, tester) == -1)
		{
			return 0;
		}
		if ( (testy = (int *) ListFirst(myList)) == NULL )
		{
			return 0;
		}
		printf("item %i: %i\t\
		list count: %i\n", i, *testy, ListCount(myList));
	}

	printf("List Prependation with Prepend completed\n\n");

	/*
	Remove all items
	*/
	for(i=15; i>=1; i--)
	{
		if ( (testy = (int *) ListRemove(myList)) == NULL )
		{
			return 0;
		}
		printf("item %i: %i\t\
		list count: %i\n", i, *testy, ListCount(myList));
		free(testy);
	}
	
	printf("Removal with Remove completed\n\n");
	
	/*
	add an item and remove it, check that the list handles emptying
	properly
	*/
	if (ListAdd(myList, &i) == -1)
	{
		return 0;
	}
	if (ListRemove(myList) == NULL)
	{
		return 0;
	}
	printf("Reinsertion, and removeal completed\n\n");

	/*
	Insert 5 items.
	Repeat above procedure on list 2
	*/
	for( i=1; i<=5; i++)
	{
		tester = (int *) malloc(1 * sizeof(int));
		*tester = i;
		if (ListInsert(myList2, tester) == -1)
		{
			return 0;
		}
		if ( (testy = (int *) ListCurr(myList2)) == NULL )
		{
			return 0;
		}
		printf("item %i: %i\t\
		list count: %i\n", i, *testy, ListCount(myList2));
	}
	
	printf("Insertion with Insert completed\n\n");
	
	for(i=6; i<=10; i++)
	{
		tester = (int *) malloc(1 * sizeof(int));
		*tester = i;
		if (ListAppend(myList2, tester) == -1)
		{
			return 0;
		}
		if ( (testy = (int *) ListLast(myList2)) == NULL )
		{
			return 0;
		}
		printf("item %i: %i\t\
		list count: %i\n", i, *testy, ListCount(myList2));
	}
	
	printf("List Appendation with Append completed\n\n");

	for(i=11; i<=15; i++)
	{
		tester = (int *) malloc(1 * sizeof(int));
		*tester = i;
		if (ListPrepend(myList2, tester) == -1)
		{
			return 0;
		}
		if ( (testy = (int *) ListFirst(myList2)) == NULL )
		{
			return 0;
		}
		printf("item %i: %i\t\
		list count: %i\n", i, *testy, ListCount(myList2));
	}

	printf("List Prependation with Prepend completed\n\n");

	for(i=15; i>=1; i--)
	{
		if ( (testy = (int *) ListRemove(myList2)) == NULL )
		{
			return 0;
		}
		printf("item %i: %i\t\
		list count: %i\n", i, *testy, ListCount(myList2));
		free(testy);
	}

	printf("Removal with Remove completed\n\n");

	if (ListInsert(myList2, &i) == -1)
	{
		return 0;
	}
	if (ListRemove(myList2) == NULL)
	{
		return 0;
	}
	printf("Reinsertion, and removeal completed\n\n");	

	/*
	test empty list insetion with append and prepend
	*/
	if (ListAppend(myList2, &i) == -1)
	{
		return 0;
	}
	if (ListRemove(myList2) == NULL)
	{
		return 0;
	}
	printf("Reinsertion, and removeal completed\n\n");	
	
	if (ListPrepend(myList2, &i) == -1)
	{
		return 0;
	}
	if (ListRemove(myList2) == NULL)
	{
		return 0;
	}
	printf("Reinsertion, and removeal completed\n\n");	
	ListFree(myList, &itemFree);
	ListFree(myList2, &itemFree);
}

int stageTwo(void)
{
	LIST *myList, *myList2;
	int i=0, *tester, *testy;

	printf("\n\n ===== STAGE 2 ===== \n\n");

	myList = ListCreate();
	myList2 = ListCreate();
	
	/*
	Add 5 items
	*/
	for(i=0; i<5; i++)
	{
		tester = (int *) malloc(1 * sizeof(int));
		*tester = i;
		if (ListAdd(myList, tester) == -1)
		{
			return 0;
		}
		ListCurr(myList);
		if (ListAdd(myList2, tester) == -1)
		{
			return 0;
		}
	}
	/*
	Insert 5 items
	*/
	for(i=5; i<10; i++)
	{
		tester = (int *) malloc(1 * sizeof(int));
		*tester = i;
		if (ListInsert(myList, tester) == -1)
		{
			return 0;
		}
		if (ListInsert(myList2, tester) == -1)
		{
			return 0;
		}
	}
	/*
	Append 5 items
	*/
	for(i=10; i<15; i++)
	{
		tester = (int *) malloc(1 * sizeof(int));
		*tester = i;
		if (ListPrepend(myList, tester) == -1)
		{
			return 0;
		}
		if (ListPrepend(myList2, tester) == -1)
		{
			return 0;
		}
	}
	/*
	Prepend 5 items
	*/
	for(i=15; i<20; i++)
	{
		tester = (int *) malloc(1 * sizeof(int));
		*tester = i;
		if (ListAppend(myList, tester) == -1)
		{
			return 0;
		}
		if (ListAppend(myList2, tester) == -1)
		{
			return 0;
		}
	}
	
	printf("List1 has length: %i\n", ListCount(myList));
	
	printf("List2 has length: %i \n", ListCount(myList2));
	
	printf("\n\nWhat's in the lists?\n\n");
	
	printf("The contents of list1 is: \n");
	
	if (!PrintForward(myList))
	{
		return 0;
	}

	printf("\nThe contents of list2 is: \n");
	
	if (!PrintForward(myList2))
	{
		return 0;
	}
	
	printf("\n\n Now in reverse: \n\n");

	printf("The contents of list1 is: \n");
	
	if (!PrintReverse(myList))
	{
		return 0;
	}

	printf("\nThe contents of list2 is: \n");
	
	if (!PrintReverse(myList2))
	{
		return 0;
	}

	printf("\n\n Concatinating \n\n");
	
	ListConcat(myList, myList2);
	
	printf("List1 has length: %i\n", ListCount(myList));
	
	printf("The contents the list (list1) is: \n");
	
	if (!PrintForward(myList))
	{
		return 0;
	}
	
	printf("\n\nIn Reverse\n");	
	
	printf("The contents of list1 is: \n");
	
	if (!PrintReverse(myList))
	{
		return 0;
	}

	printf("\n\n Trim the list\n");
	
	for(i=40; i>35; i--)
	{
		if ( (testy = ListTrim(myList)) == NULL )
		{
			return 0;
		}
	}

	printf("The contents of the list is now:\n");
	
	if (!PrintForward(myList))
	{
		return 0;
	}
	
	printf("\n\nIn Reverse\n");	
	
	printf("The contents of list1 is: \n");
	
	if (!PrintReverse(myList))
	{
		return 0;
	}
	
	printf("\n\nNow lets search the list\n\n");
	
	tester = (int *) malloc(1 * sizeof(int));
	
	*tester = 15;
	
	testy = ListSearch(myList, &comparator, tester);

	if (testy == NULL)
	{
		return 0;
	}

	printf("Item %i found at %x\n", *testy, testy);

	*tester = 14;
	
	testy = ListSearch(myList, &comparator, tester);

	if (testy == NULL)
	{
		return 0;
	}

	printf("Item %i found at %x (listFirst %x)\n", *testy, testy,
	ListFirst(myList));
	
	*tester = 65;
	
	testy = ListSearch(myList, &comparator, tester);

	if (testy == NULL)
	{
		printf("Item 67 not found, GOOD!!\n");
	}
	else if (testy != NULL)
	{
		return 0;
	}
	
	free(tester);

	printf("\n\nDeleting some elements in the front of the list\n");

	testy = (int *) ListFirst(myList);

	if (testy == NULL)
	{
		return 0;
	}

	for (i=0; i<3;i++)
	{
		testy = ListRemove(myList);
		if (testy == NULL)
			{
				return 0;
			}
	}
	
	printf("\n\nDeleting some elements in the middle of the list\n");

	testy = (int *) ListFirst(myList);

	if (testy == NULL)
	{
		return 0;
	}

	for (i=0; i<=ListCount(myList); i++)
	{
		if ( !((i+1) % 5))
		{
			testy = ListRemove(myList);
			if (testy == NULL)
			{
				return 0;
			}
		}
		else
		{
			testy = ListNext(myList);
			if (testy == NULL)
			{
				return 0;
			}
		}
	}
	
	printf("\n\nDeleting some elements in the end of the list\n");

	testy = (int *) ListLast(myList);

	if (testy == NULL)
	{
		return 0;
	}

	for (i=0; i<3; i++)
	{
		testy = ListRemove(myList);
		if (testy == NULL)
			{
				return 0;
			}
	}
	
	printf("The new contents of the list is:\n");

	if (!PrintForward(myList))
	{
		return 0;
	}

	printf("\n\nIn Reverse\n");	
	
	printf("The contents of list1 is: \n");
	
	if (!PrintReverse(myList))
	{
		return 0;
	}

	testy = (int *) ListLast(myList);

	if (testy == NULL)
	{
		return 0;
	}
	
	for(i=15; i>=1; i--)
	{
		if ( (testy = (int *) ListRemove(myList)) == NULL )
		{
			return 0;
		}
	}

	printf("\n\n\n All functions have now had basic functionality \
tested\n\n");

	ListFree(myList, &itemFree);

	return 1;
}

int stageThree(void)
{
	LIST *brutal = NULL;
	LIST *brutal2 = NULL;
	int testme = 49, i=0;
	
	printf("\n\n ===== STAGE 3 ===== \n\n");
	
	if (ListAdd(brutal, &testme) != -1)
	{
		return 0;
	}

	if (ListInsert(brutal, &testme) != -1)
	{
		return 0;
	}

	if (ListAppend(brutal, &testme) != -1)
	{
		return 0;
	}

	if (ListPrepend(brutal, &testme) != -1)
	{
		return 0;
	}

	if (ListCurr(brutal) != NULL)
	{
		return 0;
	}

	if (ListNext(brutal) != NULL)
	{
		return 0;
	}

	if (ListPrev(brutal) != NULL)
	{
		return 0;
	}

	if (ListFirst(brutal) != NULL)
	{
		return 0;
	}

	if (ListLast(brutal) != NULL)
	{
		return 0;
	}

	if (ListCount(brutal) != -1)
	{
		return 0;
	}

	if (ListRemove(brutal) != NULL)
	{
		return 0;
	}
	
	if (ListTrim(brutal) != NULL)
	{
		return 0;
	}
	
	ListFree(brutal, &itemFree); 
	
	if (ListSearch(brutal, &comparator, &testme) != NULL)
	{
		return 0;
	}

	ListConcat(brutal, brutal2);

	printf("NULL list tests completed\n\n");

	brutal = ListCreate();
	
	printf("listAdd(brutal, NULL), result: %i\n",
		ListAdd(brutal, NULL));
	printf("listInsert(brutal, NULL), result: %i\n", 
		ListInsert(brutal, NULL));
	printf("listAppend(brutal, NULL), result: %i\n", 
		ListAppend(brutal, NULL));
	printf("listPrepend(brutal, NULL), result: %i\n",
		ListPrepend(brutal, NULL));
	printf("\nlistFirst(brutal), result: %x\n", ListFirst(brutal));
	printf("listCurr(brutal), result: %x\n", ListCurr(brutal));
	printf("listPrev(brutal), result: %x\n", ListPrev(brutal));
	printf("listLast(brutal), result: %x\n", ListLast(brutal)); 
	printf("\nlistCount(brutal), result: %i\n", ListCount(brutal));
	printf("\nlistRemove(brutal), result: %x\n", 
		ListRemove(brutal)); 
	printf("\nlistCount(brutal), result: %i\n", ListCount(brutal));
	printf("\nlistTrim(brutal), result: %x\n", ListTrim(brutal));
	printf("\nlistCount(brutal), result: %i\n", ListCount(brutal));
	printf("\nlistSearch(brutal, &comparator, NULL), result: %x\n",
		ListSearch(brutal, &comparator, NULL));
	ListFree(brutal, &itemFree1);

	printf("\nInsertion of NULL items complete \n");

	brutal = ListCreate();
	brutal2 = ListCreate();

	if (ListAdd(brutal, &testme) == -1)
	{
		return 0;
	}

	ListConcat(brutal, brutal2);

	PrintForward(brutal);

	brutal2 = ListCreate();

	ListConcat(brutal2, brutal);

	PrintForward(brutal2);
	
	ListFree(brutal, &itemFree1);
	ListFree(brutal2, &itemFree1);
	
	printf("\nConcatination of list with NULL list, and \
NULL list with list: complete\n");
	return 1;
}

int stageFour(void)
{
	int testme = 49;
	long int i=0;
	LIST *big;
	
	big = ListCreate();

	printf("\n\n===== STAGE 4 =====\n\n");

	while(ListAdd(big, &testme) != -1)
	{
		i++;
	}

	printf("A total of %d nodes where created\n", i);

	ListFree(big, &itemFree1);
	return 1;
}

int comparator(void *item1, void *item2)
{
	if ( (item1 == NULL) || (item2 == NULL) )
	{
		printf("\nComparator Ignoring NULL pointer\n");
		return 0;
	}
	return ( (*(int *)item1) == (*(int *)item2) );
}

void itemFree(void *item)
{
	free((int *)item);
}

void itemFree1(void *item)
{
}

int PrintForward(LIST *myList)
{
	int *tester, i=0;
	
	tester = ListFirst(myList);

	if (tester == NULL)
	{
		return 0;
	}
	
	printf("Item 1: %i\t", *tester);
	
	for(i=1; i<ListCount(myList); i++)
	{
		if ( (tester = ListNext(myList)) == NULL )
		{
			return 0;
		}
		printf("Item %i: %i\t", (i+1), *tester);	
		if( !((i+1) % 5))
		{
			printf("\n");
		}
	}

	return 1;
}

int PrintReverse(LIST *myList)
{
	int *tester, i=0;
	
	tester = ListLast(myList);

	if (tester == NULL)
	{
		return 0;
	}
	
	printf("Item %i: %i\t", ListCount(myList), *tester);
	
	for(i=(ListCount(myList)-1); i>0; i--)
	{
		if ( (tester = ListPrev(myList)) == NULL )
		{
			return 0;
		}
		printf("Item %i: %i\t", i, *tester);	
		if( !((i-1)% 5) )
		{
			printf("\n");
		}
	}

	return 1;
}
