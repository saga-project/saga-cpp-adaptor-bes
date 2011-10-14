//bescluster.h
#ifndef _BESCLUSTER_H
#define _BESCLUSTER_H

#include "faults.h"
#include "job.h"
#include "rm.h"
#include <sys/utsname.h>
#include <time.h>

struct jobForward {
   struct soap *s;
   struct bes__CreateActivityType* req;
   struct bes__CreateActivityResponseType *resp;
   int returnValue;
}jobForward;

typedef struct OSListNode{
   char*OperatingSystem;
   char*OperatingSystemVersion;
   struct OSListNode* next;
}OSListNode;

typedef struct ArchListNode{
   char*CPUArchitecture;
   struct ArchListNode* next;
}ArchListNode;

typedef struct ResourceListNode{
   char* ResourceName;
   char* State;
   long int PhysicalMemory;
   int CPUCount;
   struct ResourceListNode* next;
}ResourceListNode;

typedef struct JobList{
   long jobid;
   struct JobList* next;
}JobList;


struct nodeResources{
  //OperatingSystem" type="jsdl:OperatingSystem_Type"
   char * OperatingSystemName;
   char * OperatingSystemVersion;
  //<xsd:element name="CPUArchitecture" type="jsdl:CPUArchitecture_Type"
   char* CPUArchitecture;
   unsigned int CPUCount;
   unsigned long CPUSpeed;
   unsigned long PhysicalMemory;
   unsigned long VirtualMemory;

   unsigned int NumberNodes;
   struct nodeResources* next;
};

//Common functions for all clusters
void getRequestInfo(struct soap*, char*, struct jobcard*);
void addRequestInfo(struct soap*, char*, struct jobcard*);
int loadResourceFile(struct soap *, struct rm_resource**, char*, int*);
void fillJobStatusDefaults(struct jobcard*);

char* trim(struct soap*,char*);
int monthToInt(char*);

#endif /* _BESCLUSTER_H */
