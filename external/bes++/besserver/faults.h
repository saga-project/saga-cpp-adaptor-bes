/* ----------------------------------------------------------------
 * faults.h
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
#ifndef _FAULTS_H
#define _FAULTS_H

#include "soapH.h"
#include "namespaces.h"

#define BESE_OK              0
#define BESE_NO_ACTIVITY     1
#define BESE_PERMISSION      2
#define BESE_BAD_ARG         3
#define BESE_SYS_ERR         4
#define BESE_OTHER           5
#define BESE_BACKEND         6
#define BESE_MEM_ALLOC       7

/* fault strings */
#define MEM_ALLOC "Memory allocation error"
#define LSF_LIB_ERROR "Failed in an LSF library call"
#define INVALID_JSDL "Invalid JSDL document"
#define SYS_ERROR "A system error occurred"
#define UNKNOWN_ERROR "Unknown error"
#define ELEMENT_UNSUPPORTED "Element is unsupported"
#define ELEMENT_UNKNOWN "Element is unrecognized"
#define BACKEND_ERROR "Failed in a call to the backend resource manager"

#define BES_FAULT_NOT_AUTHORIZED "bes:NotAuthorizedFault"
#define BES_FAULT_NOT_ACCEPTING "bes:NotAcceptingNewActivitiesFault"
#define BES_FAULT_UNSUPPORTED "bes:UnsupportedFeatureFault"
#define BES_FAULT_CANT_APPLY "bes:CantApplyOperationToCurrentStateFault"
#define BES_FAULT_APPLY_EVENTUALLY "bes:OperationWillBeAppliedEventuallyFault"
#define BES_FAULT_INVALID_ACTIVITY "bes:InvalidActivityIdentifierFault"
#define BES_FAULT_INVALID_REQUEST "bes:InvalidRequestMessageFault"
#define BES_FAULT_SOAP_RECEIVER "SOAP-ENV:Server"

int bes_send_fault(struct soap*, struct SOAP_ENV__Fault*);
struct SOAP_ENV__Fault *bes_NotAuthorizedFault(struct soap*, const char*);
struct soap_dom_element *bes_NotAuthorizedFaultDOM(struct soap*, const char*);
struct SOAP_ENV__Fault *bes_NotAcceptingFault(struct soap*, const char*);
struct SOAP_ENV__Fault *bes_UnsupportedFault(struct soap*, const char*, const char*);
struct SOAP_ENV__Fault *bes_CantApplyFault(struct soap*, const char*);
struct SOAP_ENV__Fault *bes_ApplyEventuallyFault(struct soap*, const char*);
struct SOAP_ENV__Fault *bes_InvalidActivityFault(struct soap*, const char*, const char*);
struct soap_dom_element *bes_InvalidActivityFaultDOM(struct soap*, const char*, const char*);
struct SOAP_ENV__Fault *bes_InvalidRequestFault(struct soap*, const char*, const char*);
struct SOAP_ENV__Fault *bes_backend_error(struct soap*);
struct soap_dom_element *bes_backend_errorDOM(struct soap*);
struct SOAP_ENV__Fault *bes_allocation_error(struct soap*);
struct soap_dom_element *bes_allocation_errorDOM(struct soap*);

#endif /* _FAULTS_H */
