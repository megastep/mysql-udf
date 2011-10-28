/*
  returns the weighted average of values

  input parameters:
  data (real)
  weight (real)
  number of decimals in result (int, optional)

  output:
  weighted of the values (real)

  registering the function:
  CREATE AGGREGATE FUNCTION weightedavg RETURNS REAL SONAME 'udf_weightedavg.so';

  getting rid of the function:
  DROP FUNCTION weightedavg;

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
my_bool weightedavg_init( UDF_INIT* initid, UDF_ARGS* args, char* message );
void weightedavg_deinit( UDF_INIT* initid );
void weightedavg_reset( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );
void weightedavg_add( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );
double weightedavg( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );
}


struct weightedavg_data
{
  unsigned long count;
  double datasum;
  double weightsum;
};


my_bool weightedavg_init( UDF_INIT* initid, UDF_ARGS* args, char* message )
{
  if (args->arg_count < 2 || args->arg_count>3)
  {
    strcpy(message,"wrong number of arguments: weightedavg() requires two or three arguments");
    return 1;
  }

  if (args->arg_type[0]!=REAL_RESULT)
  {
    strcpy(message,"weightedavg() requires a real as parameter 1");
    return 1;
  }

  if (args->arg_type[1]!=REAL_RESULT)
  {
    strcpy(message,"weightedavg() requires a real as parameter 2");
    return 1;
  }

  if (args->arg_count>2 && (args->arg_type[3]!=INT_RESULT))
  {
    strcpy(message,"weightedavg() requires an int as parameter 3");
    return 1;
  }

  initid->decimals=2;
  if (args->arg_count==3 && (*((ulong*)args->args[2])<=16))
  {
    initid->decimals=*((ulong*)args->args[2]);
  }

  weightedavg_data *buffer = new weightedavg_data;
  buffer->count = 0;
  buffer->datasum = 0;
  buffer->weightsum = 0;

  initid->maybe_null	= 1;
  initid->max_length	= 32;
  initid->ptr = (char*)buffer;

  return 0;
}


void weightedavg_deinit( UDF_INIT* initid )
{
  delete initid->ptr;
}

void weightedavg_clear(UDF_INIT *initid, char *is_null, char *is_error)
{
  weightedavg_data *buffer = (weightedavg_data*)initid->ptr;
  buffer->count = 0;
  buffer->datasum = 0;
  buffer->weightsum = 0;
  *is_null = 0;
  *is_error = 0;
}

void weightedavg_reset( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error )
{
  weightedavg_clear(initid, is_null, is_error);
  weightedavg_add( initid, args, is_null, is_error );
}


void weightedavg_add( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error )
{
  if (args->args[0]!=NULL && args->args[1]!=NULL)
  {
    weightedavg_data *buffer = (weightedavg_data*)initid->ptr;

    buffer->datasum+=(double) *((double*) args->args[0])*(double) *((double*) args->args[1]);
    buffer->weightsum+=(double) *((double*) args->args[1]);
    buffer->count++;
  }
}


double weightedavg( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error )
{
  weightedavg_data* buffer = (weightedavg_data*)initid->ptr;

  if (buffer->count==0 || buffer->weightsum==0.0 || *is_error!=0)
  {
    *is_null = 1;
    return 0.0;
  }

  *is_null=0;

  return (double) buffer->datasum/buffer->weightsum;
}

#endif

