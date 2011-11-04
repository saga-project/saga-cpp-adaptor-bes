/* ----------------------------------------------------------------
 * bes.c
 *   
 *      Client library of the OGSA Basic Execution Services
 *
 * Copyright (C) 2006-2009, Platform Computing Corporation. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
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
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#include "bes.hpp"
#include "jsdl.hpp"
#include "namespaces.h"
#include "wsseapi.h"
#include "BESFactoryBinding.nsmap"

#include <iostream>
#include <sstream>
#include <string>

#define CREATE_ACT     "http://schemas.ggf.org/bes/2006/08/bes-factory/BESFactoryPortType/CreateActivity"
#define STATUS_ACT     "http://schemas.ggf.org/bes/2006/08/bes-factory/BESFactoryPortType/GetActivityStatuses"
#define TERMINATE_ACT  "http://schemas.ggf.org/bes/2006/08/bes-factory/BESFactoryPortType/TerminateActivities"
#define ACTIVITIES_ACT "http://schemas.ggf.org/bes/2006/08/bes-factory/BESFactoryPortType/GetActivityDocuments"
#define FACTORY_ACT    "http://schemas.ggf.org/bes/2006/08/bes-factory/BESFactoryPortType/GetFactoryAttributesDocument"

#define NL "\n"

struct Namespace default_namespaces[] =
{
    {"SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/", "http://www.w3.org/*/soap-envelope", NULL},
    {"wsu",  WSU_NS,        NULL, NULL},
    {"wsse", WSSE_NS,       NULL, NULL},
    {"wsa",  WSA_NS,        NULL, NULL},
    {"bes",  BES_NS,        NULL, NULL},
    {"jsdl", JSDL_NS,       NULL, NULL},
    {"app",  JSDL_HPCPA_NS, NULL, NULL},
    {NULL,   NULL, NULL, NULL}
};

struct Namespace epr_namespaces[] = {
    {"wsa", WSA_NS, NULL, NULL },
    {"bes", BES_NS, NULL, NULL },
    {"xsi", "http://www.w3.org/2001/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance", NULL},
    {NULL, NULL, NULL, NULL}
};

// struct bes_epr {
//     char *str;
//     struct soap_dom_element *dom;
//     int domCreateFlag;
// };

#define FromMalloc  1

int isElement(struct soap_dom_element *, char *, char *);
int isAttribute(struct soap_dom_attribute *, const char *, const char *);
int generateAddressingHeaders(struct bes_context *, epr_t, const char *, char **);
void cleanDom(struct soap_dom_element*);
void setErrorString(struct bes_context *, struct soap *, int);
char *generateEPRString(struct soap_dom_element *, char *);
int calcDomSize(struct soap_dom_element *, char *);
void sprintDom(struct soap_dom_element *, char *, char *);
int getActivityDocumentsDOM(struct bes_context *, epr_t, epr_t, struct soap_dom_element **); 
int getActivityDocumentFromDOM(struct bes_context *, struct soap_dom_element *, struct bes_activity_document **); 


int CRYPTO_thread_setup(void);
int CRYPTO_thread_cleanup(void);

void sigpipe_handle(int x) 
{ 
	
}
 
void __attribute__ ((constructor)) bes_init_ssl(void) 
{
    soap_ssl_init();
    CRYPTO_thread_setup();    

    signal(SIGPIPE, sigpipe_handle);
}

void __attribute__ ((destructor)) bes_cleanup_ssl(void) 
{
    CRYPTO_thread_cleanup();
    signal(SIGPIPE, SIG_DFL);
}


int
bes_init(struct bes_context **context)
{
    struct bes_context *ctx;
    struct soap *soap;
    
    if (context == NULL) {
        return BESE_BAD_ARG;
    }
    
    ctx = (struct bes_context *)malloc(sizeof(struct bes_context));
    if (ctx == NULL) {
        fprintf (stderr, "err 1\n");
        return BESE_SYS_ERR;
    }
    memset(ctx, 0, sizeof(struct bes_context));
    
    soap_ssl_init();
    soap = soap_new1(SOAP_C_UTFSTRING);
    if (soap == NULL) {
        free(ctx);
        fprintf (stderr, "err 2\n");
        return BESE_SYS_ERR;
    }
// #ifdef DEBUG
//     soap_set_recv_logfile(soap, NULL);
//     soap_set_sent_logfile(soap, NULL);
//     soap_set_test_logfile(soap, NULL);
// #endif

    soap_register_plugin(soap, soap_wsse);
    soap_set_namespaces(soap, default_namespaces);
    soap_header(soap);

    ctx->soap = soap;
    *context = ctx;

    return BESE_OK;
}

int
bes_security(struct bes_context *context, 
             char *x509cert, 
             char *x509pass, 
             char *capath, 
             char *user, 
             char *pass)
{
    struct soap *soap;
    
    if (context == NULL) {
        return BESE_BAD_ARG;
    }
    
    soap = context->soap;
    
    if (soap_ssl_client_context(soap, SOAP_SSL_DEFAULT|SOAP_SSL_SKIP_HOST_CHECK,
                                x509cert, x509pass, NULL, capath, NULL)) {
        setErrorString(context, context->soap, BESE_SOAP_ERR);
        return BESE_SOAP_ERR;
    }
    
    if (user) {
        if (soap_wsse_add_UsernameTokenText(soap, NULL, user, pass)) {
            setErrorString(context, context->soap, BESE_SOAP_ERR);
            return BESE_SOAP_ERR;
        }
        // if (soap_wsse_add_Timestamp(soap, NULL, 60)) {
        //     setErrorString(context, context->soap, BESE_SOAP_ERR);
        //     return BESE_SOAP_ERR;
        // }
    }
    
    if (x509cert) {
        if (context->x509cert) free(context->x509cert);
        context->x509cert = strdup(x509cert);
    }
    
    if (x509pass) {
        if (context->x509pass) free(context->x509pass);
        context->x509pass = strdup(x509pass);
    }
    
    if (capath) {
        if (context->capath) free(context->capath);
        context->capath = strdup(capath);
    }

    if (user) {
        if (context->user) free(context->user);
        context->user = strdup(user);
    }
    
    if (pass) {
        if (context->pass) free(context->pass);
        context->pass = strdup(pass);
    }
    
    return BESE_OK;
}

int
bes_add_usertoken(struct bes_context *context, 
                  char *user, 
                  char *pass)
{
    if (context == NULL) {
        return BESE_BAD_ARG;
    }
  
    if (soap_wsse_add_UsernameTokenText(context->soap, NULL, user, pass)) {
        setErrorString(context, context->soap, BESE_SOAP_ERR);
        return BESE_SOAP_ERR;
    }
    if (soap_wsse_add_Timestamp(context->soap, NULL, 60)) {
        setErrorString(context, context->soap, BESE_SOAP_ERR);
        return BESE_SOAP_ERR;
    }
    
    return BESE_OK;
}

int
bes_finalize(struct bes_context **context)
{
    struct bes_context *ctx;
    
    if (context == NULL || *context == NULL) {
        return BESE_BAD_ARG;
    }
    
    ctx = *context;
    
    soap_end(ctx->soap);
    soap_done(ctx->soap);
    
    free(ctx->soap);
    
    if (ctx->x509cert) {
        free(ctx->x509cert);
    }
    
    if (ctx->x509pass) {
        free(ctx->x509pass);
    }
    
    if (ctx->capath) {
        free(ctx->capath);
    }
    
    if (ctx->user) {
        free(ctx->user);
    }
    
    free(ctx);
    *context = NULL;

    return BESE_OK;
}

const char* 
bes_get_lasterror(struct bes_context *context)
{
    if (context) {
        return context->error_string;
    }
    return "No valid BES Context.";
}

int
bes_createActivity (struct bes_context         * context, 
                    epr_t                        endpointepr, 
                    struct jsdl_job_definition * jsdl, 
                    epr_t                      * activityepr) 
{
  struct soap *s;
  struct bes__CreateActivityType req;
  struct bes__CreateActivityResponseType rsp;
  struct soap_dom_element *jsdl_dom, *tmpdom;
  struct bes_epr *epr;
  int    ret = BESE_OK;
  char  *endpoint;

  if ( context     == NULL ||
       endpointepr == NULL ||
       jsdl        == NULL ||
       activityepr == NULL ) 
  {
    return BESE_BAD_ARG;
  }

  s = context->soap;

  if ( (ret = generateAddressingHeaders (context, endpointepr, 
                                        CREATE_ACT, &endpoint)) )
  {
    setErrorString (context, NULL, ret);
    return ret;
  }

  if ( (ret = jsdl_generateJobDefinitionDOM (jsdl, &jsdl_dom)) )
  {
    setErrorString (context, NULL, ret);
    return ret;
  }

  memset (&req, 0, sizeof (struct bes__CreateActivityType));
  memset (&rsp, 0, sizeof (struct bes__CreateActivityResponseType));

  req.bes__ActivityDocument.__any = jsdl_dom;

  // std::cout << "jsdl: " << *(req.bes__ActivityDocument.__any) << std::endl;

  if ( soap_call___bes__CreateActivity (s, endpoint, 
                                        CREATE_ACT, &req, &rsp) != SOAP_OK )
  {
    setErrorString (context, s, BESE_SOAP_ERR);
    ret = BESE_SOAP_ERR;
    goto end;
  }

  tmpdom = rsp.__any;

  cleanDom (tmpdom);

  epr = (struct bes_epr *) malloc (sizeof (struct bes_epr));
  
  if ( ! epr )
  {
    setErrorString (context, NULL, BESE_MEM_ALLOC);
    ret = BESE_MEM_ALLOC;
    goto end;
  }

  memset (epr, 0, sizeof (struct bes_epr));

  epr->str = generateEPRString (tmpdom, NULL);
  
  if ( ! epr->str )
  {
    free (epr);
    setErrorString (context, NULL, BESE_MEM_ALLOC);
    ret = BESE_MEM_ALLOC;
    goto end;
  }

  epr->dom = tmpdom;
  
  *activityepr = (epr_t) epr;
  
end:
  jsdl_freeJobDefinitionDOM (jsdl_dom);

  return ret;
}

int
bes_createActivityFromFile(struct bes_context *context, 
                           epr_t endpointepr, 
                           char *jsdlfile, 
                           epr_t *activityepr) 
{
    struct soap *s;
    struct bes__CreateActivityType req;
    struct bes__CreateActivityResponseType rsp;
    struct soap_dom_element dom, *tmpdom;
 // struct soap_dom_attribute *attr;
    struct bes_epr *epr;
    int jsdl, ret = BESE_OK;
 // int size = 0;
    char *endpoint;
    
    if (context == NULL 
        || endpointepr == NULL
        || jsdlfile == NULL 
        || activityepr == NULL) {
        return BESE_BAD_ARG;
    }
    
    s = context->soap;
    
    if ( (ret = generateAddressingHeaders(context, endpointepr, CREATE_ACT, &endpoint)) ) {
        return ret;
    }
    
    jsdl = open(jsdlfile, O_RDONLY, 0);
    if (jsdl == -1) {
        fprintf (stderr, "err 3\n");
        setErrorString(context, NULL, BESE_SYS_ERR);
        return BESE_SYS_ERR;
    }
    
    memset(&dom, 0, sizeof(struct soap_dom_element));
    dom.soap = soap_new1(SOAP_DOM_TREE|SOAP_C_UTFSTRING);
    dom.soap->recvfd = jsdl;
    if (soap_begin_recv(dom.soap)
        || soap_in_xsd__anyType(dom.soap, NULL, &dom, NULL) == NULL
        || soap_end_recv(dom.soap)) {
        setErrorString(context, dom.soap, BESE_SOAP_ERR);
        close(jsdl);
        ret = BESE_SOAP_ERR;
        goto end;
    }
    close(jsdl);
    
    memset(&req, 0, sizeof(struct bes__CreateActivityType));
    memset(&rsp, 0, sizeof(struct bes__CreateActivityResponseType));
    
    req.bes__ActivityDocument.__any = &dom;

    if (soap_call___bes__CreateActivity(s, endpoint, CREATE_ACT, &req, &rsp) != SOAP_OK) {
        setErrorString(context, s, BESE_SOAP_ERR);
        ret = BESE_SOAP_ERR;
        goto end;
    }
    else {
        tmpdom = rsp.__any;
        
        cleanDom(tmpdom);
        
        epr = (struct bes_epr *)malloc(sizeof(struct bes_epr));
        if (!epr) {
            setErrorString(context, NULL, BESE_MEM_ALLOC);
            ret = BESE_MEM_ALLOC;
            goto end;
        }
        memset(epr, 0, sizeof(struct bes_epr));
        
        epr->str = generateEPRString(tmpdom, NULL);
        if (!epr->str) {
            free(epr);
            setErrorString(context, NULL, BESE_MEM_ALLOC);
            ret = BESE_MEM_ALLOC;
            goto end;
        }
        epr->dom = tmpdom;
        *activityepr = (epr_t)epr;
    }
    
 end:
    soap_end(dom.soap);
    soap_free(dom.soap);
    
    return ret;
}

int
bes_createActivityFromString (struct bes_context * context, 
                              epr_t                endpointepr, 
                              char               * jsdl, 
                              epr_t              * activityepr) 
{
  struct soap *s;
  struct bes__CreateActivityType req;
  struct bes__CreateActivityResponseType rsp;
  struct soap_dom_element dom, *tmpdom;
//struct soap_dom_attribute *attr;
  struct bes_epr *epr;
//int    jsdl_fd, size = 0;
  int    ret = BESE_OK;
  char   *endpoint;
//char   filename[] = "/tmp/XXXXXX";

  if ( context     == NULL ||
       endpointepr == NULL ||
       jsdl        == NULL ||
       activityepr == NULL ) 
  {
    return BESE_BAD_ARG;
  }

  s = context->soap;

  if ( (ret = generateAddressingHeaders (context,    endpointepr, 
                                         CREATE_ACT, &endpoint)) )
  {
    return ret;
  }

  memset (&dom, 0, sizeof (struct soap_dom_element));

  dom.soap = soap_new1 (SOAP_DOM_TREE | SOAP_C_UTFSTRING);

  std::istringstream iss (jsdl, std::istringstream::in);

  iss >> dom;

  if ( dom.soap->error )
  {
    setErrorString (context, dom.soap, BESE_SOAP_ERR);
    ret = BESE_SOAP_ERR;
    goto end;
  }

  memset (&req, 0, sizeof (struct bes__CreateActivityType));
  memset (&rsp, 0, sizeof (struct bes__CreateActivityResponseType));

  req.bes__ActivityDocument.__any = &dom;

  if ( soap_call___bes__CreateActivity (s, endpoint, CREATE_ACT, &req, &rsp) != SOAP_OK )
  {
    setErrorString (context, s, BESE_SOAP_ERR);
    ret = BESE_SOAP_ERR;
    goto end;
  }
  else 
  {
    tmpdom = rsp.__any;

    cleanDom (tmpdom);

    epr = (struct bes_epr *) malloc (sizeof (struct bes_epr));

    if ( ! epr ) 
    {
      setErrorString (context, NULL, BESE_MEM_ALLOC);
      ret = BESE_MEM_ALLOC;
      goto end;
    }

    memset (epr, 0, sizeof (struct bes_epr));

    epr->str = generateEPRString (tmpdom, NULL);
    
    if ( ! epr->str ) 
    {
      free (epr);
      setErrorString (context, NULL, BESE_MEM_ALLOC);
      ret = BESE_MEM_ALLOC;
      goto end;
    }

    epr->dom = tmpdom;
    
    *activityepr = (epr_t)epr;
  }

end:
  soap_end  (dom.soap);
  soap_done (dom.soap);
  free      (dom.soap);

  return ret;
}

int
bes_terminateActivities(struct bes_context *context, 
                        epr_t endpointepr, 
                        epr_t activityepr) 
{
    struct soap *s;
    struct bes__TerminateActivitiesType req;
    struct bes__TerminateActivitiesResponseType rsp;
    struct bes_epr *epr;
    int i, ret = BESE_OK;
    char *endpoint;
    
    if (context == NULL || activityepr == NULL || endpointepr == NULL) {
        return BESE_BAD_ARG;
    }
    
    s = context->soap;
    epr = (struct bes_epr *)activityepr;
    
    if ( (ret = generateAddressingHeaders(context, endpointepr, TERMINATE_ACT, &endpoint)) ) {
        return BESE_ERROR;
    }
    
    memset(&req, 0, sizeof(struct bes__TerminateActivitiesType));
    memset(&rsp, 0, sizeof(struct bes__TerminateActivitiesResponseType));
    
    req.__any = epr->dom;
    if (soap_call___bes__TerminateActivities(s, endpoint, TERMINATE_ACT, req, &rsp) != SOAP_OK) {
        setErrorString(context, s, BESE_SOAP_ERR);
        ret = BESE_ERROR;
    }
    else {
        for (i = 0; i < rsp.__sizeResponse; i++) {
            if (rsp.Response[i].Cancelled == true_) {
                ret = BESE_OK;
            }
            else {
                ret = BESE_ERROR;
            }
        }
    }
    
    return ret;
}

int
bes_getActivityStatuses(struct bes_context *context, 
                        epr_t endpointepr, 
                        epr_t activityepr, 
                        struct bes_activity_status *status) 
{
    struct soap *s;
    struct bes__GetActivityStatusesType req;
    struct bes__GetActivityStatusesResponseType rsp;
    struct bes_epr *epr;
    int ret = BESE_OK;
    char *endpoint;
    
    if (context == NULL 
        || activityepr == NULL 
        || status == NULL
        || endpointepr == NULL) {
        return BESE_BAD_ARG;
    }
    
    s = context->soap;
    epr = (struct bes_epr *)activityepr;
    
    memset(status, 0, sizeof(struct bes_activity_status));
    
    if ( (ret = generateAddressingHeaders(context, endpointepr, STATUS_ACT, &endpoint)) ) {
        return ret;
    }
    
    memset(&req, 0, sizeof(struct bes__GetActivityStatusesType));
    memset(&rsp, 0, sizeof(struct bes__GetActivityStatusesResponseType));
    
    req.__any = epr->dom;
    if (soap_call___bes__GetActivityStatuses(s, endpoint, STATUS_ACT, req, &rsp) != SOAP_OK) {
        setErrorString(context, s, BESE_SOAP_ERR);
        ret = BESE_SOAP_ERR;
        goto end;
    }
    else {
        if ((rsp.__any!=NULL)&&(!strcmp(rsp.__any->name,"Response"))) {
            struct soap_dom_element *element = rsp.__any->elts;
            int size;
            
            while (element != NULL) 
            {
              // bes_printDom(element);
              if (!strcmp(element->name,"ActivityStatus")) {
                    if(element->atts!=NULL) {
                        struct soap_dom_attribute *attributes=element->atts;
                        
                        while (attributes != NULL) {
                            if((attributes->name!=NULL)&&(!strcmp(attributes->name,"state"))) {
                                if((attributes->data!=NULL)&&(!strcmp(attributes->data,"Pending"))) {
                                    status->state = BES_Pending;
                                }
                                else if((attributes->data!=NULL)&&(!strcmp(attributes->data,"Running"))) {
                                    status->state = BES_Running;
                                }
                                else if((attributes->data!=NULL)&&(!strcmp(attributes->data,"Cancelled"))) {
                                    status->state = BES_Cancelled;
                                }
                                else if((attributes->data!=NULL)&&(!strcmp(attributes->data,"Failed"))) {
                                    status->state = BES_Failed;
                                }
                                else if((attributes->data!=NULL)&&(!strcmp(attributes->data,"Finished"))) {
                                    status->state = BES_Finished;
                                }
                                break;
                            }
                            attributes=attributes->next;
                        }
                        
                        if (element->elts) {
                            size = calcDomSize(element->elts, NULL);
                            
                            status->substate = (char *)malloc(size + 1);
                            if (!status->substate) {
                                setErrorString(context, NULL, BESE_MEM_ALLOC);
                                ret = BESE_MEM_ALLOC;
                                goto end;
                            }
                            
                            memset(status->substate, 0, size + 1);
                            sprintDom(element->elts, status->substate, NULL);
                        }
                    }
                    
                    break;
                }
                else if(!strcmp(element->name,"Fault")) {
                    setErrorString(context, NULL, BESE_GETSTATUS_ERR);
                    ret = BESE_GETSTATUS_ERR;
                    goto end;
                }
                else {
                    element=element->next;
                }
            }
        }
    }
    
 end:
    return ret;
}

int
getActivityDocumentsDOM(struct bes_context *context, 
                        epr_t endpointepr, 
                        epr_t activityepr, 
                        struct soap_dom_element **dom) 
{
    struct soap *s;
    struct bes__GetActivityDocumentsType req;
    struct bes__GetActivityDocumentsResponseType rsp;
    struct bes_epr *epr;
    int    ret = BESE_OK;
 // int    size = 0;
    char   *endpoint;
 // char   *str;
    
    if (context == NULL
        || endpointepr == NULL
        || activityepr == NULL
        || dom == NULL) {
        return BESE_BAD_ARG;
    }
    
    s = context->soap;
    epr = (struct bes_epr *)activityepr;
    
    if ( (ret = generateAddressingHeaders(context, endpointepr, ACTIVITIES_ACT, &endpoint)) ) {
        return ret;
    }
    
    memset(&req, 0, sizeof(struct bes__GetActivityDocumentsType));
    memset(&rsp, 0, sizeof(struct bes__GetActivityDocumentsResponseType));
    
    req.__any = epr->dom;
    if (soap_call___bes__GetActivityDocuments(s, endpoint, ACTIVITIES_ACT, req, &rsp) != SOAP_OK) {
        setErrorString(context, s, BESE_SOAP_ERR);
        ret = BESE_SOAP_ERR;
        goto end;
    }

    *dom = rsp.__any;
    
 end:
    return ret;
}

int
bes_getActivityDocumentsXML(struct bes_context *context, 
                         epr_t endpointepr, 
                         epr_t activityepr, 
                         char **strDoc) 
{
    struct soap_dom_element *dom;
    int size = 0, ret = BESE_OK;
    char *str;

    if (strDoc == NULL) {
        setErrorString(context, NULL, BESE_BAD_ARG);
        return BESE_BAD_ARG;
    }
    
    ret = getActivityDocumentsDOM(context, endpointepr, activityepr, &dom);
    if (ret != BESE_OK) {
        return ret;
    }

    size  = calcDomSize(dom, NULL);
    
    str = (char *)malloc(size + 1);
    if (!str) {
        setErrorString(context, NULL, BESE_MEM_ALLOC);
        return BESE_MEM_ALLOC;
    }
    memset(str, 0, size + 1);
        
    sprintDom(dom, str, NULL);
        
    *strDoc = str;
    
    return BESE_OK;
}

int
bes_getActivityDocuments(struct bes_context *context, 
                         epr_t endpointepr, 
                         epr_t activityepr, 
                         struct bes_activity_document **activity) 
{
    struct soap_dom_element *dom, *epr_dom, *jsdl_dom;
    struct bes_activity_document *doc;
    struct bes_epr *epr;
    int    ret = BESE_OK;
 // int    size = 0;
 // char *str;

    if (activity == NULL) {
        setErrorString(context, NULL, BESE_BAD_ARG);
        return BESE_BAD_ARG;
    }
    
    ret = getActivityDocumentsDOM(context, endpointepr, activityepr, &dom);
    if (ret != BESE_OK) {
        return ret;
    }
    cleanDom(dom);

    doc = (struct bes_activity_document*)malloc(sizeof(struct bes_activity_document));
    if (doc == NULL) {
        setErrorString(context, NULL, BESE_MEM_ALLOC);
        return BESE_MEM_ALLOC;
    }
    memset(doc, 0, sizeof(struct bes_activity_document));

    epr = (struct bes_epr*)malloc(sizeof(struct bes_epr));
    if (epr == NULL) {
        free(doc);
        setErrorString(context, NULL, BESE_MEM_ALLOC);
        return BESE_MEM_ALLOC;
    }
    memset(epr, 0, sizeof(struct bes_epr));

    epr_dom = dom->elts;
    jsdl_dom = epr_dom->next;
    epr_dom->next = NULL;

    epr->str = generateEPRString(epr_dom, NULL);
    if (epr->str == NULL) {
        free(epr);
        free(doc);
        setErrorString(context, NULL, BESE_MEM_ALLOC);
        return BESE_MEM_ALLOC;
    }

    doc->activityepr = (epr_t)epr;

    if (isElement(jsdl_dom, BES_NS, "JobDefinition")) {
        ret = jsdl_processJobDefinition(jsdl_dom, &doc->activity);
        if (ret != BESE_OK) {
            setErrorString(context, NULL, ret);
            return ret;
        }
    }
    /* need to process the fault in an else clause */
    
    *activity = doc;

    return BESE_OK;
}

void
bes_freeActivityDocument(struct bes_activity_document *adoc)
{
    if (adoc == NULL) {
        return;
    }
    if (adoc->activityepr) {
        bes_freeEPR(&adoc->activityepr);
        adoc->activityepr = NULL;
    }
    if (adoc->activity) {
        jsdl_freeJobDefinition(adoc->activity);
        adoc->activity = NULL;
    }
    if (adoc->fault) {
        free(adoc->fault);
        adoc->fault = NULL;
    }
}

int
bes_getFactoryAttributesDocument(struct bes_context *context, 
                                 epr_t endpointepr, 
                                 char **strDoc) 
{
    struct soap *s;
    struct bes__GetFactoryAttributesDocumentType req;
    struct bes__GetFactoryAttributesDocumentResponseType rsp;
    char *endpoint, *str;
    int size = 0, ret = BESE_OK;
    
    if (context == NULL || endpointepr == NULL || strDoc == NULL) {
        return BESE_BAD_ARG;
    }
    
    s = context->soap;
    
    if ( (ret = generateAddressingHeaders(context, endpointepr, FACTORY_ACT, &endpoint)) ) {
        return ret;
    }
    
    memset(&req, 0, sizeof(struct bes__GetFactoryAttributesDocumentType));
    memset(&rsp, 0, sizeof(struct bes__GetFactoryAttributesDocumentResponseType));
    
    if (soap_call___bes__GetFactoryAttributesDocument(s, endpoint, FACTORY_ACT, &req, &rsp) != SOAP_OK) {
        setErrorString(context, s, BESE_SOAP_ERR);
        ret = BESE_SOAP_ERR;
    }
    else {
        size = calcDomSize(rsp.__any, NULL);
        str = (char *)malloc(size + 1);
        if (!str) {
            setErrorString(context, NULL, BESE_MEM_ALLOC);
            return BESE_MEM_ALLOC;
        }
        memset(str, 0, size + 1);
        
        sprintDom(rsp.__any, str, NULL);
        *strDoc = str;
    }
    
    return ret;
}

void
bes_freeActivityStatus (struct bes_activity_status *status)
{
    if (!status) {
        return;
    }

    if (status->substate) {
        free(status->substate);
        status->substate = NULL;
    }
}

void 
bes_freeEPR(epr_t *epr)
{
    struct bes_epr *tmpEPR;
    
    if (!epr || !(*epr)) {
        return;
    }
    
    tmpEPR = (struct bes_epr *)(*epr);
    
    if (tmpEPR->domCreateFlag == FromMalloc) {
        soap_end(tmpEPR->dom->soap);
        soap_done(tmpEPR->dom->soap);
        free(tmpEPR->dom->soap);
    }
    
    free(tmpEPR->str);
    free(tmpEPR);
    
    *epr = NULL;
}

int 
bes_writeEPRToFile(struct bes_context *context, char *filename, epr_t epr)
{
    struct bes_epr *tmpEPR;
    int fd;
    
    if (context == NULL || epr == NULL || filename == NULL) {
        return BESE_BAD_ARG;
    }
    
    tmpEPR = (struct bes_epr *)(epr);
    
    fd = open(filename, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
    if (fd == -1) {
        fprintf (stderr, "err 7\n");
        setErrorString(context, NULL, BESE_SYS_ERR);
        return BESE_SYS_ERR;
    }
    
    if (write(fd, tmpEPR->str, strlen(tmpEPR->str)) == -1) {
        fprintf (stderr, "err 8\n");
        setErrorString(context, NULL, BESE_SYS_ERR);
        return BESE_SYS_ERR;
    }
    
    close(fd);
    return BESE_OK;
}

int 
bes_readEPRFromFile(struct bes_context *context, char *filename, epr_t *epr)
{
    int fd, size = 0, ret = BESE_OK;
    struct soap_dom_element *dom;
    struct bes_epr *tmpEPR;
    struct stat fileStat;
    
    if (context == NULL || epr == NULL || filename == NULL) {
        return BESE_BAD_ARG;
    }
    
    fd = open(filename, O_RDONLY, 0);
    if (fd == -1) {
        fprintf (stderr, "err 9: %s\n", filename);
        setErrorString(context, NULL, BESE_SYS_ERR);
        return BESE_SYS_ERR;
    }
    
    dom = (struct soap_dom_element *)soap_malloc(context->soap, sizeof(struct soap_dom_element));
    if (!dom) {
        close(fd);
        setErrorString(context, NULL, BESE_MEM_ALLOC);
        return BESE_MEM_ALLOC;
    }
    
    dom->soap = soap_new1(SOAP_DOM_TREE|SOAP_C_UTFSTRING);
    dom->soap->recvfd = fd;
    if (soap_begin_recv(dom->soap)
        || soap_in_xsd__anyType(dom->soap, NULL, dom, NULL) == NULL
        || soap_end_recv(dom->soap)) {
        setErrorString(context, dom->soap, BESE_SOAP_ERR);
        ret = BESE_SOAP_ERR;
        goto error_end;
    }
    
    tmpEPR = (struct bes_epr *)malloc(sizeof(struct bes_epr));
    if (!tmpEPR) {
        setErrorString(context, NULL, BESE_MEM_ALLOC);
        ret = BESE_MEM_ALLOC;
        goto error_end;
    }
    memset(tmpEPR, 0, sizeof(struct bes_epr));
    
    if (lseek(fd, 0, SEEK_SET) == -1) {
        free(tmpEPR);
        fprintf (stderr, "err 10\n");
        setErrorString(context, NULL, BESE_SYS_ERR);
        ret = BESE_SYS_ERR;
        goto error_end;
    }
    
    if (fstat(fd, &fileStat)) {
        free(tmpEPR);
        fprintf (stderr, "err 11\n");
        setErrorString(context, NULL, BESE_SYS_ERR);
        ret = BESE_SYS_ERR;
        goto error_end;
    }
    
    size = fileStat.st_size;
    tmpEPR->str = (char *)malloc(size + 1);
    if (!tmpEPR->str) {
        free(tmpEPR);
        setErrorString(context, NULL, BESE_MEM_ALLOC);
        ret = BESE_MEM_ALLOC;
        goto error_end;
    }
    memset(tmpEPR->str, 0, size + 1);
    
    if (read(fd, tmpEPR->str, size) == -1) {
        free(tmpEPR);
        fprintf (stderr, "err 12\n");
        setErrorString(context, NULL, BESE_SYS_ERR);
        ret = BESE_SYS_ERR;
        goto error_end;
    }
    
    close(fd);
    
    tmpEPR->dom = dom;
    tmpEPR->domCreateFlag = FromMalloc;
    *epr = (epr_t)tmpEPR;
    
    return BESE_OK;
    
 error_end:
    close(fd);
    soap_end(dom->soap);
    soap_done(dom->soap);
    free(dom->soap);
    
    return ret;
}


int 
bes_readEPRFromString(struct bes_context *context,const char *str, epr_t *epr)
{
    int fd, size = 0, ret = BESE_OK;
    struct soap_dom_element *dom;
    struct bes_epr *tmpEPR;
    struct stat fileStat;
    char filename[] = "/tmp/XXXXXX";
    
    if (context == NULL || epr == NULL || str == NULL) {
        return BESE_BAD_ARG;
    }
    
    fd = mkstemp(filename);
    if (fd == -1) {
        fprintf (stderr, "err 13\n");
        setErrorString(context, NULL, BESE_SYS_ERR);
        return BESE_SYS_ERR;
    }
    unlink(filename);

    if (write(fd, str, strlen(str)) < 0) {
        close(fd);
        fprintf (stderr, "err 14\n");
        setErrorString(context, NULL, BESE_SYS_ERR);
        return BESE_SYS_ERR;
    }

    if (lseek(fd, 0, SEEK_SET) == -1) {
        close(fd);
        fprintf (stderr, "err 15\n");
        setErrorString(context, NULL, BESE_SYS_ERR);
        return BESE_SYS_ERR;
    }
    
    dom = (struct soap_dom_element *)soap_malloc(context->soap, sizeof(struct soap_dom_element));
    if (!dom) {
        close(fd);
        setErrorString(context, NULL, BESE_MEM_ALLOC);
        return BESE_MEM_ALLOC;
    }
    
    dom->soap = soap_new1(SOAP_DOM_TREE|SOAP_C_UTFSTRING);
    dom->soap->recvfd = fd;
    if (soap_begin_recv(dom->soap)
        || soap_in_xsd__anyType(dom->soap, NULL, dom, NULL) == NULL
        || soap_end_recv(dom->soap)) {
        setErrorString(context, dom->soap, BESE_SOAP_ERR);
        ret = BESE_SOAP_ERR;
        goto error_end;
    }
    
    tmpEPR = (struct bes_epr *)malloc(sizeof(struct bes_epr));
    if (!tmpEPR) {
        setErrorString(context, NULL, BESE_MEM_ALLOC);
        ret = BESE_MEM_ALLOC;
        goto error_end;
    }
    memset(tmpEPR, 0, sizeof(struct bes_epr));
    
    if (lseek(fd, 0, SEEK_SET) == -1) {
        free(tmpEPR);
        fprintf (stderr, "err 16\n");
        setErrorString(context, NULL, BESE_SYS_ERR);
        ret = BESE_SYS_ERR;
        goto error_end;
    }
    
    if (fstat(fd, &fileStat)) {
        free(tmpEPR);
        fprintf (stderr, "err 17\n");
        setErrorString(context, NULL, BESE_SYS_ERR);
        ret = BESE_SYS_ERR;
        goto error_end;
    }
    
    size = fileStat.st_size;
    tmpEPR->str = (char *)malloc(size + 1);
    if (!tmpEPR->str) {
        free(tmpEPR);
        setErrorString(context, NULL, BESE_MEM_ALLOC);
        ret = BESE_MEM_ALLOC;
        goto error_end;
    }
    memset(tmpEPR->str, 0, size + 1);
    
    if (read(fd, tmpEPR->str, size) == -1) {
        free(tmpEPR);
        fprintf (stderr, "err 18\n");
        setErrorString(context, NULL, BESE_SYS_ERR);
        ret = BESE_SYS_ERR;
        goto error_end;
    }
    
    close(fd);
    
    tmpEPR->dom = dom;
    tmpEPR->domCreateFlag = FromMalloc;
    *epr = (epr_t)tmpEPR;
    
    return BESE_OK;
    
 error_end:
    close(fd);
    soap_end(dom->soap);
    soap_done(dom->soap);
    free(dom->soap);
    
    return ret;
}

// init an endpoint epr structure with the epr data given as string
// returns BESE_OK          on success, 
//         BESE_MEM_ALLOC   on memory allocation error
//         BESE_BAD_ARG     if any or the args is NULL

int 
bes_initEPRFromString (struct bes_context * context,    // soap handle
                       const char         * str,        // epr data as string
                       epr_t              * epr)        // resulting epr struct
{
    struct soap_dom_element * dom;
    struct bes_epr          * tmpEPR;
    
    if (context == NULL || 
        epr     == NULL || 
        str     == NULL )
    {
        return BESE_BAD_ARG;
    }
    
    dom = (struct soap_dom_element *) soap_malloc (context->soap, 
                                                   sizeof (struct soap_dom_element));
    if ( ! dom )
    {
        setErrorString (context, NULL, BESE_MEM_ALLOC);
        return BESE_MEM_ALLOC;
    }
    
    dom->soap = soap_new1 (SOAP_DOM_TREE | SOAP_C_UTFSTRING);

    std::istringstream iss (str, std::istringstream::in);

    iss >> (*dom);

    if ( dom->soap->error )
    {
        setErrorString (context, NULL, BESE_SOAP_ERR);
        soap_end  (dom->soap);
        soap_done (dom->soap);
        free      (dom->soap);
        return BESE_SOAP_ERR;
    }
    
    tmpEPR = (struct bes_epr *) malloc (sizeof (struct bes_epr));
    
    if ( ! tmpEPR )
    {
        setErrorString (context, NULL, BESE_MEM_ALLOC);
        soap_end  (dom->soap);
        soap_done (dom->soap);
        free      (dom->soap);
        return BESE_MEM_ALLOC;
    }

    memset (tmpEPR, 0, sizeof (struct bes_epr));
    
    tmpEPR->str           = strdup (str);
    tmpEPR->dom           = dom;
    tmpEPR->domCreateFlag = FromMalloc;

    *epr = (epr_t) tmpEPR;
    
    return BESE_OK;
}

char *
bes_getEPRString(epr_t epr)
{
    if (!epr) {
        return NULL;
    }
    return ((struct bes_epr*)epr)->str;
}

void
bes_printDom(struct soap_dom_element *node, 
             char *current_nstr, 
             int depth) 
{
    struct soap_dom_attribute *attr;
    int i;
    
    if (node == NULL) {
        return;
    }

    if ( current_nstr == NULL )
    {
      current_nstr = strdup (node->nstr);
    }

    bool sameline = true;

    for (i = 0; i < depth; i++)
      fprintf(stdout, "   ");

    fprintf(stdout, "<%s", node->name);
    /* if we don't have a current namespace, or if the current */
    /* namespace is different from this node, emit an xmlns attribute */
    if (node->nstr && (!current_nstr || strcmp(current_nstr, node->nstr))) {
        fprintf(stdout, " xmlns=\"%s\"", node->nstr);
    }
    attr = node->atts;
    while (attr) {
        /* xmlns was handled earlier */
        if (strcmp(attr->name, "xmlns")) {
            fprintf(stdout, " %s=\"%s\"", attr->name, attr->data);
        }
        attr = attr->next;
    }
    fprintf(stdout, ">");
    
    if (node->data && strlen(node->data)) {
        fprintf(stdout, "%s", node->data);
    }
    else
    {
      if ( ! node->elts )
      {
        fprintf(stdout, "\n");
        sameline = false;
      }
    }
    
    if (node->elts)
    {
      fprintf(stdout, "\n");
      sameline = false;
      bes_printDom(node->elts, (char*)node->nstr, depth+1);
    }

    if ( ! sameline )
    {
      for (i = 0; i < depth; i++)
        fprintf(stdout, "   ");
    }

    fprintf(stdout, "</%s>\n", node->name);
    
    if (node->next)
        bes_printDom(node->next, current_nstr, depth);
}
    

int
calcDomSize(struct soap_dom_element *node, 
            char *current_nstr) 
{
    struct soap_dom_attribute *attr;
    int    size = 0;
 // int    i;

    if (node == NULL) {
        return 0;
    }
    
    if (node->elts) {
        size += calcDomSize(node->elts, (char*)node->nstr);
    }
    
    if (node->next) {
        size += calcDomSize(node->next, current_nstr);
    }
    
    /* element name, data and closing element) */
    size += 2*strlen(node->name) + strlen("<></>");
    if (node->data) {
        size += strlen(node->data);
    }
    
    /* if we don't have a current namespace, or if the current */
    /* namespace is different from this node, emit an xmlns attribute */
    if (node->nstr && (!current_nstr || strcmp(current_nstr, node->nstr))) {
        size += strlen(" xmlns=\"\"") + strlen(node->nstr);
    }
    
    /* attributes */
    attr = node->atts;
    while (attr) {
        /* xmlns was handled earlier */
        if (strcmp(attr->name, "xmlns")) {
            size += strlen("=\"\"") + strlen(attr->name) + strlen(attr->data);
        }
        attr = attr->next;
    }

    // fprintf (stdout, "calcDomSize\n----\n");
    // bes_printDom (node, current_nstr, 0);
    // fprintf (stdout, "---- %d -- \n", size);

    // FIXME: for some reason, the computed size value is wrong.  I found the
    // following values:
    // computed_size  actual_size number_of_lines size_diff
    // 
    // 137 142  3 5
    // 208 213  3 5
    // 316 333  5 17
    // 583 617 10 34
    //
    // the 'fix' below opts for wasting space, and simply doubles the size.
    // Someone needs to fix the function proper, but that needs some
    // understanding of the code which I do not have :-P

    return (2 * size);
}

void
sprintDom(struct soap_dom_element *node, 
          char *str, 
          char *current_nstr) 
{
    struct soap_dom_attribute *attr;
 // int i;

    if (node == NULL) {
        return;
    }
    
    sprintf(str, "%s<%s", str, node->name);
    /* if we don't have a current namespace, or if the current */
    /* namespace is different from this node, emit an xmlns attribute */
    if (node->nstr && (!current_nstr || strcmp(current_nstr, node->nstr))) {
        sprintf(str, "%s xmlns=\"%s\"", str, node->nstr);
    }
    attr = node->atts;
    while (attr) {
        /* xmlns was handled earlier */
        if (strcmp(attr->name, "xmlns")) {
            sprintf(str, "%s %s=\"%s\"", str, attr->name, attr->data);
        }
        attr = attr->next;
    }
    sprintf(str, "%s>", str);
    
    if (node->data && strlen(node->data)) {
        sprintf(str, "%s%s", str, node->data);
    }
    
    if (node->elts)
        sprintDom(node->elts, str, (char*)node->nstr);
    
    sprintf(str, "%s</%s>", str, node->name);
    
    if (node->next)
        sprintDom(node->next, str, current_nstr);
    
}

char *
generateEPRString(struct soap_dom_element *node, 
                  char *current_nstr) 
{
 // struct soap_dom_attribute *attr;
    char *epr_buf;
    int   epr_len;
 // int   i;

    if (node == NULL) {
        return NULL;
    }
    
    epr_len = calcDomSize(node, current_nstr);
    epr_buf = (char*)malloc(epr_len+1);
    if (epr_buf == NULL) {
        perror("generateEPRSTring: malloc");
        return NULL;
    }
    memset(epr_buf, 0, epr_len+1);
    
    sprintDom(node, epr_buf, current_nstr);
    
    return epr_buf;
}

void
cleanDom(struct soap_dom_element *node) 
{
    struct soap_dom_attribute *attr;
    char *cp;
    
    if (node == NULL) {
        return;
    }
    cp = strchr(node->name, ':');
    if (cp) {
        cp++;
        node->name = cp;
    }
    if (node->nstr && strlen(node->nstr) == 0)
        node->nstr = NULL;
    attr = node->atts;
    while (attr) {
        cp = strchr(attr->name, ':');
        if (cp) {
            cp++;
            attr->name = cp;
        }
        if (attr->nstr && strlen(attr->nstr) == 0)
            attr->nstr = NULL;
        attr = attr->next;
    }
    if (node->elts)
        cleanDom(node->elts);
    if (node->next)
        cleanDom(node->next);
}

int
generateAddressHeader(struct bes_context *context, 
                      epr_t endpointepr, 
                      char *action,
                      char **endpoint_ret)
{
    struct soap *s;
    struct soap_dom_element *dom, *iter;
    struct bes_epr *epr;
    char *endpoint;
    
    s = context->soap;
    epr = (struct bes_epr *)endpointepr;
    dom = epr->dom;
    
    iter = dom->elts;
    if (!iter || !isElement(iter, WSA_NS, "Address")) {
        setErrorString(context, NULL, BESE_ENDPOINT_ERR);
        return BESE_ENDPOINT_ERR;
    }
    endpoint = soap_strdup(s, iter->data);
    if (endpoint == NULL) {
        setErrorString(context, s, BESE_SOAP_ERR);
        return BESE_SOAP_ERR;
    }
    
    s->header->wsa__To.__item = endpoint;
    *endpoint_ret = endpoint;
    
    s->header->wsa__Action.__item = soap_strdup(s, action);
    if (s->header->wsa__Action.__item == NULL) {
        setErrorString(context, s, BESE_SOAP_ERR);
        return BESE_SOAP_ERR;
    }
    
    return BESE_OK;
}


int
generateAddressingHeaders(struct bes_context *context, 
                          epr_t endpointepr, 
                          const char *action,
                          char **endpoint_ret)
{
    struct soap *s;
    struct soap_dom_element *dom, *iter;
    struct soap_dom_element *refparams, *refparam;
    struct soap_dom_attribute *isrefparam;
    struct bes_epr *epr;
    char *endpoint;
    int i, numrefparams, ret = BESE_OK;
    
    s = context->soap;
    epr = (struct bes_epr*)endpointepr;
    dom = epr->dom;
    
    iter = dom->elts;
    if (!iter || !isElement(iter, WSA_NS, "Address")) {
        setErrorString(context, NULL, BESE_ENDPOINT_ERR);
        ret = BESE_ENDPOINT_ERR;
        goto end;
    }
    endpoint = soap_strdup(s, iter->data);
    if (endpoint == NULL) {
        setErrorString(context, s, BESE_SOAP_ERR);
        ret = BESE_SOAP_ERR;
        goto end;
    }
    s->header->wsa__To.__item = endpoint;
    
    s->header->wsa__Action.__item = soap_strdup(s, action);
    if (s->header->wsa__Action.__item == NULL) {
        setErrorString(context, s, BESE_SOAP_ERR);
        ret = BESE_SOAP_ERR;
        goto end;
    }
    
    iter = iter->next;
    while (iter) {
        if (isElement(iter, WSA_NS, "ReferenceParameters")) {
            isrefparam = (struct soap_dom_attribute*)soap_malloc(s, sizeof(struct soap_dom_attribute));
            if (isrefparam == NULL) {
                setErrorString(context, s, BESE_SOAP_ERR);
                ret = BESE_SOAP_ERR;
                goto end;
            }
            memset(isrefparam, 0, sizeof(struct soap_dom_attribute));
            isrefparam->soap = s;
            isrefparam->nstr = WSA_NS;
            isrefparam->name = soap_strdup(s, "wsa:IsReferenceParameter");
            isrefparam->data = soap_strdup(s, "true");
            if (!isrefparam->name || !isrefparam->data) {
                setErrorString(context, s, BESE_SOAP_ERR);
                ret = BESE_SOAP_ERR;
                goto end;
            }
            
            refparam = iter->elts;
            numrefparams = 0;
            while (refparam) {
                if (refparam->atts) {
                    struct soap_dom_attribute *last, *attr = refparam->atts;
                    while (attr) {
                        if (isAttribute(attr, WSA_NS, "IsReferenceParameter")) {
                            attr->nstr = soap_strdup (s, isrefparam->nstr);
                            attr->name = soap_strdup (s, isrefparam->name);
                            attr->data = soap_strdup (s, isrefparam->data);
                            break;
                        }
                        last = attr;
                        attr = attr->next;
                    }
                    if (!attr) {
                        last->next = isrefparam;
                    }
                }
                else {
                    refparam->atts = isrefparam;
                }
                refparam = refparam->next;
                numrefparams++;
            }
            
            refparams = (struct soap_dom_element*)soap_malloc(s, sizeof(struct soap_dom_element)*numrefparams);
            if (refparams == NULL) {
                setErrorString(context, s, BESE_SOAP_ERR);
                ret = BESE_SOAP_ERR;
                goto end;
            }
            memset(refparams, 0, sizeof(struct soap_dom_element)*numrefparams);
            refparam = iter->elts;
            for (i = 0; i < numrefparams; i++) {
                refparams[i].nstr = soap_strdup (s, refparam->nstr);
                refparams[i].name = soap_strdup (s, refparam->name);
                refparams[i].data = soap_strdup (s, refparam->data);
                refparams[i].atts = refparam->atts;
                refparams[i].soap = refparam->soap;
                refparams[i].elts = refparam->elts;
                refparam = refparam->next;
            }
            
            s->header->__size = numrefparams;
            s->header->__any = refparams;
        }
        
        iter = iter->next;
    }

    if (endpoint_ret) *endpoint_ret = endpoint;
    
 end:
    return ret;
}

int
isElement(struct soap_dom_element *dom, 
          const char *ns, 
          const char *elt)
{
    char *cp;
    
    if (!dom || !ns || !elt) {
        return 0;
    }
    if (dom->nstr && strcmp(dom->nstr, ns)) {
        return 0;
    }
    cp = strchr(dom->name, ':');
    if (cp) {
        cp++;
    }
    else {
        cp = dom->name;
    }
    if (strcmp(cp, elt)) {
        return 0;
    }
    
    return 1;
}

int
isAttribute(struct soap_dom_attribute *attr, 
            const char *ns, 
            const char *elt)
{
    char *cp;
    
    if (!attr || !ns || !elt) {
        return 0;
    }
    if (attr->nstr && strcmp(attr->nstr, ns)) {
        return 0;
    }
    cp = strchr(attr->name, ':');
    if (cp) {
        cp++;
    }
    else {
        cp = attr->name;
    }
    if (strcmp(cp, elt)) {
        return 0;
    }
    
    return 1;
}

void 
setErrorString(struct bes_context *context, 
               struct soap *s, 
               int error_code)
{
    switch (error_code) {
    case BESE_SOAP_ERR:
        soap_sprint_fault(s, context->error_string, MAX_ERRSTR_LEN);
        break;
        
    case BESE_SYS_ERR:
        snprintf(context->error_string, MAX_ERRSTR_LEN, "System error: %s", strerror(errno));
        break;
        
    case BESE_ENDPOINT_ERR:
        snprintf(context->error_string, MAX_ERRSTR_LEN, "%s", BESE_ENDPOINT_STRING);
        break;
        
    case BESE_MEM_ALLOC:
        snprintf(context->error_string, MAX_ERRSTR_LEN, "%s", BESE_MEM_STRING);
        break;
        
    case BESE_GETSTATUS_ERR:
        snprintf(context->error_string, MAX_ERRSTR_LEN, "%s", BESE_GETSTATUS_STRING);
        break;
        
    case BESE_BAD_ARG:
        snprintf(context->error_string, MAX_ERRSTR_LEN, "%s", BESE_BAD_ARG_STRING);
        break;
        
    case BESE_UNSUPPORTED:
        snprintf(context->error_string, MAX_ERRSTR_LEN, "%s", BESE_UNSUPPORTED_STRING);
        break;

    case BESE_XML_FORMAT:
        snprintf(context->error_string, MAX_ERRSTR_LEN, "%s", BESE_XML_FORMAT_STRING);
        break;

    default:
        break;
    }
}


/*
 * Don't know why these aren't being generated, but ....
 */
struct soap_dom_element *
soap_in__bes__CreateActivityFaultMessage(struct soap *s, 
                                         const char *tag, 
                                         struct soap_dom_element *node,
                                         const char *type)
{
    return soap_in_xsd__anyType(s, tag, node, type);
}

int
soap_out__bes__CreateActivityFaultMessage(struct soap *s, 
                                          const char *tag,
                                          int id,
                                          const struct soap_dom_element *node,
                                          const char *type)
{
    return soap_out_xsd__anyType(s, tag, id, node, type);
}

void
soap_serialize__bes__CreateActivityFaultMessage(struct soap *s, 
                                                const struct soap_dom_element *node)
{
    soap_serialize_xsd__anyType(s, node);
}

