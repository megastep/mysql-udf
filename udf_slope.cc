/*
  returns the slope of the linear regression function of a data set

  input parameters:
  dependent data (real)
  independent data (real)
  number of decimals in result (int, optional)

  output:
  slope of the linear regression function (real)

  registering the function:
  CREATE AGGREGATE FUNCTION slope RETURNS REAL SONAME 'udf_slope.so';

  getting rid of the function:
  DROP FUNCTION slope;

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

my_bool slope_init( UDF_INIT* initid, UDF_ARGS* args, char* message );
void slope_deinit( UDF_INIT* initid );
void slope_clear(UDF_INIT *initid, char *is_null, char *is_error);
void slope_reset( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );
void slope_add( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );
double slope( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );

}


struct regression_data
{
  unsigned long count;
  unsigned long abscount;
  unsigned long pages;
  double *valuesx;
  double *valuesy;
};



my_bool slope_init( UDF_INIT* initid, UDF_ARGS* args, char* message )
{
  if (args->arg_count < 2 || args->arg_count>3)
  {
    strcpy(message,"wrong number of arguments: slope() requires two or three arguments");
    return 1;
  }

  if (args->arg_type[0]!=REAL_RESULT && args->arg_type[0]!=DECIMAL_RESULT)
  {
    strcpy(message,"slope() requires a real or decimal as parameter 1");
    return 1;
  }

  if (args->arg_type[1]!=REAL_RESULT && args->arg_type[1]!=DECIMAL_RESULT)
  {
    strcpy(message,"slope() requires a real or decimal as parameter 2");
    return 1;
  }


  if (args->arg_count>2 && (args->arg_type[2]!=INT_RESULT))
  {
    strcpy(message,"slope() requires an int as parameter 3");
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


void slope_deinit( UDF_INIT* initid )
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

void slope_clear(UDF_INIT *initid, char *is_null, char *is_error)
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

void slope_reset( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error )
{
  slope_clear(initid, is_null, is_error);
  slope_add( initid, args, is_null, is_error );
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

void slope_add( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error )
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

double slope( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error )
{
  regression_data* buffer = (regression_data*)initid->ptr;

  if (buffer->abscount==0 || *is_error!=0)
  {
    *is_null = 1;
    return 0.0;
  }

  ulong i;
  double xmean=0.0; double ymean=0.0; double xxsum=0.0; double xysum=0.0; double xvalue; double yvalue;

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
    xysum+=xvalue*yvalue;
  }

  if (xxsum!=0.0)
  {
    *is_null=0;
    return xysum/xxsum;
  } else
  {
    *is_null=1;
    return 0.0;
  }
}



