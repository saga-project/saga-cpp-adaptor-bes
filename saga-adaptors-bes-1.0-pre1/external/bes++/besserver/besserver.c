/* ----------------------------------------------------------------
 * besserver.c
 *
 *      Server implementation of the OGSA Basic Execution Services
 *
 * Copyright (C) 2006-2009, Platform Computing Corporation. All Rights Reserved.
 *
 *
 * This file is part of BESserver.
 *
 * BESserver is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#include "../config.h"

#include <string.h>
#include <wchar.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <pwd.h>
#include <paths.h>
#include <time.h>

#include "wsseapi.h"

#include "BESFactoryBinding.nsmap"

#include "job.h"
#include "namespaces.h"
#include "faults.h"
#include "rm.h"


#define USERNAMELEN 256

#define clear_header(x) if (x->header) memset(x->header, 0, sizeof(struct SOAP_ENV__Header));


int isElement(struct soap_dom_element*, char*, char*);
int makeActivityEPR(struct soap*, char*, char*, struct wsa__EndpointReferenceType*);
int makeActivityDomEPR(struct soap*, char*, char*, struct soap_dom_element**);
int getJobIdFromEPR(struct soap*, struct wsa__EndpointReferenceType*, char**);
int processHeaders(struct soap*, char*, int);


char *service_endpoint = NULL;
char *generic_user = NULL;
uid_t service_uid;

struct Namespace default_namespaces[] = {
    {"SOAP-ENV", SOAP_ENV_NS, SOAP_ENV_NS_WILDCARD, NULL},
    {"wsse", WSSE_NS, NULL, NULL},
    {"wsu", WSU_NS, NULL, NULL},
    {"wsa", WSA_NS, NULL, NULL},
    {"bes", BES_NS, NULL, NULL},
    {"jsdl", JSDL_NS, NULL, NULL},
    {"app", JSDL_HPCPA_NS, NULL, NULL},
    {NULL, NULL, NULL, NULL}
};


void 
usage()
{
    fprintf(stderr, "Usage: besserver");
    fprintf(stderr, " -u serviceuser");
    fprintf(stderr, " [-h hostname] [-p port]");
    fprintf(stderr, " [-s server.pem] [-c capath]");
    fprintf(stderr, " [-e service_endpoint]");
    fprintf(stderr, " [-g generic user]");
    fprintf(stderr, " [-r resource manager]");
    fprintf(stderr, "\n");
    exit(1);
}
  
int 
main(int argc, char **argv) 
{
    struct soap soap, *psoap;
    int m, s, i, ch, port;
    char *host, *portstr, *capath, *certfile, *service_user, *resource_manager;
    struct passwd *pwent;
    time_t now;
  
    host = "localhost";
    portstr = "";
    port = 443;
    certfile = "server.pem";
    capath = "certs";
    service_user = NULL;
    resource_manager = NULL;

    while ((ch = getopt(argc, argv, "Vh:p:s:c:u:e:g:r:")) != -1) {
        switch (ch) {
        case 'h':
            host = optarg;
            break;
        case 'p':
            portstr = optarg;
            port = atoi(portstr);
            break;
        case 's':
            certfile = optarg;
            break;
        case 'c':
            capath = optarg;
            break;
        case 'u':
            service_user = optarg;
            break;
        case 'e':
            service_endpoint = optarg;
            break;
        case 'g':
            generic_user = optarg;
            break;
	case 'r':
	    resource_manager = optarg;
	    break;
      case 'V':
          printf("besserver, %s\n%s\n", VERSION_STRING, COPYRIGHT);
          exit(0);
          break;
        case '?':
        default:
            usage();
        }
    }
    
    if (service_user == NULL) {
        usage();
    }

    if (generic_user) {
        pwent = getpwnam(generic_user);
        if (pwent == NULL) {
            perror("getpwnam generic_user");
            exit(1);
        }
    }
    
    pwent = getpwnam(service_user);
    if (pwent == NULL) {
        perror("getpwnam");
        exit(1);
    }    
    
    service_uid = pwent->pw_uid;

    if (setgid(pwent->pw_gid) == -1) {
        perror("setgid service user");
        exit(1);
    }
    
    if (chdir(_PATH_TMP) == -1) {
        perror("chdir _PATH_TMP");
        exit(1);
    }

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("signal SIGPIPE");
        exit(1);
    }

    soap_ssl_init();
    soap_init(&soap);
    soap_set_mode(&soap, SOAP_C_UTFSTRING|SOAP_IO_STORE);
    soap_register_plugin(&soap, soap_wsse);

    if (soap_ssl_server_context(&soap, SOAP_SSL_DEFAULT, certfile, 
            NULL, NULL, capath, NULL, NULL, "besserver")) {
        soap_print_fault(&soap, stderr);
        exit(1);
    }

    if (service_endpoint == NULL) {
        service_endpoint = (char*)soap_malloc(&soap, strlen(host) 
                + strlen(portstr)+10);
        if (service_endpoint == NULL) {
            soap_print_fault(&soap, stderr);
            exit(1);
        }
        sprintf(service_endpoint, "https://%s%s%s", host, strlen(portstr)?":":"", portstr);
    }
  
    if (rm_initialize(&soap, resource_manager)) {
      fprintf(stderr, "Couldn't initialize the resource manager\n");
      exit(1);
    }

    m = soap_bind(&soap, host, port, 100);
    if (m < 0) {
        soap_print_fault(&soap, stderr);
    }
    else {
        int i;

        fprintf(stderr, "Socket connection successful: %d \n", m);

        if (seteuid(service_uid)) {
            perror("seteuid service_uid");
            exit(1);
        }
  
        for (i = 1;; i++) {
            soap_set_namespaces(&soap, namespaces);
            s = soap_accept(&soap);
            if (time(&now) > -1) {
                printf("%s", ctime(&now));
            }
            fprintf(stderr, "Accepted connection\n");
            if (s < 0) {
                soap_print_fault(&soap, stderr);
                break;
            }
            psoap = soap_copy(&soap);
            if (!psoap) {
                soap_print_fault(&soap, stderr);
                break;
            }
            if (soap_ssl_accept(psoap)) {
                soap_print_fault(psoap, stderr);
                soap_end(psoap);
                soap_done(psoap);
                free(psoap);
                continue;
            }
            if (soap_serve(psoap) != SOAP_OK) {
                soap_print_fault(&soap, stderr);
                fprintf(stderr, "soap error\n");
            }
            fprintf(stderr, "Request served\n");
            soap_end(psoap);
            soap_done(psoap);
            free(psoap);            
        }
    }
    soap_done(&soap);
}

int
__bes__CreateActivity(struct soap *s,
        struct bes__CreateActivityType req, 
        struct bes__CreateActivityResponseType *resp)
{
    static char fname[] = "__bes__CreateActivity";
    struct jobcard *jc;
    struct soap_dom_element *dom;
    int rc;
    struct SOAP_ENV__Fault *fault;
    char username[USERNAMELEN], *jobid;
  
    fprintf(stderr, "In %s...\n", fname);
    
    if ((rc = processHeaders(s, username, USERNAMELEN))) {
        return rc;
    }
    
    jc = (struct jobcard*)soap_malloc(s, sizeof(struct jobcard));
    if (jc == NULL) {
        return soap_receiver_fault(s, MEM_ALLOC, NULL);
    }
    memset(jc, 0, sizeof(struct jobcard));

    if (req.__any && isElement(req.__any, BES_NS, "ActivityDocument")) {
        if ((rc = processActivityDocument(s, req.__any, jc)) != SOAP_OK) {
            return rc;
        }
    } else {
        fault = bes_UnsupportedFault(s, ELEMENT_UNKNOWN, req.__any->name);
        if (!fault) {
            return soap_receiver_fault(s, MEM_ALLOC, NULL);
        }
        return bes_send_fault(s, fault);
    }
  
    rc = rm_submitJob(s, jc, username, &jobid);
    if (rc != BESE_OK) {
        if (rc == BESE_PERMISSION) {
            fault = bes_NotAuthorizedFault(s, ""/*FIXME: lsf dependent code lsb_sysmsg()*/);
            return bes_send_fault(s, fault);
        } else {
            return soap_receiver_fault(s, BACKEND_ERROR, NULL);
        }
    }

    rc = makeActivityDomEPR(s, service_endpoint, jobid, &dom);
    if (rc) {
        return rc;
    }

    resp->__size = 1;
    resp->__any = dom;

    return SOAP_OK;
}

int
__bes__TerminateActivities(struct soap *s,
        struct bes__TerminateActivitiesType *req,
        struct bes__TerminateActivitiesResponseType *resp)
{
    static char fname[] = "__bes__TerminateActivities";
    struct wsa__EndpointReferenceType *cur;
    struct bes__TerminateActivityResponseType *resp_array;
    struct soap_dom_element *fault;
    int i, rc;
    char username[USERNAMELEN], *jobid;

    fprintf(stderr, "In %s...\n", fname);
    
    if ((rc = processHeaders(s, username, USERNAMELEN))) {
        return rc;
    }

    if (req->__sizeActivityIdentifier == 0) {
        resp->__sizeResponse = 0;
        resp->Response = NULL;
        return SOAP_OK;
    }

    resp_array = (struct bes__TerminateActivityResponseType*)soap_malloc(s,
            sizeof(struct bes__TerminateActivityResponseType) 
            * req->__sizeActivityIdentifier);
    if (resp_array == NULL) {
        return soap_receiver_fault(s, MEM_ALLOC, NULL);
    }
    memset(resp_array, 0, sizeof(struct bes__TerminateActivityResponseType)
            * req->__sizeActivityIdentifier);
  
    for (i = 0; i < req->__sizeActivityIdentifier; i++) {
        cur = &(req->ActivityIdentifier[0]);
        resp_array[i].ActivityIdentifier = cur;

        jobid = NULL;
        if (getJobIdFromEPR(s, cur, &jobid) || !jobid) {
            resp_array[i].__any = bes_InvalidActivityFaultDOM(s, "Malformed EPR", "Malformed EPR");
            if (resp_array[i].__any == NULL) {
                return soap_receiver_fault(s, MEM_ALLOC, NULL);
            }
            resp_array[i].__size = 1;
            resp_array[i].Cancelled = false_;
            continue;
        }

        fprintf(stderr, "Terminating job %s\n", jobid);

        rc = rm_terminateJob(s, jobid, username);
        if (rc != BESE_OK) {
            if (rc == BESE_NO_ACTIVITY) {
                fault = bes_InvalidActivityFaultDOM(s, "Unknown Activity", "Unknown Activity");
            } else if (rc == BESE_PERMISSION) {
                fault = bes_NotAuthorizedFaultDOM(s, "Permission denied");
            } else {
                fault = bes_backend_errorDOM(s);
            }
            if (fault == NULL) {
                return soap_receiver_fault(s, MEM_ALLOC, NULL);
            }
            resp_array[i].__any = fault;
            resp_array[i].__size = 1;
            resp_array[i].Cancelled = false_;
        } else {
            resp_array[i].Cancelled = true_;
        }
    }

    resp->__sizeResponse = req->__sizeActivityIdentifier;
    resp->Response = resp_array;

    return SOAP_OK;
}

int
__bes__GetActivityStatuses(struct soap *s,
        struct bes__GetActivityStatusesType *req,
        struct bes__GetActivityStatusesResponseType *resp)
{
    static char fname[] = "__bes__GetActivityStatuses";
    struct wsa__EndpointReferenceType *cur;
    struct bes__GetActivityStatusResponseType *resp_array;
    struct bes__ActivityStatusType *status;
    struct soap_dom_element *fault;
    int i, rc;
    char username[USERNAMELEN], *jobid;

    fprintf(stderr, "In %s...\n", fname);
    
    if ((rc = processHeaders(s, username, USERNAMELEN))) {
        return rc;
    }

    if (req->__sizeActivityIdentifier == 0) {
        resp->__sizeResponse = 0;
        resp->Response = NULL;
        return SOAP_OK;
    }

    resp_array = (struct bes__GetActivityStatusResponseType*)soap_malloc(s,
            sizeof(struct bes__GetActivityStatusResponseType) 
            * req->__sizeActivityIdentifier);
    if (resp_array == NULL) {
        return soap_receiver_fault(s, MEM_ALLOC, NULL);
    }
    memset(resp_array, 0, sizeof(struct bes__GetActivityStatusResponseType)
            * req->__sizeActivityIdentifier);

    for (i = 0; i < req->__sizeActivityIdentifier; i++) {
        cur = &(req->ActivityIdentifier[0]);
        resp_array[i].ActivityIdentifier = cur;

        jobid = NULL;
        if (getJobIdFromEPR(s, cur, &jobid) || !jobid) {
            resp_array[i].__any = bes_InvalidActivityFaultDOM(s, "Malformed EPR", "Malformed EPR");
            if (resp_array[i].__any == NULL) {
                return soap_receiver_fault(s, MEM_ALLOC, NULL);
            }
            resp_array[i].__size = 1;
            continue;
        }

        fprintf(stderr, "Getting status for job %s\n", jobid);

        rc = rm_getJobStatus(s, jobid, username, &status);
        if (rc != BESE_OK) {
            fprintf (stderr, "status problem\n");
            if (rc == BESE_NO_ACTIVITY) {
                fault = bes_InvalidActivityFaultDOM(s, "Unknown Activity", "Unknown Activity");
            } 
            else {
                fault = bes_backend_errorDOM(s);
            }
            if (fault == NULL) {
                return soap_receiver_fault(s, MEM_ALLOC, NULL);
            }
            resp_array[i].__any = fault;
            resp_array[i].__size = 1;
            continue;
        }

        resp_array[i].bes__ActivityStatus = status;

    }

    resp->__sizeResponse = req->__sizeActivityIdentifier;
    resp->Response = resp_array;

    return SOAP_OK;
}

int
__bes__GetActivityDocuments(struct soap *s,
        struct bes__GetActivityDocumentsType *req,
        struct bes__GetActivityDocumentsResponseType *resp)
{
    static char fname[] = "__bes__GetActivityDocuments";
    struct wsa__EndpointReferenceType *cur;
    struct bes__GetActivityDocumentResponseType *resp_array;
    struct jobcard *job_info;
    struct soap_dom_element *fault;
    int i, rc;
    char username[USERNAMELEN], *jobid;

    fprintf(stderr, "In %s...\n", fname);

    if ((rc = processHeaders(s, username, USERNAMELEN))) {
        return rc;
    }

    if (req->__sizeActivityIdentifier == 0) {
        resp->__sizeResponse = 0;
        resp->Response = NULL;
        return SOAP_OK;
    }

    resp_array = (struct bes__GetActivityDocumentResponseType*)soap_malloc(s,
            sizeof(struct bes__GetActivityDocumentResponseType)
            * req->__sizeActivityIdentifier);
    if (resp_array == NULL) {
        return soap_receiver_fault(s, MEM_ALLOC, NULL);
    }
    memset(resp_array, 0, sizeof(struct bes__GetActivityDocumentResponseType)
            * req->__sizeActivityIdentifier);

    for (i = 0; i < req->__sizeActivityIdentifier; i++) {
        cur = &(req->ActivityIdentifier[0]);
        resp_array[i].ActivityIdentifier = cur;

        jobid = NULL;
        if (getJobIdFromEPR(s, cur, &jobid) || !jobid) {
            resp_array[i].__any = bes_InvalidActivityFaultDOM(s, "Malformed EPR", "Malformed EPR");
            if (resp_array[i].__any == NULL) {
                return soap_receiver_fault(s, MEM_ALLOC, NULL);
            }
            resp_array[i].__size = 1;
            continue;
        }

        fprintf(stderr, "Getting information for job %s\n", jobid);

        rc = rm_getJobInfo(s, jobid, username, &job_info);
        if (rc != BESE_OK) {
            if (rc == BESE_NO_ACTIVITY) {
                fault = bes_InvalidActivityFaultDOM(s, "Unknown Activity", "Unknown Activity");
            } 
            else {
                fault = bes_backend_errorDOM(s);
            }
            if (fault == NULL) {
                return soap_receiver_fault(s, MEM_ALLOC, NULL);
            }
            resp_array[i].__any = fault;
            resp_array[i].__size = 1;
            continue;
        }

        rc = getJSDLFromJobInfo(s, job_info, &resp_array[i].JobDefinition);
        if (rc) {
            return rc;
        }

    }

    resp->__sizeResponse = req->__sizeActivityIdentifier;
    resp->Response = resp_array;

    return SOAP_OK;
}

int
__bes__GetFactoryAttributesDocument(struct soap *s,
        struct bes__GetFactoryAttributesDocumentType *req,
        struct bes__GetFactoryAttributesDocumentResponseType *resp)
{
    static char fname[] = "__bes__GetFactoryAttributesDocument";
    struct bes__FactoryResourceAttributesDocumentType *attrs;
    struct wsa_EndpointReferenceType *epr;
    struct rm_clusterInfo *cinfo;
    struct rm_job *joblist, *job;
    struct rm_resource *resourcelist, *resource;
    struct soap_dom_element *contained_resources;
    struct bes__BasicResourceAttributesDocumentType *res;
    int num_jobs, num_resources, i, rc;
    char username[USERNAMELEN];

    fprintf(stderr, "In %s....\n", fname);

    if ((rc = processHeaders(s, username, USERNAMELEN))) {
        return rc;
    }

    attrs = (struct bes__FactoryResourceAttributesDocumentType*)soap_malloc(s,
            sizeof(struct bes__FactoryResourceAttributesDocumentType));
    if (attrs == NULL) {
        soap_print_fault(s, stderr);
        return soap_receiver_fault(s, MEM_ALLOC, NULL);
    }
    memset(attrs, 0, sizeof(struct bes__FactoryResourceAttributesDocumentType));


    if ((rc = rm_getClusterInfo(s, &cinfo)) != BESE_OK) {
        if (rc == BESE_MEM_ALLOC) {
            return soap_receiver_fault(s, MEM_ALLOC, NULL);
        }
        else if (rc == BESE_BACKEND) {
            return soap_receiver_fault(s, BACKEND_ERROR, NULL);
        }
        else {
            return soap_receiver_fault(s, UNKNOWN_ERROR, NULL);
        }
    }

    /* attributes from the backend */
    attrs->IsAcceptingNewActivities = cinfo->IsAcceptingNewActivities;
    attrs->CommonName = cinfo->CommonName;
    attrs->LongDescription = cinfo->LongDescription;
    attrs->__sizeBESExtension = cinfo->num_extensions;
    attrs->BESExtension = cinfo->BESExtensions;
    attrs->LocalResourceManagerType = cinfo->LocalResourceManagerType;

    /* Only support basic EPRs, no WS-Names */
    attrs->__sizeNamingProfile = 1;
    attrs->NamingProfile = (char**)soap_malloc(s, sizeof(char*));
    attrs->NamingProfile[0] = soap_strdup(s, 
            "http://schemas.ogf.org/bes/2006/08/bes/naming/BasicWSAddressing");
    if (attrs->NamingProfile == NULL) {
        soap_print_fault(s, stderr);
        return soap_receiver_fault(s, MEM_ALLOC, NULL);
    }

    /* TotalNumberOfActivities and ActivityReference */
    rc = rm_getJobList(s, NULL, &joblist, &num_jobs);
    if (rc != BESE_OK) {
        if (rc == BESE_MEM_ALLOC) {
            return soap_receiver_fault(s, MEM_ALLOC, NULL);
        }
        else if (rc == BESE_BACKEND) {
            return soap_receiver_fault(s, BACKEND_ERROR, NULL);
        }
        else {
            return soap_receiver_fault(s, UNKNOWN_ERROR, NULL);
        }
    }
    attrs->TotalNumberOfActivities = num_jobs;
    if (num_jobs) {
        attrs->__sizeActivityReference = num_jobs;
        attrs->ActivityReference = (struct wsa__EndpointReferenceType*)soap_malloc(s, sizeof(struct wsa__EndpointReferenceType)*num_jobs);
        if (attrs->ActivityReference == NULL) {
            soap_print_fault(s, stderr);
            return soap_receiver_fault(s, MEM_ALLOC, NULL);
        }
        job = joblist;
        for (i = 0; i < num_jobs; i++) {
            rc = makeActivityEPR(s, service_endpoint, job->jobid,
                                 &(attrs->ActivityReference[i]));
            if (rc) {
                soap_print_fault(s, stderr);
                return rc;
            }
            job = job->next;
        }
    }

    /* TotalNumberOfContainedResources and ContainedResource */
    rc = rm_getResourceList(s, NULL, &resourcelist, &num_resources);
    if (rc != BESE_OK) {
        if (rc == BESE_MEM_ALLOC) {
            return soap_receiver_fault(s, MEM_ALLOC, NULL);
        }
        else if (rc == BESE_BACKEND) {
            return soap_receiver_fault(s, BACKEND_ERROR, NULL);
        }
        else {
            return soap_receiver_fault(s, UNKNOWN_ERROR, NULL);
        }
    }
    attrs->TotalNumberOfContainedResources = num_resources;
    if (num_resources) {
        contained_resources = (struct soap_dom_element*)soap_malloc(s,
                sizeof(struct soap_dom_element)*num_resources);
        if (!contained_resources) {
            return soap_receiver_fault(s, MEM_ALLOC, NULL);
        }
        memset(contained_resources, 0, sizeof(struct soap_dom_element)
                *num_resources);
        resource = resourcelist;
        for (i = 0; i < num_resources; i++) {
            res = (struct bes__BasicResourceAttributesDocumentType*)soap_malloc(s, sizeof(struct bes__BasicResourceAttributesDocumentType));
            if (!res) {
                return soap_receiver_fault(s, MEM_ALLOC, NULL);
            }
            memset(res, 0, sizeof(struct bes__BasicResourceAttributesDocumentType));

            res->ResourceName = resource->ResourceName;
            res->CPUCount = resource->CPUCount;
            res->CPUSpeed = resource->CPUSpeed;
            res->PhysicalMemory = resource->PhysicalMemory;
            res->VirtualMemory = resource->VirtualMemory;

            contained_resources[i].type
                    = SOAP_TYPE_bes__BasicResourceAttributesDocumentType;
            contained_resources[i].node = res;
            contained_resources[i].soap = s;
            resource = resource->next;
        }      
        attrs->ContainedResource = contained_resources;
        attrs->__sizeContainedResource = num_resources;
    }

    resp->bes__FactoryResourceAttributesDocument = attrs;

    return SOAP_OK;
}

int
isElement(struct soap_dom_element *dom, char *ns, char *elt)
{
    static char fname[] = "isElement";
    char *cp;

    if (!dom || !ns || !elt) {
        return 0;
    }
    if (strcmp(dom->nstr, ns)) {
        return 0;
    }
    cp = strchr(dom->name, ':');
    if (cp) {
        cp++;
    } else {
        cp = dom->name;
    }
    if (strcmp(cp, elt)) {
        return 0;
    }

    return 1;
}

int
makeActivityEPR(struct soap *s, char *endpoint, char *jobid,
        struct wsa__EndpointReferenceType *epr)
{
    static char fname[] = "makeActivityEPR";

    memset(epr, 0, sizeof(wsa__EndpointReferenceType));
    epr->Address = (wsa__AttributedURIType*)soap_malloc(s,
            sizeof(wsa__AttributedURIType));
    if (epr->Address == NULL) {
        return soap_receiver_fault(s, MEM_ALLOC, NULL);
    }
    memset(epr->Address, 0, sizeof(wsa__AttributedURIType));
    epr->Address->__item = (char*)soap_malloc(s, strlen(endpoint)+strlen(jobid)+2);
    if (epr->Address->__item == NULL) {
        return soap_receiver_fault(s, MEM_ALLOC, NULL);
    }
    sprintf(epr->Address->__item, "%s/%s", endpoint, jobid);

    return SOAP_OK;
}

int
makeActivityDomEPR(struct soap *s, char *endpoint, char *jobid,
        struct soap_dom_element **ret)
{
    static char fname[] = "makeActivityDomEPR";
    struct soap_dom_element *activityid, *addr;

    activityid = (struct soap_dom_element*)soap_malloc(s,
            sizeof(struct soap_dom_element));
    addr = (struct soap_dom_element*)soap_malloc(s,
            sizeof(struct soap_dom_element));
    if (!activityid || !addr) {
        return soap_receiver_fault(s, MEM_ALLOC, NULL);
    }
    memset(activityid, 0, sizeof(struct soap_dom_element));
    memset(addr, 0, sizeof(struct soap_dom_element));

    activityid->name = soap_strdup(s, "ActivityIdentifier");
    activityid->nstr = soap_strdup(s, BES_NS);
    if (!activityid->name || !activityid->nstr) {
        return soap_receiver_fault(s, MEM_ALLOC, NULL);
    }
    activityid->elts = addr;
    activityid->soap = s;

    addr->name = soap_strdup(s, "Address");
    addr->nstr = soap_strdup(s, WSA_NS);
    addr->data = (char*)soap_malloc(s, strlen(endpoint) + strlen(jobid) + 2);
    if (!addr->name || !addr->nstr || !addr->data) {
        return soap_receiver_fault(s, MEM_ALLOC, NULL);
    }
    sprintf(addr->data, "%s/%s", endpoint, jobid);
    addr->soap = s;
    addr->prnt = activityid;

    *ret = activityid;
    return SOAP_OK;
}

int
getJobIdFromEPR(struct soap *s, struct wsa__EndpointReferenceType *epr,
        char **jobid)
{
    static char fname[] = "getJobIdFromEPR";
    char *cp;

    if (!jobid) {
        fprintf(stderr, "%s: passed NULL jobid\n", fname);
        return -1;
    }

    cp = strrchr(epr->Address->__item, '/');
    if (!cp) {
        return -1;
    }
    cp++;
    *jobid = soap_strdup(s, cp);

    return 0;
}

int
processHeaders(struct soap *s, char *username, int usernamelen)
{
    int rc = SOAP_OK;
    int sslauth = 0;

    rc = authenticate(s, username, usernamelen, &sslauth);
    clear_header(s);
    if (!sslauth) {
        soap_wsse_add_Timestamp(s, NULL, 30);
    }

    return rc;
}

/*
 * Don't know why these aren't being generated, but ....
 */
struct soap_dom_element *
soap_in__bes__CreateActivityFaultMessage(
        struct soap *s, const char *tag, struct soap_dom_element *node,
        const char *type)
{
    return soap_in_xsd__anyType(s, tag, node, type);
}

int
soap_out__bes__CreateActivityFaultMessage(struct soap *s, const char *tag,
        int id, const struct soap_dom_element *node, const char *type)
{
    return soap_out_xsd__anyType(s, tag, id, node, type);
}

void
soap_serialize__bes__CreateActivityFaultMessage(struct soap *s,
        const struct soap_dom_element *node)
{
    soap_serialize_xsd__anyType(s, node);
}

