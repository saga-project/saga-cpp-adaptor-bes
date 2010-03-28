TOP = ..
include $(TOP)/Make.config

GSOAP_PLUGIN = $(GSOAP_TOP)/plugin
GSOAP_IMPORT = $(GSOAP_TOP)/import
GSOAP_INC = -I$(GSOAP_TOP) -I$(GSOAP_PLUGIN)
GSOAP_DEF = -DDEBUG -DWITH_DOM -DWITH_OPENSSL
SOAPCPP2 = $(GSOAP_TOP)/src/soapcpp2 -LCxc -I$(GSOAP_IMPORT)
WSDL2H = $(GSOAP_TOP)/wsdl/wsdl2h -cdegy -I$(GSOAP_IMPORT)
SCHEMAS =	http://schemas.ggf.org/jsdl/2006/07/jsdl-hpcpa \
		http://schemas.ggf.org/jsdl/2005/11/jsdl-posix

CC      = gcc
CFLAGS  = -g $(GSOAP_DEF) -I. $(GSOAP_INC) $(EXTRA_INC) $(EXTRA_CFLAGS)
LDFLAGS = -g -L. $(EXTRA_LIB) $(EXTRA_LDFLAGS)
LIBS    = -lssl -lcrypto $(EXTRA_LIBS)
LIBBES  = -lbes

BES_WSDL = bes-factory.wsdl
BES_HEADER = bes-factory.h

SOAP_NS   = BESFactoryBinding.nsmap
SOAP_H_FILES = soapH.h soapStub.h
SOAP_C_FILES = soapC.c soapClient.c soapClientLib.c soapServer.c soapServerLib.c

GSOAP_OBJ = stdsoap2_ssl.o dom.o smdevp.o wsseapi.o
SERVER_OBJ = soapServer.o soapC.o $(GSOAP_OBJ)
CLIENT_OBJ = soapClient.o soapC.o $(GSOAP_OBJ)

OSNAME     = $(shell uname -s)

ifeq "$(OSNAME)" "Darwin"
 SO        = dylib
 LIBLDFLAG = -dynamiclib
else
 SO        =  so
 LIBLDFLAG = -shared
endif

all: libbes.$(SO) libbes.a besclient libtests

besclient: besclient.o libbes.a
	$(CC) $(LDFLAGS) -o besclient besclient.o $(LIBBES) $(LIBS)

libtests: libtests.o libbes.a
	$(CC) $(LDFLAGS) -o libtests libtests.o $(LIBBES) $(LIBS)

libbes.a: bes.o jsdl.o $(CLIENT_OBJ)
	ar -rsv $@  bes.o jsdl.o $(CLIENT_OBJ)

libbes.$(SO): bes.o jsdl.o $(CLIENT_OBJ)
	$(CC) -dynamiclib -o $@ bes.o jsdl.o $(CLIENT_OBJ) $(LIBS)

stdsoap2_ssl.o: $(GSOAP_TOP)/stdsoap2_ssl.c
	$(CC) $(CFLAGS) -c $(GSOAP_TOP)/stdsoap2_ssl.c

dom.o: $(GSOAP_TOP)/dom.c
	$(CC) $(CFLAGS) -c $(GSOAP_TOP)/dom.c

smdevp.o: $(GSOAP_PLUGIN)/smdevp.c
	$(CC) $(CFLAGS) -c $(GSOAP_PLUGIN)/smdevp.c

wsseapi.o: $(GSOAP_PLUGIN)/wsseapi.c
	$(CC) $(CFLAGS) -c $(GSOAP_PLUGIN)/wsseapi.c

$(SOAP_NS): $(BES_HEADER)
	$(SOAPCPP2) $(BES_HEADER)

$(BES_HEADER): $(BES_WSDL)
	$(WSDL2H) -tbesclient-typemap.dat -o $(BES_HEADER) $(BES_WSDL) $(SCHEMAS)

$(BES_WSDL):
	ln -fs ../*.xsd .
	ln -fs ../*.wsdl . 
	
clean:
	rm -f $(SOAP_H_FILES)
	rm -f $(SOAP_C_FILES)
	rm -f *.nsmap
	rm -f *.res.xml *.req.xml
	rm -f besclient libtests libbes.a *.o *~
	rm -f $(BES_HEADER)
	rm -f *.log
	rm -f *.wsdl *.xsd










# dependencies
$(GSOAP_PLUGIN)/smdevp.c:  $(GSOAP_PLUGIN)/smdevp.h
$(GSOAP_PLUGIN)/wsseapi.c: $(GSOAP_PLUGIN)/wsseapi.h

$(BES_WSDL):  besclient-typemap.dat
besclient.o:  besclient.c bes.h
libtests.o:   libtests.c  bes.h
bes.o:        bes.c       bes.h jsdl.h $(SOAP_NS)
jsdl.o:       jsdl.c            jsdl.h $(SOAP_NS)
soapServer.o: soapServer.c
soapClient.o: soapClient.c
soapC.o:      soapC.c

