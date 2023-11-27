/*
 * Copyright 2013, SoS Laboratory, Lehigh University
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of SoS Laboratory, Lehigh University nor the
 * names of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "JavaTypeParser.h"

//code originally appeared in jnicheck.h

// a global pointer pointing to the next char.
const char *nextchar;

//This is the entry point method
// for a method type, the first one in the list is the return type;
// others are argument types.
// If returns NULL, there was some error in parsing.
// This function will malloc some space; the space will be managed by GC.
JavaType * parsecompletemethodtype (const char * s) {
  // initialize the global pointer
  nextchar = s;

  JavaType * tp = NULL;

  if (*nextchar != '(') return NULL;
  nextchar++;
  tp = parsesimpletypelist();
  if (*nextchar != ')') return NULL;
  nextchar++;

  JavaType *tp1 = parsesimpletype(); // the return type
  if (*nextchar != '\0') return NULL;
  tp1->next = tp;

  return tp1;
    
}

// Takes a string as an input, and returns a JavaType pointer;
// When the string is not valid, return NULL pointer.
// The pointer s will move to the next token.
JavaType * parsesimpletype () {
  JavaType * tp = (JavaType *) malloc (sizeof (JavaType));
  tp->next = NULL;

  int len;

  switch (*nextchar) {
   case 'Z' :
     tp->tk = simpletype; tp->st = booleantype; tp->name = NULL;
     nextchar++; break;
   case 'B' :
     tp->tk = simpletype; tp->st = bytetype; tp->name = NULL;
     nextchar++; break;
   case 'C' :
     tp->tk = simpletype; tp->st = chartype; tp->name = NULL;
     nextchar++; break;
   case 'D' :
     tp->tk = simpletype; tp->st = doubletype; tp->name = NULL;
     nextchar++; break;
   case 'F' :
     tp->tk = simpletype; tp->st = floattype; tp->name = NULL;
     nextchar++; break;
   case 'I' :
     tp->tk = simpletype; tp->st = inttype; tp->name = NULL;
     nextchar++; break;
   case 'J' :
     tp->tk = simpletype; tp->st = longtype; tp->name = NULL;
     nextchar++; break;
   case 'S' :
     tp->tk = simpletype; tp->st = shorttype; tp->name = NULL;
     nextchar++; break;
   case 'V' :
     tp->tk = simpletype; tp->st = voidtype; tp->name = NULL;
     nextchar++; break;

   case 'L' :
     tp->tk = classtype;
     len = parsesimpletypehelper(nextchar);
     if (len == 0) return NULL;
 
     // strip out the begining 'L' and the end ';'
     tp->name = (char *) malloc (len-2+1);
     strncpy(tp->name, nextchar+1, len-2); 
     *(tp->name + len - 2) = '\0';
     nextchar +=  len; break;

   case '[' :
     tp->tk = arraytype;
     len = parsesimpletypehelper(nextchar);
     if (len == 0) return NULL;
     tp->name = (char *) malloc (len+1);
     strncpy(tp->name, nextchar, len);
     *(tp->name + len) = '\0';
     nextchar += len; break;

   default : return NULL;
  }

  return tp;
}

// Takes a string as an input, and returns the length of the type signature.
// When the string is not valid, return 0;
int parsesimpletypehelper () {
  BOOL over = FALSE;
  int len = 0;

  while (!over) {
    switch (*(nextchar + len)) {
     case 'Z': case 'B': case 'C': case 'D':
     case 'F': case 'I': case 'J': case 'S' : case 'V':
       len++; over = TRUE; break;

     case 'L':
       len++;
       while (*(nextchar+len) != ';') {
         if (* (nextchar+len) == '\0') return 0;
         len ++;
       }
       len++; over = TRUE;
       break;
       
     case '[':
       len ++;
       break;
       
     default : return 0;
    }
  }
  return len;
}


JavaType * parsesimpletypelist () {
  JavaType * tp = NULL;
  JavaType * tp1 = NULL;
  JavaType * head = parsesimpletype();

  if(head != NULL)
  {
	tp = head;
	tp1=parsesimpletype();
	while(tp1)
	{
		tp->next = tp1;
		tp=tp1;
		tp1=parsesimpletype();
	}
	tp->next=NULL;
  }

  return head;
}


JavaType * parsecompletesimpletype (const char * s) {
  nextchar = s;
  JavaType * tp = parsesimpletype (s);
  // Has to be end of tokens.
  if (*nextchar == '\0') return tp;
  else return NULL;
}

void printsimpletype (JavaType *tp) {
  if (tp == NULL) return;

  switch (tp->tk) {
   case simpletype:
     switch (tp->st) {
      case booleantype:
        printf("Z");break;
      case bytetype:
        printf("B");break;
      case chartype:
        printf("C");break;
      case doubletype:
        printf("D");break;
      case floattype:
        printf("F");break;
      case inttype:
        printf("I");break;
      case longtype:
        printf("J");break;
      case shorttype:
        printf("S");break;
      case voidtype:
        printf("V");break;
      default: 
	//what to do here for an invalid signature element?
	break;
     }
     break;

   case classtype:
     printf("L%s;", tp->name);
     break;

   case arraytype:
     printf("%s", tp->name);
     break;

   default: 
	//what to do here for an impossible type?
	break;
  }
  return;
}

void printmethodtype (JavaType *tp) {
  if (tp == NULL) return;

  printf("(");
  JavaType *tp1 = tp -> next;
  while (tp1) {
    printsimpletype(tp1); tp1 = tp1->next;
  }
  printf(")");

  printsimpletype(tp);
}

