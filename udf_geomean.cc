/*
  returns the geometric mean of values

  input parameters:
  data (real)
  number of decimals in result (int, optional)

  output:
  geometric mean of the values (real)

  registering the function:
  CREATE AGGREGATE FUNCTION geomean RETURNS REAL SONAME 'udf_geomean.so';

  getting rid of the function:
  DROP FUNCTION geomean;

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



extern "C" {
my_bool geomean_init( UDF_INIT* initid, UDF_ARGS* args, char* message );
void geomean_deinit( UDF_INIT* initid );
void geomean_reset( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );
void geomean_add( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );
double geomean( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );
}


struct geomean_data
{
  unsigned long count;
  bool isset;
  double value;
};


my_bool geomean_init( UDF_INIT* initid, UDF_ARGS* args, char* message )
{
  if (args->arg_count < 1 || args->arg_count>2)
  {
    strcpy(message,"wrong number of arguments: geomean() requires one or two arguments");
    return 1;
  }

  if (args->arg_type[0]!=REAL_RESULT)
  {
    strcpy(message,"geomean() requires a real as parameter 1");
    return 1;
  }

  if (args->arg_count>1 && (args->arg_type[1]!=INT_RESULT))
  {
    strcpy(message,"geomean() requires an int as parameter 2");
    return 1;
  }

  initid->decimals=2;
  if (args->arg_count==2 && (*((ulong*)args->args[1])<=16))
  {
    initid->decimals=*((ulong*)args->args[1]);
  }

  geomean_data *buffer = new geomean_data;
  buffer->count = 0;
  buffer->value=0;
  buffer->isset=false;

  initid->maybe_null	= 1;
  initid->max_length	= 32;
  initid->ptr = (char*)buffer;

  return 0;
}


void geomean_deinit( UDF_INIT* initid )
{
  delete initid->ptr;
}



void geomean_reset( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error )
{
  geomean_data *buffer = (geomean_data*)initid->ptr;
  buffer->count = 0;
  buffer->value=0;
  buffer->isset=false;
  *is_null = 0;
  *is_error = 0;

  geomean_add( initid, args, is_null, is_error );
}


void geomean_add( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error )
{
  if (args->args[0]!=NULL)
  {
    double value;

    geomean_data *buffer = (geomean_data*)initid->ptr;

    value=(double) *((double*) args->args[0]);
    buffer->count++;
    if (buffer->isset)
    {
      buffer->value*=value;
    } else
    {
      buffer->value=value;
      buffer->isset=true;
    }
  }
}


double geomean( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error )
{
  geomean_data* buffer = (geomean_data*)initid->ptr;

  if (buffer->count==0 || *is_error!=0)
  {
    *is_null = 1;
    return 0.0;
  }

  *is_null=0;
  if (buffer->count==1)
  {
    return buffer->value;
  }

  return (double) pow(buffer->value,1.0/buffer->count);
}

#endif

