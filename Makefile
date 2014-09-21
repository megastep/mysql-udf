# Where to copy the final library
#DEST	= ./plugin

# Where are mysql libraries
INCL	= -I/usr/src/mariadb-5.5.30/include/ -I/usr/include/mysql

TARGET = udf_math.so
CFLAGS = -O2 -fPIC $(INCL) -DHAVE_DLOPEN=1


SRCS = 	udf_colwidth.cc \
	udf_confidence_higher.cc \
	udf_confidence_lower.cc \
	udf_correlation.cc \
	udf_faculty.cc \
	udf_geomean.cc \
	udf_intercept.cc \
	udf_kurtosis.cc \
	udf_longest.cc \
	udf_median.cc \
	udf_noverm.cc \
	udf_skewness.cc \
	udf_slope.cc \
	udf_stdnorm_density.cc \
	udf_stdnorm_dist.cc \
	udf_steyx.cc \
	udf_weightedavg.cc
OBJS =  $(SRCS:%.cc=%.o)

all: $(TARGET)

install: $(TARGET)
	@echo
	@echo	"To install the udf functions,"
	@echo	"copy $(TARGET) to mysql/plugins directory,"
	@echo	"then run the SQL file import.sql in MySQL"
	@echo
#	cp $(TARGET) $(DEST)

clean:
	$(RM) $(OBJS) $(TARGET)

%.o: %.cc
	$(CXX) -o $@ $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	$(LD) -shared -o $(TARGET) $(OBJS)

