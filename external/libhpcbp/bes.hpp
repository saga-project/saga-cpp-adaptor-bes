/* ----------------------------------------------------------------
 * bes.h
 *   
 *     Header file of BES++ client library
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
#ifndef _BES_H
#define _BES_H

#include "jsdl.hpp"

#define MAX_ERRSTR_LEN 256

typedef void * epr_t;

struct bes_context
{
  struct soap * soap;    
  char        * x509cert;
  char        * x509pass;
  char        * capath;
  char        * user;
  char        * pass;

  char error_string[MAX_ERRSTR_LEN];
};

enum bes_activity_state 
{
  BES_Pending   = 0,
  BES_Running   = 1,
  BES_Cancelled = 2,
  BES_Failed    = 3,
  BES_Finished  = 4,
  BES_State_Num = 5
};

struct bes_activity_status 
{
  enum bes_activity_state   state;  
  char                    * substate;
};

struct bes_activity_document 
{
  epr_t                        activityepr;
  struct jsdl_job_definition * activity;
  char                       * fault;
};

struct bes_epr 
{
  char                    * str;
  struct soap_dom_element * dom;
  int                       domCreateFlag;
};

/* Error code */
#define BESE_OK                 0
#define BESE_SOAP_ERR           1
#define BESE_SYS_ERR            2
#define BESE_ENDPOINT_ERR       3
#define BESE_MEM_ALLOC          4
#define BESE_GETSTATUS_ERR      5
#define BESE_BAD_ARG            6
#define BESE_UNSUPPORTED        7
#define BESE_XML_FORMAT         8
#define BESE_ERROR              9

/*Error strings*/
#define BESE_ENDPOINT_STRING    "The endpoint.xml does not contain a valid EPR"
#define BESE_MEM_STRING         "Memory allocation error"
#define BESE_GETSTATUS_STRING   "Failed to get the status of activity"
#define BESE_BAD_ARG_STRING     "Bad arguments for the function"
#define BESE_UNSUPPORTED_STRING "Feature is unsupported by BES++ at this time"
#define BESE_XML_FORMAT_STRING  "Format error in XML request/response"

int          isElement                        (struct soap_dom_element *dom, const char *ns, const char *elt);

int          bes_init                         (struct bes_context **);
int          bes_security                     (struct bes_context *, char *, char *, char *, char *, char *);
int          bes_add_usertoken                (struct bes_context *context, char *user, char *pass);
int          bes_finalize                     (struct bes_context **);
const char * bes_get_lasterror                (struct bes_context *context);

int          bes_createActivity               (struct bes_context *, epr_t, 
                                               struct jsdl_job_definition *, epr_t *);
int          bes_createActivityFromFile       (struct bes_context *, epr_t, char *, epr_t *);
int          bes_createActivityFromString     (struct bes_context *, epr_t, char *, epr_t *);
int          bes_terminateActivities          (struct bes_context *, epr_t, epr_t);
int          bes_getActivityStatuses          (struct bes_context *, epr_t, epr_t, 
                                               struct bes_activity_status *);
int          bes_getActivityDocumentsXML      (struct bes_context *, epr_t, epr_t, char **);
int          bes_getActivityDocuments         (struct bes_context *, epr_t, epr_t, 
                                               struct bes_activity_document **);
void         bes_freeActivityDocument         (struct bes_activity_document *);
int          bes_getFactoryAttributesDocument (struct bes_context *, epr_t, char **);
void         bes_freeEPR                      (epr_t *epr);
int          bes_writeEPRToFile               (struct bes_context *, char *, epr_t);
int          bes_readEPRFromFile              (struct bes_context *, char *, epr_t *);
int          bes_readEPRFromString            (struct bes_context *, const char *, epr_t *);
int          bes_initEPRFromString            (struct bes_context *, const char *, epr_t *);
char       * bes_getEPRString                 (epr_t);
void         bes_freeActivityStatus           (struct bes_activity_status *);

void         bes_printDom                     (struct soap_dom_element*, char * current_nstr = NULL, int depth = 0); 

#endif /* _BES_H */

