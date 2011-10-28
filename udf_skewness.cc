/*
  returns the skewness of the values in a distribution

  input parameters:
  data (real)
  number of decimals in result (int, optional)

  output:
  skewness of the distribution (real)

  registering the function:
  CREATE AGGREGATE FUNCTION skewness RETURNS REAL SONAME 'udf_skewness.so';

  getting rid of the function:
  DROP FUNCTION skewness;

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


#define BUFFERSIZE 1024		// maximum size of sets for which you can give a perfect median



extern "C" {
my_bool skewness_init( UDF_INIT* initid, UDF_ARGS* args, char* message );
void skewness_deinit( UDF_INIT* initid );
void skewness_reset( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );
void skewness_add( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );
double skewness( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );
}


struct skewness_data
{
  unsigned long count;
  unsigned long abscount;
  unsigned long pages;
  double *values;
};


my_bool skewness_init( UDF_INIT* initid, UDF_ARGS* args, char* message )
{
  if (args->arg_count < 1 || args->arg_count>2)
  {
    strcpy(message,"wrong number of arguments: skewness() requires one or two arguments");
    return 1;
  }

  if (args->arg_type[0]!=REAL_RESULT)
  {
    strcpy(message,"skewness() requires a real as parameter 1");
    return 1;
  }

  if (args->arg_count>1 && (args->arg_type[1]!=INT_RESULT))
  {
    strcpy(message,"skewness() requires an int as parameter 2");
    return 1;
  }

  initid->decimals=2;
  if (args->arg_count==2 && (*((ulong*)args->args[1])<=16))
  {
    initid->decimals=*((ulong*)args->args[1]);
  }

  skewness_data *buffer = new skewness_data;
  buffer->count = 0;
  buffer->abscount=0;
  buffer->pages = 1;
  buffer->values = NULL;

  initid->maybe_null	= 1;
  initid->max_length	= 32;
  initid->ptr = (char*)buffer;

  return 0;
}


void skewness_deinit( UDF_INIT* initid )
{
  skewness_data *buffer = (skewness_data*)initid->ptr;

  if (buffer->values != NULL)
  {
    free(buffer->values);
    buffer->values=NULL;
  }
  delete initid->ptr;
}



void skewness_reset( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error )
{
  skewness_data *buffer = (skewness_data*)initid->ptr;
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

  skewness_add( initid, args, is_null, is_error );
}


void skewness_add( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error )
{
  if (args->args[0]!=NULL)
  {
    skewness_data *buffer = (skewness_data*)initid->ptr;
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



double skewness( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error )
{
  skewness_data* buffer = (skewness_data*)initid->ptr;

  *is_null=1;
  if (buffer->abscount<2 || *is_error!=0)
  {
    return 0.0;
  }

  ulong i;
  double mean=0.0; double term=0.0; double stddev=0.0; double skewness=0.0;

  for (i=0;i<buffer->abscount;++i)
  {
    mean+=buffer->values[i];
  }

  mean/=(double) buffer->abscount;
  
  for (i=0;i<buffer->abscount;++i)
  {
    term=buffer->values[i]-mean;
    stddev+=term*term;

    skewness+=term*term*term;
  }

  stddev/=(double) buffer->abscount;
  stddev=pow(stddev,0.5);

  if (stddev==0.0)
  {
    return 0.0;
  }

  *is_null=0;
  skewness/=((double) buffer->abscount)*stddev*stddev*stddev;

  return skewness;
}

#endif

