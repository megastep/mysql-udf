/*
  returns the correlation coefficient of the regression function a data set

  input parameters:
  dependent data (real)
  independent data (real)
  number of decimals in result (int, optional)

  output:
  correlation coefficient of the regression function (real)

  registering the function:
  CREATE AGGREGATE FUNCTION correlation RETURNS REAL SONAME 'udf_correlation.so';

  getting rid of the function:
  DROP FUNCTION correlation;

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

my_bool correlation_init( UDF_INIT* initid, UDF_ARGS* args, char* message );
void correlation_deinit( UDF_INIT* initid );
void correlation_clear(UDF_INIT *initid, char *is_null, char *is_error);
void correlation_reset( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );
void correlation_add( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );
double correlation( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );

}


struct regression_data
{
  unsigned long count;
  unsigned long abscount;
  unsigned long pages;
  double *valuesx;
  double *valuesy;
};


my_bool correlation_init( UDF_INIT* initid, UDF_ARGS* args, char* message )
{
  if (args->arg_count < 2 || args->arg_count>3)
  {
    strcpy(message,"wrong number of arguments: correlation() requires two or three arguments");
    return 1;
  }

  if (args->arg_type[0]!=REAL_RESULT)
  {
    strcpy(message,"correlation() requires a real as parameter 1");
    return 1;
  }

  if (args->arg_type[1]!=REAL_RESULT)
  {
    strcpy(message,"correlation() requires a real as parameter 2");
    return 1;
  }


  if (args->arg_count>2 && (args->arg_type[2]!=INT_RESULT))
  {
    strcpy(message,"correlation() requires an int as parameter 3");
    return 1;
  }

  initid->decimals=2;
  if (args->arg_count==3 && (*((ulong*)args->args[2])<=16))
  {
    initid->decimals=*((ulong*)args->args[2]);
  }

  regression_data *buffer = new regression_data;
  buffer->count = 0;
  buffer->abscount=0;
  buffer->pages = 1;
  buffer->valuesx = NULL;
  buffer->valuesy = NULL;

  initid->maybe_null    = 1;
  initid->max_length    = 32;
  initid->ptr = (char*)buffer;

  return 0;
}


void correlation_deinit( UDF_INIT* initid )
{
  regression_data *buffer = (regression_data*)initid->ptr;

  if (buffer->valuesx != NULL)
  {
    free(buffer->valuesx);
    buffer->valuesx=NULL;
  }
  if (buffer->valuesy != NULL)
  {
    free(buffer->valuesy);
    buffer->valuesy=NULL;
  }
  delete initid->ptr;
}

void correlation_clear(UDF_INIT *initid, char *is_null, char *is_error)
{
  regression_data *buffer = (regression_data*)initid->ptr;
  buffer->count = 0;
  buffer->abscount=0;
  buffer->pages = 1;
  *is_null = 0;
  *is_error = 0;

  if (buffer->valuesx != NULL)
  {
    free(buffer->valuesx);
    buffer->valuesx=NULL;
  }

  if (buffer->valuesy !=NULL)
  {
    free(buffer->valuesy);
    buffer->valuesy=NULL;
  }

  buffer->valuesx=(double *) malloc(BUFFERSIZE*sizeof(double));
  buffer->valuesy=(double *) malloc(BUFFERSIZE*sizeof(double));
}

void correlation_reset( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error )
{
  correlation_clear(initid, is_null, is_error);
  correlation_add( initid, args, is_null, is_error );
}


void correlation_add( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error )
{
  if (args->args[0]!=NULL && args->args[1]!=NULL)
  {
    regression_data *buffer = (regression_data*)initid->ptr;
    if (buffer->count>=BUFFERSIZE)
    {
      buffer->pages++;
      buffer->count=0;
      buffer->valuesx=(double *) realloc(buffer->valuesx,BUFFERSIZE*buffer->pages*sizeof(double));
      buffer->valuesy=(double *) realloc(buffer->valuesy,BUFFERSIZE*buffer->pages*sizeof(double));
    }
    buffer->valuesx[buffer->abscount] = *((double*)args->args[0]);
    buffer->valuesy[buffer->abscount] = *((double*)args->args[1]);
    buffer->abscount++;
    buffer->count++;
  }
}



double correlation( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error )
{
  regression_data* buffer = (regression_data*)initid->ptr;

  if (buffer->abscount==0 || *is_error!=0)
  {
    *is_null = 1;
    return 0.0;
  }

  ulong i;
  double xmean=0.0; double ymean=0.0; double xxsum=0.0; double yysum=0.0; double xysum=0.0; double xvalue; double yvalue;

  for (i=0;i<buffer->abscount;++i)
  {
    xmean+=buffer->valuesx[i];
    ymean+=buffer->valuesy[i];
  }

  xmean/=buffer->abscount;
  ymean/=buffer->abscount;

  for (i=0;i<buffer->abscount;++i)
  {
    xvalue=buffer->valuesx[i]-xmean;
    yvalue=buffer->valuesy[i]-ymean;

    xxsum+=xvalue*xvalue;
    yysum+=yvalue*yvalue;
    xysum+=xvalue*yvalue;
  }

  if ((xxsum*yysum)>=0.0)
  {
    return xysum/sqrt(xxsum*yysum);
  } else
  {
    return 0.0;
  }
}



#endif


