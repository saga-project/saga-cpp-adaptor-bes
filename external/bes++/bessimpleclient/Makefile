
GSOAP_TOP    = ../../gsoap-2.7/gsoap
GSOAP_DEF    = -DDEBUG -DWITH_DOM -DWITH_OPENSSL
SOAPCPP2     = $(GSOAP_TOP)/src/soapcpp2 -LCxc 
WSDL2H       = $(GSOAP_TOP)/wsdl/wsdl2h -cdegy 
SCHEMAS      =	http://schemas.ggf.org/jsdl/2006/07/jsdl-hpcpa \
		            http://schemas.ggf.org/jsdl/2005/11/jsdl-posix

CXX          = g++
CXXFLAGS     = -g $(GSOAP_DEF) -I. $(EXTRA_INC) $(EXTRA_CFLAGS)

LDFLAGS      = -g -L. $(EXTRA_LIB) $(EXTRA_LDFLAGS)
LIBS         = -lssl -lcrypto $(EXTRA_LIBS)
LIBBES       = -lbes

BES_WSDL     = bes-factory.wsdl
BES_HEADER   = bes-factory.h

SOAP_NS      = BESFactoryBinding.nsmap
SOAP_H_FILES = soapH.h soapStub.h
SOAP_C_FILES = soapC.c soapClient.c soapClientLib.c soapServer.c soapServerLib.c

GSOAP_OBJ    = stdsoap2_ssl.o dom.o smdevp.o wsseapi.o
SERVER_OBJ   = soapServer.o soapC.o $(GSOAP_OBJ)
CLIENT_OBJ   = soapClient.o soapC.o $(GSOAP_OBJ)

OSNAME       = $(shell uname -s)

ifeq "$(OSNAME)" "Darwin"
 SO          = dylib
 LIBLDFLAG   = -dynamiclib
else
 SO          = so
 LIBLDFLAG   = -shared
endif

all: libbes.$(SO) libbes.a bes_sagatest 

%.o: %.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

bes_sagatest: bes_sagatest.o libbes.a
	$(CXX) $(LDFLAGS) -o bes_sagatest bes_sagatest.o $(LIBBES) $(LIBS)

libbes.a: bes.o jsdl.o $(CLIENT_OBJ)
	ar -rsv $@  bes.o jsdl.o $(CLIENT_OBJ)

libbes.$(SO): bes.o jsdl.o $(CLIENT_OBJ)
	$(CXX) -dynamiclib -o $@ bes.o jsdl.o $(CLIENT_OBJ) $(LIBS)

$(SOAP_NS): $(BES_HEADER)
	$(SOAPCPP2) $<

$(BES_HEADER): $(BES_WSDL)
	$(WSDL2H) -tbes_sagatest-typemap.dat -o $@ $< $(SCHEMAS)

$(BES_WSDL):
	ln -fs ../*.xsd .
	ln -fs ../*.wsdl . 
	
clean:
	rm -f $(SOAP_H_FILES)
	rm -f $(SOAP_C_FILES)
	rm -f *.nsmap
	rm -f *.res.xml *.req.xml
	rm -f bes_sagatest libbes.a libbes.$(SO) *.o *~ 
	rm -f $(BES_HEADER)
	rm -f *.log
	rm -f *.wsdl *.xsd
	-cp save/BESFactoryBinding.nsmap .
	-cp save/bes-factory.h           .
	-cp save/bes-factory.wsdl        .
	-cp save/bes-factory.xsd         .
	-cp save/soapC.c                 .
	-cp save/soapClient.c            .
	-cp save/soapH.h                 .
	-cp save/soapStub.h              .


# dependencies
bes-factory.wsdl: bes_sagatest-typemap.dat
bes_sagatest.o:   bes_sagatest.cpp  bes.hpp bes_connector.hpp
bes.o:            bes.cpp           bes.hpp jsdl.hpp $(SOAP_NS)
jsdl.o:           jsdl.cpp                  jsdl.hpp $(SOAP_NS)
soapServer.o:     soapServer.c
soapClient.o:     soapClient.c
soapC.o:          soapC.c
smdevp.c:         smdevp.h
wsseapi.c:        wsseapi.h threads.h

