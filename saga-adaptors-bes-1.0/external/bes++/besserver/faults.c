/* ----------------------------------------------------------------
 * faults.c
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
#include "faults.h"

static struct SOAP_ENV__Fault *allocate_fault(struct soap*, const char*, const char*);
static struct soap_dom_element* createBESFaultElement(struct soap*, const char *, const char*, struct soap_dom_element **);

int
bes_send_fault(struct soap *s, struct SOAP_ENV__Fault *fault)
{
    /* should deal with version 1 vs version 2 faults here */
    s->fault = fault;
    return SOAP_FAULT;
}

struct SOAP_ENV__Fault*
bes_NotAuthorizedFault(struct soap *s,
        const char *faultstring)
{
    struct SOAP_ENV__Fault *fault;

    fault = allocate_fault(s, BES_FAULT_NOT_AUTHORIZED, faultstring);
    if (fault == NULL) {
        return NULL;
    }

    if (fault->detail) {
        fault->detail->bes__NotAuthorizedFault
                = (struct bes__NotAuthorizedFaultType*)soap_malloc(s,
                        sizeof(struct bes__NotAuthorizedFaultType));
        if (fault->detail->bes__NotAuthorizedFault) {
            memset(fault->detail->bes__NotAuthorizedFault, 0,
                    sizeof(struct bes__NotAuthorizedFaultType));
        }
    }

    return fault;
}

struct soap_dom_element*
bes_NotAuthorizedFaultDOM(struct soap *s, const char *faultstring)
{
    struct soap_dom_element *fault, *besdetail;
    
    fault = createBESFaultElement(s, "NotAuthorizedFault", faultstring, &besdetail);
    if (fault == NULL) {
        return NULL;
    }
    
    return fault;
}

struct SOAP_ENV__Fault*
bes_NotAcceptingFault(struct soap* s,
        const char *faultstring)
{
    struct SOAP_ENV__Fault *fault;

    fault = allocate_fault(s, BES_FAULT_NOT_ACCEPTING, faultstring);
    if (fault == NULL) {
        return NULL;
    }

    if (fault->detail) {
        fault->detail->bes__NotAcceptingNewActivitiesFault
                = (struct bes__NotAcceptingNewActivitiesFaultType*)soap_malloc(
                        s,
                        sizeof(struct bes__NotAcceptingNewActivitiesFaultType));
        if (fault->detail->bes__NotAcceptingNewActivitiesFault) {
            memset(fault->detail->bes__NotAcceptingNewActivitiesFault, 0,
                    sizeof(struct bes__NotAcceptingNewActivitiesFaultType));
        }
    }

    return fault;
}

struct SOAP_ENV__Fault*
bes_UnsupportedFault(struct soap* s,
        const char *faultstring, const char *element)
{
    struct SOAP_ENV__Fault *fault;

    fault = allocate_fault(s, BES_FAULT_UNSUPPORTED, faultstring);
    if (fault == NULL) {
        return NULL;
    }

    if (fault->detail) {
        fault->detail->bes__UnsupportedFeatureFault
                = (struct bes__UnsupportedFeatureFaultType*)soap_malloc(s,
                        sizeof(struct bes__UnsupportedFeatureFaultType));
        if (fault->detail->bes__UnsupportedFeatureFault) {
            memset(fault->detail->bes__UnsupportedFeatureFault, 0,
                    sizeof(struct bes__UnsupportedFeatureFaultType));
        }
        fault->detail->bes__UnsupportedFeatureFault->Feature = (char**)soap_malloc(s, sizeof(char*));
        if (!fault->detail->bes__UnsupportedFeatureFault->Feature) {
            return NULL;
        }
        *fault->detail->bes__UnsupportedFeatureFault->Feature = soap_strdup(s, 
                element?element:"");
        if (!*fault->detail->bes__UnsupportedFeatureFault->Feature) {
            return NULL;
        }
        fault->detail->bes__UnsupportedFeatureFault->__sizeFeature = 1;
    }

    return fault;
}

struct SOAP_ENV__Fault*
bes_CantApplyFault(struct soap* s,
        const char *faultstring)
{
    struct SOAP_ENV__Fault *fault;

    fault = allocate_fault(s, BES_FAULT_CANT_APPLY, faultstring);
    if (fault == NULL) {
        return NULL;
    }

    if (fault->detail) {
        fault->detail->bes__CantApplyOperationToCurrentStateFault
                = (struct bes__CantApplyOperationToCurrentStateFaultType*)soap_malloc(
                        s,
                        sizeof(struct bes__CantApplyOperationToCurrentStateFaultType));
        if (fault->detail->bes__CantApplyOperationToCurrentStateFault) {
            memset(
                    fault->detail->bes__CantApplyOperationToCurrentStateFault,
                    0,
                    sizeof(struct bes__CantApplyOperationToCurrentStateFaultType));
        }
    }

    return fault;
}

struct SOAP_ENV__Fault*
bes_ApplyEventuallyFault(struct soap* s,
        const char *faultstring)
{
    struct SOAP_ENV__Fault *fault;

    fault = allocate_fault(s, BES_FAULT_APPLY_EVENTUALLY, faultstring);
    if (fault == NULL) {
        return NULL;
    }

    if (fault->detail) {
        fault->detail->bes__OperationWillBeAppliedEventuallyFault
                = (struct bes__OperationWillBeAppliedEventuallyFaultType*)soap_malloc(
                        s,
                        sizeof(struct bes__OperationWillBeAppliedEventuallyFaultType));
        if (fault->detail->bes__OperationWillBeAppliedEventuallyFault) {
            memset(
                    fault->detail->bes__OperationWillBeAppliedEventuallyFault,
                    0,
                    sizeof(struct bes__OperationWillBeAppliedEventuallyFaultType));
        }
    }

    return fault;
}

struct SOAP_ENV__Fault*
bes_InvalidActivityFault(struct soap* s,
        const char *faultstring, const char *message)
{
    struct SOAP_ENV__Fault *fault;

    fault = allocate_fault(s, BES_FAULT_INVALID_ACTIVITY, faultstring);
    if (fault == NULL) {
        return NULL;
    }

    if (fault->detail) {
        fault->detail->bes__InvalidActivityIdentifierFault
                = (struct bes__InvalidActivityIdentifierFaultType*)soap_malloc(
                        s,
                        sizeof(struct bes__InvalidActivityIdentifierFaultType));
        if (fault->detail->bes__InvalidActivityIdentifierFault) {
            memset(fault->detail->bes__InvalidActivityIdentifierFault, 0,
                    sizeof(struct bes__InvalidActivityIdentifierFaultType));
            fault->detail->bes__InvalidActivityIdentifierFault->Message = soap_strdup(s, 
                    message?message:"");
            if (!fault->detail->bes__InvalidActivityIdentifierFault->Message) {
                return NULL;
            }
        }
    }
    
    return fault;
}

struct soap_dom_element*
bes_InvalidActivityFaultDOM(struct soap *s,
        const char *faultstring, const char *message)
{
    struct soap_dom_element *fault, *besdetail, *messageElt;
    
    fault = createBESFaultElement(s, "InvalidActivityIdentifierFault", faultstring, &besdetail);
    if (fault == NULL) {
        return NULL;
    }

    messageElt = (struct soap_dom_element*)soap_malloc(s, sizeof(struct soap_dom_element));
    if (messageElt == NULL) {
        return NULL;
    }
    memset(messageElt, 0, sizeof(struct soap_dom_element));
    
    messageElt->name = soap_strdup(s, "Message");
    messageElt->nstr = soap_strdup(s, BES_NS);
    messageElt->data = soap_strdup(s, message);
    messageElt->prnt = besdetail;
    messageElt->soap = s;
    besdetail->elts = messageElt;
    
    return fault;
}

struct SOAP_ENV__Fault*
bes_InvalidRequestFault(struct soap* s,
        const char *faultstring, const char *element)
{
    struct SOAP_ENV__Fault *fault;

    fault = allocate_fault(s, BES_FAULT_INVALID_REQUEST, faultstring);
    if (fault == NULL) {
        return NULL;
    }

    if (fault->detail) {
        fault->detail->bes__InvalidRequestMessageFault
                = (struct bes__InvalidRequestMessageFaultType*)soap_malloc(s,
                        sizeof(struct bes__InvalidRequestMessageFaultType));
        if (fault->detail->bes__InvalidRequestMessageFault) {
            memset(fault->detail->bes__InvalidRequestMessageFault, 0,
                    sizeof(struct bes__InvalidRequestMessageFaultType));
            fault->detail->bes__InvalidRequestMessageFault->InvalidElement = (char**)soap_malloc(s, sizeof(char*));
            if (!fault->detail->bes__InvalidRequestMessageFault->InvalidElement) {
                return NULL;
            }
            *fault->detail->bes__InvalidRequestMessageFault->InvalidElement = soap_strdup(s, 
                    element?element:"");
            if (!*fault->detail->bes__InvalidRequestMessageFault->InvalidElement) {
                return NULL;
            }
            fault->detail->bes__InvalidRequestMessageFault->__sizeInvalidElement = 1;
        }
    }

    return fault;
}

struct SOAP_ENV__Fault*
bes_backend_error(struct soap* s)
{
    return allocate_fault(s, BES_FAULT_SOAP_RECEIVER, LSF_LIB_ERROR);
}

struct soap_dom_element*
bes_backend_errorDOM(struct soap *s)
{
    struct soap_dom_element *fault, *faultcode, *faultstr;
    
    fault = (struct soap_dom_element*)soap_malloc(s, sizeof(struct soap_dom_element));
    faultcode = (struct soap_dom_element*)soap_malloc(s, sizeof(struct soap_dom_element));
    faultstr = (struct soap_dom_element*)soap_malloc(s, sizeof(struct soap_dom_element));
    if (!fault || !faultcode || !faultstr) {
        return NULL;
    }
    memset(fault, 0, sizeof(struct soap_dom_element));
    memset(faultcode, 0, sizeof(struct soap_dom_element));
    memset(faultstr, 0, sizeof(struct soap_dom_element));
    
    faultcode->name = soap_strdup(s, "faultcode");
    faultcode->data = soap_strdup(s, BES_FAULT_SOAP_RECEIVER);
    faultcode->prnt = fault;
    faultcode->next = faultstr;
    faultcode->soap = s;
    
    faultstr->name = soap_strdup(s, "faultstring");
    faultstr->data = soap_strdup(s, LSF_LIB_ERROR);
    faultstr->prnt = fault;
    faultstr->soap = s;
    
    fault->nstr = soap_strdup(s, BES_NS);
    fault->name = soap_strdup(s, "Fault");
    fault->elts = faultcode;
    fault->soap = s;
    
    return fault;
}

struct SOAP_ENV__Fault*
bes_allocation_error(struct soap* s)
{
    return allocate_fault(s, BES_FAULT_SOAP_RECEIVER, MEM_ALLOC);
}

struct soap_dom_element*
bes_allocation_errorDOM(struct soap *s)
{
    struct soap_dom_element *fault, *faultcode, *faultstr;
    
    fault = (struct soap_dom_element*)soap_malloc(s, sizeof(struct soap_dom_element));
    faultcode = (struct soap_dom_element*)soap_malloc(s, sizeof(struct soap_dom_element));
    faultstr = (struct soap_dom_element*)soap_malloc(s, sizeof(struct soap_dom_element));
    if (!fault || !faultcode || !faultstr) {
        return NULL;
    }
    memset(fault, 0, sizeof(struct soap_dom_element));
    memset(faultcode, 0, sizeof(struct soap_dom_element));
    memset(faultstr, 0, sizeof(struct soap_dom_element));
    
    faultcode->name = soap_strdup(s, "faultcode");
    faultcode->data = soap_strdup(s, BES_FAULT_SOAP_RECEIVER);
    faultcode->prnt = fault;
    faultcode->next = faultstr;
    faultcode->soap = s;
    
    faultstr->name = soap_strdup(s, "faultstring");
    faultstr->data = soap_strdup(s, MEM_ALLOC);
    faultstr->prnt = fault;
    faultstr->soap = s;
    
    fault->nstr = soap_strdup(s, BES_NS);
    fault->name = soap_strdup(s, "Fault");
    fault->elts = faultcode;
    fault->soap = s;
    
    return fault;
}

static struct SOAP_ENV__Fault*
allocate_fault(struct soap *s,
        const char *faultcode, const char *faultstring)
{
    struct SOAP_ENV__Fault *fault;

    /* require a faultcode */
    if (faultcode == NULL) {
        return NULL;
    }

    fault = (struct SOAP_ENV__Fault*)soap_malloc(s,
            sizeof(struct SOAP_ENV__Fault));
    if (fault == NULL) {
        return NULL;
    }
    soap_default_SOAP_ENV__Fault(s, fault);

    fault->faultcode = soap_strdup(s, faultcode);

    if (faultstring) {
        fault->faultstring = soap_strdup(s, faultstring);
    }

    fault->detail = (struct SOAP_ENV__Detail*)soap_malloc(s,
            sizeof(struct SOAP_ENV__Detail));
    if (fault->detail) {
        soap_default_SOAP_ENV__Detail(s, fault->detail);
    }

    return fault;
}

static struct soap_dom_element*
createBESFaultElement(struct soap *s, 
        const char *bescode, const char *faultstring,
        struct soap_dom_element **besdetailpp)
{
    struct soap_dom_element *fault, *faultcode, *faultstr;
    struct soap_dom_element *detail, *besdetail;
    
    if (!s || !bescode || !faultstring || !besdetailpp) {
        return NULL;
    }
    
    fault = (struct soap_dom_element*)soap_malloc(s, sizeof(struct soap_dom_element));
    faultcode = (struct soap_dom_element*)soap_malloc(s, sizeof(struct soap_dom_element));
    faultstr = (struct soap_dom_element*)soap_malloc(s, sizeof(struct soap_dom_element));
    detail = (struct soap_dom_element*)soap_malloc(s, sizeof(struct soap_dom_element));
    besdetail = (struct soap_dom_element*)soap_malloc(s, sizeof(struct soap_dom_element));
    if (!fault || !faultcode || !faultstr || !detail || !besdetail) {
        return NULL;
    }
    memset(fault, 0, sizeof(struct soap_dom_element));
    memset(faultcode, 0, sizeof(struct soap_dom_element));
    memset(faultstr, 0, sizeof(struct soap_dom_element));
    memset(detail, 0, sizeof(struct soap_dom_element));
    memset(besdetail, 0, sizeof(struct soap_dom_element));
    
    faultcode->name = soap_strdup(s, "faultcode");
    faultcode->data = (char*)soap_malloc(s, strlen("bes:") + strlen(bescode) + 1);
    sprintf(faultcode->data, "bes:%s", bescode);
    faultcode->prnt = fault;
    faultcode->next = faultstr;
    faultcode->soap = s;
    
    faultstr->name = soap_strdup(s, "faultstring");
    faultstr->data = soap_strdup(s, faultstring);
    faultstr->prnt = fault;
    faultstr->next = detail;
    faultstr->soap = s;
    
    detail->name = soap_strdup(s, "detail");
    detail->elts = besdetail;
    detail->prnt = fault;
    detail->soap = s;

    besdetail->name = soap_strdup(s, bescode);
    besdetail->nstr = soap_strdup(s, BES_NS);
    besdetail->prnt = detail;
    besdetail->soap = s;
    
    fault->nstr = soap_strdup(s, BES_NS);
    fault->name = soap_strdup(s, "Fault");
    fault->elts = faultcode;
    fault->soap = s;
    
    *besdetailpp = besdetail;
    
    return fault;
}

