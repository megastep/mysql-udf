/*
  returns the faculty of a value

  input parameters:
  value (int or real)

  output:
  faculty of the value (real)

  registering the function:
  CREATE FUNCTION faculty RETURNS REAL SONAME 'udf_faculty.so';

  getting rid of the function:
  DROP FUNCTION faculty;

*/

#ifdef STANDARD
#include <stdio.h>
#include <string.h>
#ifdef __WIN__
typedef unsigned __int64 ulonglong;	
typedef __int64 longlong;
#else
typedef unsigned long long ulonglong;
typedef long long longlong;
#endif /*__WIN__*/
#else
#include <my_global.h>
#include <my_sys.h>
#endif
#include <mysql.h>
#include <m_ctype.h>
#include <m_string.h>		

#ifdef HAVE_DLOPEN


extern "C" 
{

my_bool faculty_init(UDF_INIT *, UDF_ARGS *args, char *message);
double faculty(UDF_INIT *initid, UDF_ARGS *args, char *is_null,char *error);

}


my_bool faculty_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
  if (args->arg_count != 1)
  {
    strcpy(message,"faculty() must have exactly one argument");
    return 1;
  }

  if (args->arg_type[0] != INT_RESULT && args->arg_type[0]!=REAL_RESULT)
  {
    strcpy(message,"faculty() must have an integer or a real as parameter 1");
    return 1;
  }
  initid->maybe_null    = 0; 
  initid->decimals      = 0;
  initid->max_length    = 20;

  return 0;
}


double faculty(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error)
{
  double result=0.0;
  longlong i;
  double initvalue=0.0;

  if (args->arg_type[0]==INT_RESULT)
  {
    initvalue=(double) *((longlong*)args->args[0]);
  } else if (args->arg_type[0]==REAL_RESULT)
  {
    initvalue=(double) *((double*) args->args[0]);
  }

  if (initvalue>0.0)
  {
    result=1.0;
    for (i=2;i<=(longlong) initvalue;i++)
    {
      result*=(double) i;
    }
  }

  return result;
}


#endif /* HAVE_DLOPEN */


