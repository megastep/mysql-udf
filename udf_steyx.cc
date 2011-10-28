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


extern "C" {

my_bool steyx_init( UDF_INIT* initid, UDF_ARGS* args, char* message );
void steyx_deinit( UDF_INIT* initid );
void steyx_clear(UDF_INIT *initid, char *is_null, char *is_error);
void steyx_reset( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );
void steyx_add( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );
double steyx( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );

}

#define BUFFERSIZE 1024	

struct steyx_data
{
    int fCount;
    double fSumX;
    double fSumY;
    double fSumDeltaXDeltaY; // sum of (ValX-MeanX)*(ValY-MeanY)
    double fSumSqrDeltaX; // sum of (ValX-MeanX)^2
    double fSumSqrDeltaY; // sum of (ValY-MeanY)^2
    
    // Copying the values
	double *valuesx;
	double *valuesy;
	int pages; // Number of buffers allocated
};

my_bool steyx_init( UDF_INIT* initid, UDF_ARGS* args, char* message )
{
	if (args->arg_count != 2)
  	{
    	strcpy(message,"wrong number of arguments: steyx() requires two arguments");
    	return 1;
  	}
  
	if ((args->arg_type[0]!=REAL_RESULT && args->arg_type[0]!=DECIMAL_RESULT) || 
		(args->arg_type[1]!=REAL_RESULT && args->arg_type[1]!=DECIMAL_RESULT))
    {
    	sprintf(message,"steyx() requires both real or decimal arguments: %d,%d",
    				(int)args->arg_type[0], (int)args->arg_type[1]);
    	return 1;
    }

  	steyx_data *buf = new steyx_data;
    buf->fCount           = 0.0;
    buf->fSumX            = 0.0;
    buf->fSumY            = 0.0;
    buf->fSumDeltaXDeltaY = 0.0; // sum of (ValX-MeanX)*(ValY-MeanY)
    buf->fSumSqrDeltaX    = 0.0; // sum of (ValX-MeanX)^2
    buf->fSumSqrDeltaY    = 0.0; // sum of (ValY-MeanY)^2
	buf->valuesx = buf->valuesy = NULL;
	buf->pages = 1;
	
  	initid->ptr = (char *)buf;
  	return 0;
}

void steyx_deinit( UDF_INIT* initid )
{
	steyx_data *buf = (steyx_data *)initid->ptr;
  	if (buf->valuesx != NULL)
  	{
    	free(buf->valuesx);
    	buf->valuesx=NULL;
  	}
	if (buf->valuesy != NULL)
  	{
    	free(buf->valuesy);
    	buf->valuesy=NULL;
  	}	
	delete buf;
}

void steyx_clear(UDF_INIT *initid, char *is_null, char *is_error)
{
	steyx_data *buf = (steyx_data *)initid->ptr;
    buf->fCount           = 0.0;
    buf->fSumX            = 0.0;
    buf->fSumY            = 0.0;
    buf->fSumDeltaXDeltaY = 0.0; // sum of (ValX-MeanX)*(ValY-MeanY)
    buf->fSumSqrDeltaX    = 0.0; // sum of (ValX-MeanX)^2
    buf->fSumSqrDeltaY    = 0.0; // sum of (ValY-MeanY)^2
  	if (buf->valuesx != NULL)
  	{
    	free(buf->valuesx);
  	}
	if (buf->valuesy != NULL)
  	{
    	free(buf->valuesy);
  	}
  	buf->valuesx=(double *) malloc(BUFFERSIZE*sizeof(double));
  	buf->valuesy=(double *) malloc(BUFFERSIZE*sizeof(double));
	buf->pages = 1;

	*is_null = *is_error = 0;	
}

void steyx_reset( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *is_error )
{
  steyx_clear(initid, is_null, is_error);
  steyx_add( initid, args, is_null, is_error );
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

void steyx_add( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error )
{
	if (args->args[0]!=NULL && args->args[1]!=NULL) {
		steyx_data *buf = (steyx_data *)initid->ptr;

		double fValX = parse_arg(args, 0), fValY = parse_arg(args, 1);

		if (buf->fCount >= BUFFERSIZE) {
			buf->pages ++;
			buf->valuesx = (double *)realloc(buf->valuesx, BUFFERSIZE*buf->pages*sizeof(double));
			buf->valuesy = (double *)realloc(buf->valuesy, BUFFERSIZE*buf->pages*sizeof(double));
		}

		buf->valuesx[buf->fCount] = fValX;
		buf->valuesy[buf->fCount] = fValY;
		buf->fSumX += fValX;
		buf->fSumY += fValY;
		buf->fCount ++;
	}
}

double steyx( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error )
{
	steyx_data *buf = (steyx_data *)initid->ptr;

    const double fMeanX = buf->fSumX / buf->fCount;
    const double fMeanY = buf->fSumY / buf->fCount;
    double fValX, fValY;

	for(int i = 0; i < buf->fCount; i++) {
		fValX = buf->valuesx[i];
		fValY = buf->valuesy[i];
		buf->fSumDeltaXDeltaY += (fValX - fMeanX) * (fValY - fMeanY);
		buf->fSumSqrDeltaX += (fValX - fMeanX) * (fValX - fMeanX);
		buf->fSumSqrDeltaY += (fValY - fMeanY) * (fValY - fMeanY);
	}
	
	if (buf->fSumSqrDeltaX==0.0) {
		*is_null = 1;
		return 0.0;
	}
	
	return sqrt((buf->fSumSqrDeltaY - buf->fSumDeltaXDeltaY*buf->fSumDeltaXDeltaY/buf->fSumSqrDeltaX) 
					/ (buf->fCount-2));
}

