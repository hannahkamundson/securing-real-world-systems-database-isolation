/*
Jon Lovering

FILE:		example.c
DESCRIPTION:	This is an example of using the list to store a structure
DATE:		30/12/05
CONTACT:		jonlovering@ieee.org
*/

#include <stdio.h>
#include <string.h>
#include "list.h"

typedef struct ts {
	char name[30];
	int age;
} Item;

void Populate(Item *s);
void Print(Item *s);

int main (int argc, char **argv)
{
	Item *s;
	LIST *mylist;
	int i;
	
	mylist = ListCreate();

	for(i=0; i<5; i++)
	{
		s = (Item *) malloc (1 * sizeof(Item));
		Populate(s);
		ListAppend(mylist, s);
	}

	i = 0;
	s = (Item *)ListFirst(mylist); /* Ignoring return */
	do
	{
		fprintf(stdout, "Item %i:\n", i+1);
		Print(s);
		i++;
	} while ((s = (Item *)ListNext(mylist)) != NULL);
	return 0;
}

void Populate(Item *s)
{
	char buf[30];
	
	fprintf(stdout, "Enter Name: ");
	if (fgets(buf, 30, stdin) == NULL)
	{
		fprintf(stderr, "EOF\n");
		exit(0);
	}
	strcpy(s->name, buf);

	fprintf(stdout, "Enter Age: ");
	if (fgets(buf, 30, stdin) == NULL)
	{
		fprintf(stderr, "EOF\n");
		exit(0);
	}

	s->age = atoi(buf);
	return;
}

void Print(Item *s)
{
	fprintf(stdout,"\tName: %s\n\tAge: %i\n", s->name, s->age);
}
