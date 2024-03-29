TOP = ..
include $(TOP)/Make.config

GSOAP_PLUGIN = $(GSOAP_TOP)/plugin
GSOAP_IMPORT = $(GSOAP_TOP)/import
GSOAP_INC = -I$(GSOAP_TOP) -I$(GSOAP_PLUGIN)
GSOAP_DEF = -DDEBUG -DWITH_DOM -DWITH_OPENSSL
SOAPCPP2 = $(GSOAP_TOP)/src/soapcpp2 -LSxc -I$(GSOAP_IMPORT)
WSDL2H = $(GSOAP_TOP)/wsdl/wsdl2h -cdegy -I$(GSOAP_IMPORT)
SCHEMAS = 	http://schemas.ggf.org/jsdl/2006/07/jsdl-hpcpa \
#		http://schemas.ggf.org/jsdl/2005/11/jsdl-posix

CC = gcc
CFLAGS = -g $(GSOAP_DEF) -I. $(GSOAP_INC) $(RM_INC) $(EXTRA_INC) $(EXTRA_CFLAGS)

CXX = g++
CPPFLAGS = $(CFLAGS)

LDFLAGS = -g $(EXTRA_LIB) $(EXTRA_LDFLAGS)
LIBS = -lssl -lcrypto -lpam $(RM_LIBS) $(EXTRA_LIBS)

BES_WSDL = bes-factory.wsdl
BES_HEADER = bes-factory.h
BES_H_FILES = job.h namespaces.h faults.h auth.h rm.h
ifneq ($(RM),rm_lsf)
   BES_OBJ = besserver.o job.o faults.o auth.o $(RM).o rm_util.o
else 
 ifneq ($(RM),rm_saga)
   BES_OBJ = besserver.o job.o faults.o auth.o $(RM).o rm_util.o rm_saga.o
 else 
  ifneq ($(RM),rm_lava)
   BES_OBJ = besserver.o job.o faults.o auth.o $(RM).o rm_util.o
  else
   BES_OBJ = besserver.o job.o faults.o auth.o $(RM).o
  endif
 endif
endif

SOAP_NS   = BESFactoryBinding.nsmap
SOAP_H_FILES = soapH.h soapStub.h
SOAP_C_FILES = soapC.c soapClient.c soapClientLib.c soapServer.c soapServerLib.c

GSOAP_OBJ = stdsoap2.o dom.o smdevp.o wsseapi.o
SERVER_OBJ = soapServer.o soapC.o $(GSOAP_OBJ)
CLIENT_OBJ = soapClient.o soapC.o $(GSOAP_OBJ)

all: besserver

besserver: $(BES_OBJ) $(SERVER_OBJ)
	$(RM_LD) $(LDFLAGS) -o besserver $(BES_OBJ) $(SERVER_OBJ) $(LSFLIBS) $(LIBS)

besserver.o: besserver.c $(BES_H_FILES) $(SOAP_NS)
job.o: job.c $(BES_H_FILES) $(SOAP_NS)
faults.o: faults.c $(BES_H_FILES) $(SOAP_NS)
auth.o: auth.c $(BES_H_FILES) $(SOAP_NS)

ifeq ($(RM),rm_saga)
$(RM).o: $(RM).cpp rm_util.o $(BES_H_FILES) $(SOAP_NS)
else
$(RM).o: $(RM).c rm_util.o $(BES_H_FILES) $(SOAP_NS)
endif


soapServer.o: soapServer.c
soapClient.o: soapClient.c
soapC.o: soapC.c
ifneq ($(RM),rm_lsf)
   rm_util.o: rm_util.c
endif
ifneq ($(RM),rm_lava)
   rm_util.o: rm_util.c
endif

stdsoap2.o: $(GSOAP_TOP)/stdsoap2.c
	$(CC) $(CFLAGS) -c $(GSOAP_TOP)/stdsoap2.c

dom.o: $(GSOAP_TOP)/dom.c
	$(CC) $(CFLAGS) -c $(GSOAP_TOP)/dom.c

smdevp.o: $(GSOAP_PLUGIN)/smdevp.c $(GSOAP_PLUGIN)/smdevp.h
	$(CC) $(CFLAGS) -c $(GSOAP_PLUGIN)/smdevp.c

wsseapi.o: $(GSOAP_PLUGIN)/wsseapi.c $(GSOAP_PLUGIN)/wsseapi.h
	$(CC) $(CFLAGS) -c $(GSOAP_PLUGIN)/wsseapi.c

$(SOAP_NS): $(BES_HEADER)
	$(SOAPCPP2) $(BES_HEADER)

$(BES_HEADER): $(BES_WSDL) besserver-typemap.dat
	$(WSDL2H) -tbesserver-typemap.dat -o $(BES_HEADER) $(BES_WSDL) $(SCHEMAS)

$(BES_WSDL):
	ln -fs ../*.xsd .
	ln -fs ../*.wsdl .
	
clean:
	rm -f $(SOAP_H_FILES)
	rm -f $(SOAP_C_FILES)
	rm -f *.nsmap
	rm -f *.res.xml *.req.xml
	rm -f besserver *.o *~
	rm -f $(BES_HEADER)
	rm -f *.log
	rm -f *.wsdl *.xsd



