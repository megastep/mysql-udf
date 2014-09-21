/*
  returns the longest value of a set

  input parameters:
  data (string)

  output:
  longest value (string)

  registering the function:
  CREATE AGGREGATE FUNCTION longest RETURNS STRING SONAME 'udf_longest.so';

  getting rid of the function:
  DROP FUNCTION longest;

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
my_bool longest_init( UDF_INIT* initid, UDF_ARGS* args, char* message );
void longest_deinit( UDF_INIT* initid );
void longest_clear( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error );
void longest_reset( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );
void longest_add( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );
char *longest(UDF_INIT * initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char * /*error*/ );
}


struct longest_data
{
  long long	length;
  char *result_string;
};


my_bool longest_init( UDF_INIT* initid, UDF_ARGS* args, char* message )
{
  struct longest_data*	data = NULL;

  if (args->arg_count != 1)
  {
    strcpy(message, "wrong number of arguments: longest() requires exactly one argument");
    return 1;
  }

  if ( args->arg_type[0] != STRING_RESULT )
  {
    strcpy(message, "wrong argument type: longest() requires a string as parameter 1");
    return 1;
  }
  data = new struct longest_data;
  data->length=0;
  data->result_string	= NULL;

  initid->maybe_null	= 0;		  
  initid->max_length	= 65535;	
  initid->ptr = (char*)data;

  return 0;
}

void longest_deinit( UDF_INIT* initid )
{
  struct longest_data* data = (struct longest_data*)initid->ptr;
  if (data != NULL)
  {
    if (data->result_string!= NULL)
    {
      free(data->result_string);
      data->result_string	= NULL;
    }
    data->length = 0;
    delete data;
    initid->ptr = NULL;
  }
}

void longest_clear( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error ){
  struct longest_data* data = (struct longest_data*) initid->ptr;
  if (data->result_string != NULL)
  {
    free(data->result_string);
    data->result_string	= NULL;
  }
  data->length = 0;
  *is_null = 0;}

void longest_reset( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* message )
{
  longest_clear( initid, args, is_null, message );
  longest_add( initid, args, is_null, message );
}


void longest_add( UDF_INIT* initid, UDF_ARGS* args, char* , char* )
{
  struct longest_data* data	= (struct longest_data*)initid->ptr;

  if (args->args[0] && (args->lengths[0]>data->length))
  {
    data->length=args->lengths[0];
    char *lpstrTotal = NULL;
    
    lpstrTotal = (char *)malloc(data->length+1);

    strncpy(lpstrTotal,args->args[0],data->length);
    lpstrTotal[data->length]='\0';

    if (data->result_string!=NULL)
    {
      free(data->result_string);
    }
    data->result_string=lpstrTotal;
  }

}

char *longest(UDF_INIT * initid, UDF_ARGS * , char * , unsigned long *length, char *is_null, char *)
{

  struct longest_data* data = (struct longest_data*)initid->ptr;
  if (data->length==0)
  {
    *is_null = 1;
    (*length) = 0;
    return NULL;
  }

  *is_null = 0;
  initid->max_length	= data->length;

  (*length) = data->length;

  return data->result_string;
}


#endif

