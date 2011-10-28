/*
  returns the length of the longest value in a set

  input parameters:
  data (string)

  output:
  length of longest value (int)

  registering the function:
  CREATE AGGREGATE FUNCTION colwidth RETURNS INTEGER SONAME 'udf_colwidth.so';

  getting rid of the function:
  DROP FUNCTION colwidth;

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

my_bool colwidth_init( UDF_INIT* initid, UDF_ARGS* args, char* message );
void colwidth_deinit( UDF_INIT* initid );
void colwidth_reset( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );
void colwidth_add( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );
long long colwidth( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* error );


struct colwidth_data
{
  long long maxlength;
};


my_bool colwidth_init( UDF_INIT* initid, UDF_ARGS* args, char* message )
{
  struct colwidth_data*	data = NULL;

  if (args->arg_count != 1)
  {
    strcpy(message, "wrong number of arguments: colwidth() requires exactly one argument");
    return 1;
  }

  if ( args->arg_type[0] != STRING_RESULT )
  {
    strcpy(message, "wrong argument type: colwidth() requires a string as parameter 1");
    return 1;
  }
  
  data = new struct colwidth_data;
  data->maxlength=0;
  
  initid->maybe_null	= 0;
  initid->ptr = (char*)data;

  return 0;
}



void colwidth_deinit( UDF_INIT* initid )
{
  struct colwidth_data* data = (struct colwidth_data*)initid->ptr;
  if (data != NULL)
  {
    data->maxlength=0;
    delete data;
    initid->ptr = NULL;
  }
}


void colwidth_reset( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* message )
{
  struct colwidth_data* data = (struct colwidth_data*)initid->ptr;
  
  data->maxlength=(long long) 0;
  colwidth_add( initid, args, is_null, message );
}


void colwidth_add( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* message )
{
  struct colwidth_data* data	= (struct colwidth_data*)initid->ptr;
  
  if (args->args[0] && (args->lengths[0]>data->maxlength)) 
  {
    data->maxlength=args->lengths[0];
  }
}


long long colwidth( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* error )
{
  struct colwidth_data* data = (struct colwidth_data*)initid->ptr;
 
  *is_null=0; 
  if (data->maxlength)
  {
    return data->maxlength;
  }
  return 0;
}


}
#endif 

