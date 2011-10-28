/*
  returns the intercept of the regression function of a data set 

  input parameters:
  dependent data (real)
  independent data (real)
  number of decimals in result (int, optional)

  output:
  intercept of the regression function (real)

  registering the function:
  CREATE AGGREGATE FUNCTION intercept RETURNS REAL SONAME 'udf_intercept.so';

  getting rid of the function:
  DROP FUNCTION intercept;

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


#define BUFFERSIZE 1024	



extern "C" {
my_bool intercept_init( UDF_INIT* initid, UDF_ARGS* args, char* message );
void intercept_deinit( UDF_INIT* initid );
void intercept_clear(UDF_INIT *initid, char *is_null, char *is_error);
void intercept_reset( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );
void intercept_add( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );
double intercept( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );

}


struct regression_data
{
  unsigned long count;
  unsigned long abscount;
  unsigned long pages;
  double *valuesx;
  double *valuesy;
};


my_bool intercept_init( UDF_INIT* initid, UDF_ARGS* args, char* message )
{
  if (args->arg_count < 2 || args->arg_count>3)
  {
    strcpy(message,"wrong number of arguments: intercept() requires two or three arguments");
    return 1;
  }

  if (args->arg_type[0]!=REAL_RESULT && args->arg_type[0]!=DECIMAL_RESULT)
  {
    strcpy(message,"intercept() requires a real or decimal as parameter 1");
    return 1;
  }

  if (args->arg_type[1]!=REAL_RESULT && args->arg_type[1]!=DECIMAL_RESULT)
  {
    strcpy(message,"intercept() requires a real or decimal as parameter 2");
    return 1;
  }


  if (args->arg_count>2 && (args->arg_type[2]!=INT_RESULT))
  {
    strcpy(message,"intercept() requires an int as parameter 3");
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

  initid->maybe_null	= 1;
  initid->max_length	= 32;
  initid->ptr = (char*)buffer;

  return 0;
}


void intercept_deinit( UDF_INIT* initid )
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

void intercept_clear(UDF_INIT *initid, char *is_null, char *is_error)
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

void intercept_reset( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error )
{
  intercept_clear(initid, is_null, is_error);
  intercept_add( initid, args, is_null, is_error );
}

static double parse_arg(UDF_ARGS *args, int index)
{
	if (args->arg_type[index] == REAL_RESULT) {
		return *((double*)args->args[index]);
	} else {
		double ret;
		char *str;

		str = (char *) malloc(args->lengths[index] + 1);
		strncpy(str, args->args[index], args->lengths[index]);
		str[args->lengths[index]] = '\0';
		sscanf(str, "%lf", &ret);
		free(str);
		return ret;
	}
}


void intercept_add( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error )
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
    buffer->valuesx[buffer->abscount] = parse_arg(args, 0);
    buffer->valuesy[buffer->abscount] = parse_arg(args, 1);
    buffer->abscount++;
    buffer->count++;
  }
}



double intercept( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error )
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

  if (xxsum!=0.0)
  {
    *is_null=0;
    return ymean-(xysum/xxsum)*xmean;
  } else
  {
    *is_null=1;
    return 0.0;
  }

}


