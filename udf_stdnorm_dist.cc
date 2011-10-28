/*
  returns the value of the distribution function of 
  the standard normal distribution for a given point

  input parameters:
  point (int or real)
  number of decimals in result (int, optional)

  output:
  value of the distribution function of the standard normal distribution (real)

  registering the function:
  CREATE FUNCTION stdnorm_dist RETURNS REAL SONAME 'udf_stdnorm_dist.so';

  getting rid of the function:
  DROP FUNCTION stdnorm_dist;

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

my_bool stdnorm_dist_init(UDF_INIT *, UDF_ARGS *args, char *message);
double stdnorm_dist(UDF_INIT *initid, UDF_ARGS *args, char *is_null,char *error);

}


my_bool stdnorm_dist_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
  if (args->arg_count != 1 && args->arg_count != 2)
  {
    strcpy(message,"stdnorm_dist() must have one or two arguments");
    return 1;
  }

  if (args->arg_type[0] != INT_RESULT && args->arg_type[0]!=REAL_RESULT)
  {
    strcpy(message,"stdnorm_dist() must have an integer or a real as parameter 1");
    return 1;
  }

  initid->decimals = 2;

  if (args->arg_count==2 && args->arg_type[1]!=INT_RESULT)
  {
    strcpy(message,"stdnorm_dist() must have an integer as parameter 2");
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


double stdnorm_dist(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error)
{
  const double d1 = 0.0498673470;
  const double d2 = 0.0211410061;
  const double d3 = 0.0032776263;
  const double d4 = 0.0000380036;
  const double d5 = 0.0000488906;
  const double d6 = 0.0000053830;

  double val; 
  double z;
  double result;

  if (args->arg_type[0]==INT_RESULT)
  {
    val=(double) *((longlong*)args->args[0]);
  } else if (args->arg_type[0]==REAL_RESULT)
  {
    val=(double) *((double*) args->args[0]);
  }

  z=val;
  if (val<0.0)
  {
    z=-val;
  }

  result=1.0+z*(d1+z*(d2+z*(d3+z*(d4+z*(d5+z*d6)))));
  result*=result;
  result*=result;
  result*=result;
  result*=result;

  if (result==0.0)
  {
    *is_null=1;
    return 0.0;
  }
  *is_null=0;

  result=1.0/(result+result);

  if (val>=0.0)
  {
    result=1.0-result;
  }
  return result;

}


#endif /* HAVE_DLOPEN */


