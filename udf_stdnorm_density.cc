/*
  returns the value of the density function of
  the standard normal distribution for a given point

  input parameters:
  point (int or real)
  number of decimals in result (int, optional)

  output:
  value of the density function of the standard normal distribution (real)

  registering the function:
  CREATE FUNCTION stdnorm_density RETURNS REAL SONAME 'udf_stdnorm_density.so';

  getting rid of the function:
  DROP FUNCTION stdnorm_density;

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

my_bool stdnorm_density_init(UDF_INIT *, UDF_ARGS *args, char *message);
double stdnorm_density(UDF_INIT *initid, UDF_ARGS *args, char *is_null,char *error);

}


my_bool stdnorm_density_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
  if (args->arg_count != 1 && args->arg_count != 2)
  {
    strcpy(message,"stdnorm_density() must have one or two arguments");
    return 1;
  }

  if (args->arg_type[0] != INT_RESULT && args->arg_type[0]!=REAL_RESULT)
  {
    strcpy(message,"stdnorm_density() must have an integer or a real as parameter 1");
    return 1;
  }

  initid->decimals = 2;

  if (args->arg_count==2 && args->arg_type[1]!=INT_RESULT)
  {
    strcpy(message,"stdnorm_density() must have an integer as parameter 2");
    return 1;
  }

  if (args->arg_count==2 && (*((ulong*)args->args[1])<=16))
  {
    initid->decimals=*((ulong*)args->args[1]);
  }

  initid->maybe_null    = 0; 
  initid->max_length    = 20;

  return 0;
}


double stdnorm_density(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error)
{
  double x;

  if (args->arg_type[0]==INT_RESULT)
  {
    x=(double) *((longlong*)args->args[0]);
  } else if (args->arg_type[0]==REAL_RESULT)
  {
    x=(double) *((double*) args->args[0]);
  }

  return pow(2.71828182845904523536,-0.5*x*x)/2.506628274631000241612355;
}


#endif /* HAVE_DLOPEN */


