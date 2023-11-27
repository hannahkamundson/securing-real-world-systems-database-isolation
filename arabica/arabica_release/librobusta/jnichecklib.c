#include "jnicheckcommon.h"

// Headers file and defintions from this point cannot be put into
// jnicheckcommon.h.  The reason is that jnicheckcommon.h will also be
// included by jnicheck.h, which will be inserted into the source C
// file. If stdio.h is included into jnicheck.h, there will be a type
// redefinition.

#include <stddef.h>  // include def of NULL
#include <ccuredcheck.h>

struct __ccured_va_list;
struct __ccured_va_localinfo {
   int next ;
   int count ;
   int tags[32] ;
   void *    nextp ;
};

extern int __ccured_va_count ;
extern int __ccured_va_tags[32] ;

extern void __ccured_va_start_vs(struct __ccured_va_localinfo*, unsigned long);
extern void __ccured_va_end_vs(struct __ccured_va_localinfo *);
extern void * __ccured_va_arg_svs
  (struct  __ccured_va_localinfo *, unsigned int, int);


//-----------------------------------------------------------------------------

// If argument type checking is not successful, halt.
// uses the ccured facility __ccured_va_arg_svs to check the case of insufficient arguments and the case of wrong basic types.
void argsTypeCheck (JNIEnv * env, JavaType *tp, va_list args) {

  struct __ccured_va_localinfo vainfo;
  vainfo.nextp = NULL;
  struct __ccured_va_localinfo * cap = &vainfo;
  __ccured_va_start_vs (cap, (unsigned long) args);


  jobject obj;
  jclass cls;

  ASSERT_BUG (tp, "The type signature pointer is NULL.");

  tp = tp -> next; // skip the return type.

  while (tp) {
    switch (tp->tk) {
     case simpletype:
       // for simple types, no need to do type checking
       switch (tp->st) {
        case booleantype: // jboolean is promoted to int, with tag 0
          (int *)(__ccured_va_arg_svs(cap, sizeof(int), 0));
          // va_arg(args, int);
          break;
        case bytetype: // jbyte is promoted to int, with tag 0
          (int *)(__ccured_va_arg_svs(cap, sizeof(int), 0));
          break;
        case chartype: // jchar is promoted to int, with tag 0
          (int *)(__ccured_va_arg_svs(cap, sizeof(int), 0));
          break;
        case doubletype: // tag is 6
          (double *)(__ccured_va_arg_svs(cap, sizeof(double), 6));
          break;
        case floattype: // jfloat is promoted to double, with tag 6
          (double *)(__ccured_va_arg_svs(cap, sizeof(double), 6));
          break;
        case inttype: // tag is 0
          (int *)(__ccured_va_arg_svs(cap, sizeof(int), 0));
          break;
        case longtype: // tag is 5
          (long *)(__ccured_va_arg_svs(cap, sizeof(long), 5));
          break;
        case shorttype: // jshort is promoted to int, with tag 0
          (int *)(__ccured_va_arg_svs(cap, sizeof(int), 0));
          break;
        default: BUG ("Impossible Java simple types.");
       }
       break;

     case classtype: // tag is 8
     case arraytype: // tag is 8
       // obj = va_arg(args, jobject);
       obj = *((jobject *)(__ccured_va_arg_svs(cap, sizeof(jobject), 8)));

       // need to free the local refs.
       cls = (*env)->FindClass(env, tp->name);
       ASSERT_BUG_ARGS(cls, ("Cannot find the class %s.", tp->name));


       ASSERT_SAFETY_ARGS((*env)->IsInstanceOf(env, obj, cls),
                          ("The object is not of the class %s.", tp->name));

       (*env)->DeleteLocalRef(env, cls);
       
       break;

     default:  BUG("Impossible Java type kinds.");
    }
    tp = tp -> next;
  }

  __ccured_va_end_vs (cap);
}

struct seq_char {
   char *  _p ;
   struct meta_seq _ms ;
}   ;
typedef struct seq_char seq_char;

struct seq_void {
  void *  _p ;
  struct meta_seq _ms ;
}   ;
typedef struct seq_void seq_void;


// for this function, to find out the end bound of the array
// we need to dynamically find the type of the array
INLINE_STATIC_CHECK
seq_void __jni_GetPrimitiveArrayCritical_qssss
(JNIEnvPtr env, jarray array, jboolean *isCopy)
{
  CHECK_EXCEPTION(env);

  jstring jstr = getTypeNameOfObject(env, array);
  const char * name = (*env)->GetStringUTFChars(env, jstr, 0);

  #ifdef RUNTIME_TYPE_CHECKING
    ASSERT_SAFETY(NAME_IS_PRIMITIVEARRAY(name),
                  "The object is not an primitive-typed array.");
  #endif

  seq_void res;
  res._p = (*env)->GetPrimitiveArrayCritical(env, array, isCopy);
  res._ms._b = res._p;
  jsize len = (*env)->GetArrayLength(env, array);
  
  switch (name[1]) {
     case 'Z' :
       res._ms._e = ((jboolean *) res._ms._b) + len;
       break;
     case 'B' :
       res._ms._e = ((jbyte *) res._ms._b) + len;
       break;
     case 'C' :
       res._ms._e = ((jchar *) res._ms._b) + len;
       break;
     case 'S' :
       res._ms._e = ((jshort *) res._ms._b) + len;
       break;
     case 'I' :
       res._ms._e = ((jint *) res._ms._b) + len;
       break;
     case 'J' :
       res._ms._e = ((jlong *) res._ms._b) + len;
       break;
     case 'F' :
       res._ms._e = ((jfloat *) res._ms._b) + len;
       break;
     case 'D' :
       res._ms._e = ((jdouble *) res._ms._b) + len;
       break;

     default :
       BUG ("The object is not an primitive-typed array.");
  }

  (*env)->ReleaseStringUTFChars(env, jstr, name);
  return res;
}

INLINE_STATIC_CHECK
seq_char  __jni_GetStringUTFChars_qssss
(JNIEnv *env , jstring str , jboolean *isCopy)
{
  CHECK_EXCEPTION(env);
  //check that str is of string class
  CHECK_STRING_TYPE(env,str);  

  seq_char f;
  f._p = (char *)((*env)->GetStringUTFChars(env,str,isCopy));
  f._ms._b = f._p;
  /* plus one for the null char at the end */
  f._ms._e = f._p + (*env)->GetStringUTFLength(env,str)+1;
  return f;
}


