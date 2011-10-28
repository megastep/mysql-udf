/*
  returns the kurtosis of the values in a distribution

  input parameters:
  data (real)
  number of decimals in result (int, optional)

  output:
  kurtosis of the distribution (real)

  registering the function:
  CREATE AGGREGATE FUNCTION kurtosis RETURNS REAL SONAME 'udf_kurtosis.so';

  getting rid of the function:
  DROP FUNCTION kurtosis;

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


#define BUFFERSIZE 1024		



extern "C" {
my_bool kurtosis_init( UDF_INIT* initid, UDF_ARGS* args, char* message );
void kurtosis_deinit( UDF_INIT* initid );
void kurtosis_clear(UDF_INIT *initid, char *is_null, char *is_error);
void kurtosis_reset( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );
void kurtosis_add( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );
double kurtosis( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );
}


struct kurtosis_data
{
  unsigned long count;
  unsigned long abscount;
  unsigned long pages;
  double *values;
};


my_bool kurtosis_init( UDF_INIT* initid, UDF_ARGS* args, char* message )
{
  if (args->arg_count < 1 || args->arg_count>2)
  {
    strcpy(message,"wrong number of arguments: kurtosis() requires one or two arguments");
    return 1;
  }

  if (args->arg_type[0]!=REAL_RESULT)
  {
    strcpy(message,"kurtosis() requires a real as parameter 1");
    return 1;
  }

  if (args->arg_count>1 && (args->arg_type[1]!=INT_RESULT))
  {
    strcpy(message,"kurtosis() requires an int as parameter 2");
    return 1;
  }

  initid->decimals=2;
  if (args->arg_count==2 && (*((ulong*)args->args[1])<=16))
  {
    initid->decimals=*((ulong*)args->args[1]);
  }

  kurtosis_data *buffer = new kurtosis_data;
  buffer->count = 0;
  buffer->abscount=0;
  buffer->pages = 1;
  buffer->values = NULL;

  initid->maybe_null	= 1;
  initid->max_length	= 32;
  initid->ptr = (char*)buffer;

  return 0;
}


void kurtosis_deinit( UDF_INIT* initid )
{
  kurtosis_data *buffer = (kurtosis_data*)initid->ptr;

  if (buffer->values != NULL)
  {
    free(buffer->values);
    buffer->values=NULL;
  }
  delete initid->ptr;
}

void kurtosis_clear(UDF_INIT *initid, char *is_null, char *is_error)
{
  kurtosis_data *buffer = (kurtosis_data*)initid->ptr;
  buffer->count = 0;
  buffer->abscount=0;
  buffer->pages = 1;
  *is_null = 0;
  *is_error = 0;

  if (buffer->values != NULL)
  {
    free(buffer->values);
    buffer->values=NULL;
  }

  buffer->values=(double *) malloc(BUFFERSIZE*sizeof(double));
}

void kurtosis_reset( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error )
{
  kurtosis_clear(initid, is_null, is_error);
  kurtosis_add( initid, args, is_null, is_error );
}


void kurtosis_add( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error )
{
  if (args->args[0]!=NULL)
  {
    kurtosis_data *buffer = (kurtosis_data*)initid->ptr;
    if (buffer->count>=BUFFERSIZE)
    {
      buffer->pages++;
      buffer->count=0;
      buffer->values=(double *) realloc(buffer->values,BUFFERSIZE*buffer->pages*sizeof(double));
    }
    buffer->values[buffer->abscount++] = *((double*)args->args[0]);
    buffer->count++;
  }
}



double kurtosis( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error )
{
  kurtosis_data* buffer = (kurtosis_data*)initid->ptr;

  *is_null=1;
  if (buffer->abscount<2 || *is_error!=0)
  {
    return 0.0;
  }

  ulong i;
  double mean=0.0; double term=0.0; double stddev=0.0; double kurtosis=0.0;

  for (i=0;i<buffer->abscount;++i)
  {
    mean+=buffer->values[i];
  }

  mean/=(double) buffer->abscount;
  
  for (i=0;i<buffer->abscount;++i)
  {
    term=buffer->values[i]-mean;
    stddev+=term*term;

    kurtosis+=term*term*term*term;
  }

  stddev/=(double) buffer->abscount;
  stddev=pow(stddev,0.5);

  if (stddev==0.0)
  {
    return 0.0;
  }

  *is_null=0;
  kurtosis/=((double) buffer->abscount)*stddev*stddev*stddev*stddev;

  return kurtosis-3.0;
}

#endif

