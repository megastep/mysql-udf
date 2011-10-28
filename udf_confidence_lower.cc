/*
  returns the lower bound of the confidence interval for a given 
  standard deviation, sample size, mean and confidence probability

  input parameters:
  confidence probability (int or real)
  sample size (int)
  arithmetic mean (int or real)
  standard deviation (int or real)
  number of decimals in result (int, optional)

  output:
  lower bound of the confidence interval (real)

  registering the function:
  CREATE FUNCTION confidence_lower RETURNS REAL SONAME 'udf_confidence_lower.so';

  getting rid of the function:
  DROP FUNCTION confidence_lower;

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

my_bool confidence_lower_init(UDF_INIT *, UDF_ARGS *args, char *message);
double confidence_lower(UDF_INIT *initid, UDF_ARGS *args, char *is_null,char *error);

}


my_bool confidence_lower_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
  if (args->arg_count != 4 && args->arg_count != 5)
  {
    strcpy(message,"confidence_lower() must have four or five arguments");
    return 1;
  }

  if (args->arg_type[0] != INT_RESULT && args->arg_type[0]!=REAL_RESULT)
  {
    strcpy(message,"confidence_lower() must have an integer or a real as parameter 1");
    return 1;
  }

  if (args->arg_type[1] != INT_RESULT)
  {
    strcpy(message,"confidence_lower() must have an integer as parameter 1");
    return 1;
  } 

  if (*((ulong*)args->args[1])<1)
  {
    strcpy(message,"confidence_lower() must have a positive value as parameter 1");
    return 1;
  }

  
  if (args->arg_type[2] != INT_RESULT && args->arg_type[2]!=REAL_RESULT)
  {
    strcpy(message,"confidence_lower() must have an integer or a real as parameter 3");
    return 1;
  }

  if (args->arg_type[3] != INT_RESULT && args->arg_type[3]!=REAL_RESULT)
  {
    strcpy(message,"confidence_lower() must have an integer or a real as parameter 4");
    return 1;
  }


  initid->decimals = 2;

  if (args->arg_count==5 && args->arg_type[4]!=INT_RESULT)
  {
    strcpy(message,"confidence_lower() must have an integer as parameter 5");
    return 1;
  }

  if (args->arg_count==5 && (*((ulong*)args->args[4])<=16))
  {
    initid->decimals=*((ulong*)args->args[4]);
  }

  initid->maybe_null    = 0; 
  initid->max_length    = 20;

  return 0;
}


static double invnormalp(double p)
{
  const double p0 = -0.322232431088;
  const double p1 = -1.0;
  const double p2 = -0.342242088547;
  const double p3 = -0.0204231210245;
  const double p4 = -0.453642210148E-4;
  const double q0 =  0.0993484626060;
  const double q1 =  0.588581570495;
  const double q2 =  0.531103462366;
  const double q3 =  0.103537752850;
  const double q4 =  0.38560700634E-2;
  
  double pp, y, xp;

  if (p < 0.5)  
  {
    pp = p;
  } else
  {
    pp = 1 - p;
  }

  if (pp < 1E-12)
  {
    xp = 99;
  } else
  {
    y = sqrt(log(1/(pp*pp)));
    xp = y + ((((y * p4 + p3) * y + p2) * y + p1) * y + p0) /
             ((((y * q4 + q3) * y + q2) * y + q1) * y + q0);
  }

  if (p < 0.5)  
  {
    return -xp;
  } else
  {
    return  xp;
  }
}


double confidence_lower(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error)
{
  double pz;
  double su;
  double m;
  double stddev;
  double t;

  if (args->arg_type[0]==INT_RESULT)
  {
    pz=(double) *((longlong*)args->args[0]);
  } else if (args->arg_type[0]==REAL_RESULT)
  {
    pz=(double) *((double*) args->args[0]);
  }
  
  if (args->arg_type[1]==INT_RESULT)
  {
    su=(double) *((longlong*)args->args[1]);
  } else if (args->arg_type[1]==REAL_RESULT)
  {
    su=(double) *((double*) args->args[1]);
  }

  if (args->arg_type[2]==INT_RESULT)
  {
    m=(double) *((longlong*)args->args[2]);
  } else if (args->arg_type[2]==REAL_RESULT)
  {
    m=(double) *((double*) args->args[2]);
  }

  if (args->arg_type[3]==INT_RESULT)
  {
    stddev=(double) *((longlong*)args->args[3]);
  } else if (args->arg_type[3]==REAL_RESULT)
  {
    stddev=(double) *((double*) args->args[3]);
  }

  t=invnormalp(pz*0.5+0.5);
  if (t<0.0)
  {
    t=-t;
  }

  return 1.0*m-(stddev*t/sqrt(su)); 

}


#endif /* HAVE_DLOPEN */


