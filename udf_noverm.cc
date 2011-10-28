/*
  returns n over m

  input parameters:
  n (int or real)
  m (int or real)

  output:
  n over m (real)

  registering the function:
  CREATE FUNCTION noverm RETURNS REAL SONAME 'udf_noverm.so';

  getting rid of the function:
  DROP FUNCTION noverm;

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

my_bool noverm_init(UDF_INIT *, UDF_ARGS *args, char *message);
double noverm(UDF_INIT *initid, UDF_ARGS *args, char *is_null,char *error);

}


my_bool noverm_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
  if (args->arg_count != 2)
  {
    strcpy(message,"noverm() must have exactly two arguments");
    return 1;
  }

  if (args->arg_type[0] != INT_RESULT && args->arg_type[0]!=REAL_RESULT)
  {
    strcpy(message,"noverm() must have an integer or a real as first parameter");
    return 1;
  }

  if (args->arg_type[1] != INT_RESULT && args->arg_type[1]!=REAL_RESULT)
  {
    strcpy(message,"noverm() must have an integer or a real as second parameter");
    return 1;
  }

  initid->maybe_null    = 0; 
  initid->decimals      = 0;
  initid->max_length    = 20;

  return 0;
}


double calcfaculty(double value)
{
  double result=1.0;
  longlong i;

  for (i=2;i<(longlong) value;i++)
  {
    result*=(double) i;
  }

  return result;
}


double noverm(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error)
{
  double result=0.0;
  double n;
  double m;

  if (args->arg_type[0]==INT_RESULT)
  {
    n=(double) *((longlong*)args->args[0]);
  } else if (args->arg_type[0]==REAL_RESULT)
  {
    n=(double) *((double*) args->args[0]);
  }

  if (args->arg_type[1]==INT_RESULT)
  {
    m=(double) *((longlong*)args->args[1]);
  } else if (args->arg_type[1]==REAL_RESULT)
  {
    m=(double) *((double*) args->args[1]);
  }

  if ((n>0.0) && (m>0.0) && (n>m))
  {
    result=calcfaculty(n)/(calcfaculty(m)*calcfaculty(n-m));
  }

  return result;
}


#endif /* HAVE_DLOPEN */


