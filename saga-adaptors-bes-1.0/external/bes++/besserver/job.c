/* ----------------------------------------------------------------
 * job.c
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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>

#include "job.h"

int 
addArg(struct soap *s, struct jobcard *jc, char *arg)
{
    char *cp, **cpp;
    int i;

    cp = soap_strdup(s, arg);
    if (cp == NULL) {
        return soap_receiver_fault(s, MEM_ALLOC, NULL);
    }

    cpp = (char**)soap_malloc(s, sizeof(char*)*(jc->num_args+1));
    if (cpp == NULL) {
        return soap_receiver_fault(s, MEM_ALLOC, NULL);
    }

    for (i = 0; i < jc->num_args; i++)
        cpp[i] = jc->args[i];
    cpp[i] = cp;

    if (jc->args) {
        soap_dealloc(s, jc->args);
    }

    jc->args = cpp;
    jc->num_args++;

    return SOAP_OK;
}

int 
addEnv(struct soap *s, struct jobcard *jc, struct soap_dom_element *dom)
{
    struct envvar *newvar, *cur;
    struct soap_dom_attribute *attr;

    newvar = (struct envvar*)soap_malloc(s, sizeof(struct envvar));
    if (newvar == NULL) {
        return soap_receiver_fault(s, MEM_ALLOC, NULL);
    }
    memset(newvar, 0, sizeof(struct envvar));

    newvar->val = soap_strdup(s, dom->data);
    if (newvar->val == NULL) {
        return soap_receiver_fault(s, MEM_ALLOC, NULL);
    }

    newvar->name = NULL;
    for (attr = dom->atts; attr; attr = attr->next) {
        if (!strcmp(attr->name, "name")) {
            newvar->name = soap_strdup(s, attr->data);
            if (newvar->name == NULL) {
                return soap_receiver_fault(s, MEM_ALLOC, NULL);
            }
        }
    }
    if (newvar->name == NULL) {
        return soap_receiver_fault(s, "Environment element missing name", NULL);
    }

    if (jc->environment == NULL) {
        jc->environment = newvar;
    }
    else {
        cur = jc->environment;
        while (cur->next) {
            cur = cur->next;
        }
        cur->next = newvar;
    }
       
    return SOAP_OK;
}

int 
addHost(struct soap *s, struct jobcard *jc, char *host)
{
    char *cp, **cpp;
    int i;

    cp = soap_strdup(s, host);
    if (cp == NULL) {
        return soap_receiver_fault(s, MEM_ALLOC, NULL);
    }

    cpp = (char**)soap_malloc(s, sizeof(char*)*(jc->num_hostnames+1));
    if (cpp == NULL) {
        return soap_receiver_fault(s, MEM_ALLOC, NULL);
    }

    for (i = 0; i < jc->num_hostnames; i++)
        cpp[i] = jc->hostnames[i];
    cpp[i] = cp;

    if (jc->hostnames) {
        soap_dealloc(s, jc->hostnames);
    }

    jc->hostnames = cpp;
    jc->num_hostnames++;

    return SOAP_OK;
}

int
processCredential(struct soap *s, struct soap_dom_element *dom,
        struct credential **cred)
{
    struct soap_dom_element *cur = dom->elts;
    struct soap_dom_element *user, *pass;
    struct credential *new_cred = NULL;

    printf("----%s\n", dom->name);
    
    while (cur) {
        if (isElement(cur, WSSE_NS, "UsernameToken")) {
            printf("-----%s\n", cur->name);
            user = cur->elts;
            if (!isElement(user, WSSE_NS, "Username")) {
                struct SOAP_ENV__Fault *fault;
                fault = bes_UnsupportedFault(s, ELEMENT_UNKNOWN, user->name);
                if (!fault) {
                    return soap_receiver_fault(s, MEM_ALLOC, NULL);
                }
                return bes_send_fault(s, fault);
            }
            pass = user->next;
            if (!isElement(pass, WSSE_NS, "Password")) {
                struct SOAP_ENV__Fault *fault;
                fault = bes_UnsupportedFault(s, ELEMENT_UNKNOWN, pass->name);
                if (!fault) {
                    return soap_receiver_fault(s, MEM_ALLOC, NULL);
                }
                return bes_send_fault(s, fault);
            }
            new_cred = (struct credential*)soap_malloc(s, sizeof(struct credential));
            if (new_cred == NULL) {
                return soap_receiver_fault(s, MEM_ALLOC, NULL);
            }
            new_cred->username = soap_strdup(s, user->data);
            new_cred->password = soap_strdup(s, pass->data);
            if (!new_cred->username || !new_cred->password) {
                return soap_receiver_fault(s, MEM_ALLOC, NULL);
            }
            *cred = new_cred;
        }
        else {
            struct SOAP_ENV__Fault *fault;
            fault = bes_UnsupportedFault(s, ELEMENT_UNKNOWN, cur->name);
            if (!fault) {
                return soap_receiver_fault(s, MEM_ALLOC, NULL);
            }
            return bes_send_fault(s, fault);
        }
        cur = cur->next;
    }
    return SOAP_OK;
}
int 
processHPCProfileApplication(struct soap *s, struct soap_dom_element *dom,
        struct jobcard *jc)
{
    struct soap_dom_element *cur = dom->elts;
    int rc;

    printf("----%s\n", dom->name);

    while (cur) {
        if (isElement(cur, JSDL_HPCPA_NS, "Executable")) {
            printf("-----Executable = %s\n", cur->data);
            if (cur->data && strlen(cur->data)) {
                jc->executable = soap_strdup(s, cur->data);
                if (jc->executable == NULL) {
                    return soap_receiver_fault(s, MEM_ALLOC, NULL);
                }
            } 
            else {
                struct SOAP_ENV__Fault *fault;
                fault = bes_InvalidRequestFault(s, 
                        "LSF requires JSDL Executable", cur->name);
                if (!fault) {
                    return soap_receiver_fault(s, MEM_ALLOC, NULL);
                }
                return bes_send_fault(s, fault);
            }
        } else if (isElement(cur, JSDL_HPCPA_NS, "Argument")) {
            printf("-----Argument = %s\n", cur->data);
            if ((rc = addArg(s, jc, cur->data)) != SOAP_OK) {
                return rc;
            }
        } else if (isElement(cur, JSDL_HPCPA_NS, "Input")) {
            printf("-----Input = %s\n", cur->data);
            if (cur->data && strlen(cur->data)) {
                jc->input = soap_strdup(s, cur->data);
                if (jc->input == NULL) {
                    return soap_receiver_fault(s, MEM_ALLOC, NULL);
                }
            }
        } else if (isElement(cur, JSDL_HPCPA_NS, "Output")) {
            printf("-----Output = %s\n", cur->data);
            if (cur->data && strlen(cur->data)) {
                jc->output = soap_strdup(s, cur->data);
                if (jc->output == NULL) {
                    return soap_receiver_fault(s, MEM_ALLOC, NULL);
                }
            }
        } else if (isElement(cur, JSDL_HPCPA_NS, "Error")) {
            printf("-----Error = %s\n", cur->data);
            if (cur->data && strlen(cur->data)) {
                jc->error = soap_strdup(s, cur->data);
                if (jc->error == NULL) {
                    return soap_receiver_fault(s, MEM_ALLOC, NULL);
                }
            }
        } else if (isElement(cur, JSDL_HPCPA_NS, "WorkingDirectory")) {
            printf("-----WorkingDirectory = %s\n", cur->data);
            if (cur->data && strlen(cur->data)) {
                jc->wd = soap_strdup(s, cur->data);
                if (jc->wd == NULL) {
                    return soap_receiver_fault(s, MEM_ALLOC, NULL);
                }
            }
        } else if (isElement(cur, JSDL_HPCPA_NS, "Environment")) {
            printf("-----Environment\n");
            if ((rc = addEnv(s, jc, cur)) != SOAP_OK) {
                return rc;
            }
        } else if (isElement(cur, JSDL_HPCPA_NS, "UserName")) {
            printf("-----UserName = %s\n", cur->data);
            if (cur->data && strlen(cur->data)) {
                jc->username = soap_strdup(s, cur->data);
                if (jc->username == NULL) {
                    return soap_receiver_fault(s, MEM_ALLOC, NULL);
                }
            }
        } else {
            struct SOAP_ENV__Fault *fault;
            fault = bes_UnsupportedFault(s, ELEMENT_UNKNOWN, cur->name);
            if (!fault) {
                return soap_receiver_fault(s, MEM_ALLOC, NULL);
            }
            return bes_send_fault(s, fault);
        }
        cur = cur->next;
    }

    return SOAP_OK;
}

int 
processJobIdentification(struct soap *s, struct soap_dom_element *dom,
        struct jobcard *jc)
{
    struct soap_dom_element *cur = dom->elts;
    int rc;

    printf("---%s\n", dom->name);

    while (cur) {
        if (isElement(cur, JSDL_NS, "JobName")) {
            printf("----JobName = %s\n", cur->data);
            jc->jobname = soap_strdup(s, cur->data);
            if (jc->jobname == NULL) {
                return soap_receiver_fault(s, MEM_ALLOC, NULL);
            }
        } else if (isElement(cur, JSDL_NS, "Description")) {
            printf("----Description = %s\n", cur->data);
        } else if (isElement(cur, JSDL_NS, "JobAnnotation")) {
            printf("----JobAnnotation = %s\n", cur->data);
        } else if (isElement(cur, JSDL_NS, "JobProject")) {
            printf("----JobProject = %s\n", cur->data);
            jc->jobproject = soap_strdup(s, cur->data);
            if (jc->jobproject == NULL) {
                return soap_receiver_fault(s, MEM_ALLOC, NULL);
            }
        } else {
            struct SOAP_ENV__Fault *fault;
            fault = bes_UnsupportedFault(s, ELEMENT_UNKNOWN, cur->name);
            if (!fault) {
                return soap_receiver_fault(s, MEM_ALLOC, NULL);
            }
            return bes_send_fault(s, fault);
        }
        cur = cur->next;
    }

    return SOAP_OK;
}

int 
processApplication(struct soap *s, struct soap_dom_element *dom,
        struct jobcard *jc)
{
    struct soap_dom_element *cur = dom->elts;
    int rc;

    printf("---%s\n", dom->name);

    while (cur) {
        if (isElement(cur, JSDL_NS, "ApplicationName")) {
            printf("----ApplicationName = %s\n", cur->data);
            jc->appname = soap_strdup(s, cur->data);
            if (jc->appname == NULL) {
                return soap_receiver_fault(s, MEM_ALLOC, NULL);
            }
        } else if (isElement(cur, JSDL_NS, "ApplicationVersion")) {
            printf("----ApplicationVersion = %s\n", cur->data);
            jc->appversion = soap_strdup(s, cur->data);
            if (jc->appversion == NULL) {
                return soap_receiver_fault(s, MEM_ALLOC, NULL);
            }
        } else if (isElement(cur, JSDL_HPCPA_NS, "HPCProfileApplication")) {
            if ((rc = processHPCProfileApplication(s, cur, jc)) != SOAP_OK) {
                return rc;
            }
        } else {
            struct SOAP_ENV__Fault *fault;
            fault = bes_UnsupportedFault(s, ELEMENT_UNKNOWN, cur->name);
            if (!fault) {
                return soap_receiver_fault(s, MEM_ALLOC, NULL);
            }
            return bes_send_fault(s, fault);
        }
        cur = cur->next;
    }

    return SOAP_OK;
}

int 
processRangeValue(struct soap *s, struct soap_dom_element *dom, int *value)
{
    struct soap_dom_element *elem;
    double val;

    elem = dom->elts;

    if (!isElement(elem, JSDL_NS, "Exact")) {
        struct SOAP_ENV__Fault *fault;
        fault = bes_UnsupportedFault(s, ELEMENT_UNKNOWN, elem->name);
        if (!fault) {
            return soap_receiver_fault(s, MEM_ALLOC, NULL);
        }
        return bes_send_fault(s, fault);
    }

    val = atof(elem->data);
    *value = (int)val;

    return SOAP_OK;
}

int 
processCandidateHosts(struct soap *s, struct soap_dom_element *dom,
        struct jobcard *jc)
{
    struct soap_dom_element *cur = dom->elts;
    int rc;

    while (cur) {
        if (isElement(cur, JSDL_NS, "HostName")) {
            if ((rc = addHost(s, jc, cur->data)) != SOAP_OK) {
                return rc;
            }
        }
        else {
            struct SOAP_ENV__Fault *fault;
            fault = bes_UnsupportedFault(s, ELEMENT_UNKNOWN, cur->name);
            if (!fault) {
                return soap_receiver_fault(s, MEM_ALLOC, NULL);
            }
            return bes_send_fault(s, fault);
        }
        cur = cur->next;
    }

    return SOAP_OK;
}

int 
processOperatingSystem(struct soap *s, struct soap_dom_element *dom,
        struct jobcard *jc)
{
    struct soap_dom_element *cur = dom->elts;
    int rc;

    printf("----%s\n", dom->name);

    while (cur) {
        if (isElement(cur, JSDL_NS, "OperatingSystemType")) {
            if (isElement(cur->elts, JSDL_NS, "OperatingSystemName")) {
                jc->osname = soap_strdup(s, cur->elts->data);
                if (jc->osname == NULL) {
                    return soap_receiver_fault(s, MEM_ALLOC, NULL);
                }
            }
        } else if (isElement(cur, JSDL_NS, "OperatingSystemVersion")) {
            jc->osver = soap_strdup(s, cur->data);
            if (jc->osver == NULL) {
                return soap_receiver_fault(s, MEM_ALLOC, NULL);
            }
        } else if (isElement(cur, JSDL_NS, "Description")) {
            ;
        }
        else {
            struct SOAP_ENV__Fault *fault;
            fault = bes_UnsupportedFault(s, ELEMENT_UNKNOWN, cur->name);
            if (!fault) {
                return soap_receiver_fault(s, MEM_ALLOC, NULL);
            }
            return bes_send_fault(s, fault);
        }
        cur = cur->next;
    }

    return SOAP_OK;
}

int 
processCPUArchitecture(struct soap *s, struct soap_dom_element *dom,
        struct jobcard *jc)
{
    struct soap_dom_element *cur = dom->elts;
    int rc;

    printf("----%s\n", dom->name);

    while (cur) {
        if (isElement(cur, JSDL_NS, "CPUArchitectureName")) {
            jc->cpuarch = soap_strdup(s, cur->data);
            if (jc->cpuarch == NULL) {
                return soap_receiver_fault(s, MEM_ALLOC, NULL);
            }
        }
        else {
            struct SOAP_ENV__Fault *fault;
            fault = bes_UnsupportedFault(s, ELEMENT_UNKNOWN, cur->name);
            if (!fault) {
                return soap_receiver_fault(s, MEM_ALLOC, NULL);
            }
            return bes_send_fault(s, fault);
        }
        cur = cur->next;
    }

    return SOAP_OK;
}

int 
processResources(struct soap *s, struct soap_dom_element *dom,
        struct jobcard *jc)
{
    struct soap_dom_element *cur = dom->elts;
    int rc;

    printf("---%s\n", dom->name);

    while (cur) {
        if (isElement(cur, JSDL_NS, "CandidateHosts")) {
            if ((rc = processCandidateHosts(s, cur, jc)) != SOAP_OK) {
                return rc;
            }
        } else if (isElement(cur, JSDL_NS, "ExclusiveExecution")) {
            printf("----ExclusiveExecution = %s\n", cur->data);
            if (!strcmp(cur->data, "true")) {
                jc->exclusive = 1;
            }
        } else if (isElement(cur, JSDL_NS, "OperatingSystem")) {
            if ((rc = processOperatingSystem(s, cur, jc)) != SOAP_OK) {
                return rc;
            }
        } else if (isElement(cur, JSDL_NS, "CPUArchitecture")) {
            if ((rc = processCPUArchitecture(s, cur, jc)) != SOAP_OK) {
                return rc;
            }
        } else if (isElement(cur, JSDL_NS, "IndividualCPUCount")) {
            printf("----IndividualCPUCount\n");
            if ((rc = processRangeValue(s, cur, &jc->icpu)) != SOAP_OK) {
                return rc;
            }
        } else if (isElement(cur, JSDL_NS, "IndividualPhysicalMemory")) {
            printf("----IndividualPhysicalMemory\n");
            if ((rc = processRangeValue(s, cur, &jc->ipmem)) != SOAP_OK) {
                return rc;
            }
        } else if (isElement(cur, JSDL_NS, "IndividualVirtualMemory")) {
            printf("----IndividualVirtualMemory\n");
            if ((rc = processRangeValue(s, cur, &jc->ivmem)) != SOAP_OK) {
                return rc;
            }
        } else if (isElement(cur, JSDL_NS, "TotalCPUCount")) {
            printf("----TotalCPUCount\n");
            if ((rc = processRangeValue(s, cur, &jc->tcpu)) != SOAP_OK) {
                return rc;
            }
        } else if (isElement(cur, JSDL_NS, "TotalResourceCount")) {
            printf("----TotalResourceCount\n");
            if ((rc = processRangeValue(s, cur, &jc->tres)) != SOAP_OK) {
                return rc;
            }
        } else {
            struct SOAP_ENV__Fault *fault;
            fault = bes_UnsupportedFault(s, ELEMENT_UNKNOWN, cur->name);
            if (!fault) {
                return soap_receiver_fault(s, MEM_ALLOC, NULL);
            }
            return bes_send_fault(s, fault);
        }
        cur = cur->next;
    }

    return SOAP_OK;
}

int 
processDataStaging(struct soap *s, struct soap_dom_element *dom,
        struct jobcard *jc)
{
    struct soap_dom_element *cur = dom->elts;
    int rc;
    struct SOAP_ENV__Fault *fault;
    struct fileStage *file, *cur_file;
    struct credential *cred;
    
    printf("---%s\n", dom->name);

    file = (struct fileStage *)soap_malloc(s, sizeof(struct fileStage));
    if (file == NULL) {
        return soap_receiver_fault(s, MEM_ALLOC, NULL);
    }
    memset(file, 0, sizeof(struct fileStage));
    
    
    while (cur) {
        if (isElement(cur, JSDL_NS, "FileName")) {
            printf("----FileName\n");
            file->filename = soap_strdup(s, cur->data);
            if (file->filename == NULL) {
                return soap_receiver_fault(s, MEM_ALLOC, NULL);
            }
        }
        else if (isElement(cur, JSDL_NS, "FileSystemName")) {
            printf("----FileSystemName\n");
            fault = bes_UnsupportedFault(s, ELEMENT_UNSUPPORTED, cur->name);
            if (!fault) {
                return soap_receiver_fault(s, MEM_ALLOC, NULL);
            }
            return bes_send_fault(s, fault);
        }
        else if (isElement(cur, JSDL_NS, "CreationFlag")) {
            printf("----CreationFlag\n");
            if (!strcmp(cur->data, "overwrite")) {
                file->creationflag = overwrite;
            }
            else if (!strcmp(cur->data, "append")) {
                file->creationflag = append;
            }
            else if (!strcmp(cur->data, "dontOverwrite")) {
                file->creationflag = dontOverwrite;
            }
        }
        else if (isElement(cur, JSDL_NS, "DeleteOnTermination")) {
            printf("----DeleteOnTermination\n");
            if (!strcmp(cur->data, "true")) {
                file->del = true_;
            }
            else {
                file->del = false_;
            }
        }
        else if (isElement(cur, JSDL_NS, "Source")) {
            printf("----Source\n");
            if (isElement(cur->elts, JSDL_NS, "URI")) {
                file->source = soap_strdup(s, cur->elts->data);
                if (file->source == NULL) {
                    return soap_receiver_fault(s, MEM_ALLOC, NULL);
                }
            }
            else {
                fault = bes_UnsupportedFault(s, ELEMENT_UNSUPPORTED, cur->elts->name);
                if (!fault) {
                    return soap_receiver_fault(s, MEM_ALLOC, NULL);
                }
                return bes_send_fault(s, fault);
            }
        }
        else if (isElement(cur, JSDL_NS, "Target")) {
            printf("----Target\n");
            if (isElement(cur->elts, JSDL_NS, "URI")) {
                file->target = soap_strdup(s, cur->elts->data);
                if (file->target == NULL) {
                    return soap_receiver_fault(s, MEM_ALLOC, NULL);
                }
            }
            else {
                fault = bes_UnsupportedFault(s, ELEMENT_UNSUPPORTED, cur->elts->name);
                if (!fault) {
                    return soap_receiver_fault(s, MEM_ALLOC, NULL);
                }
                return bes_send_fault(s, fault);
            }
        }
        else if (isElement(cur, HPCP_AC_NS, "Credential")) {
            cred = NULL;
            if ((rc = processCredential(s, cur, &cred)) != SOAP_OK) {
                return rc;
            }
            file->credential = cred;
        }
        else {
            fault = bes_UnsupportedFault(s, ELEMENT_UNSUPPORTED, cur->name);
            if (!fault) {
                return soap_receiver_fault(s, MEM_ALLOC, NULL);
            }
            return bes_send_fault(s, fault);
        }
        
        cur = cur->next;
    }

    cur_file = jc->files;
    if (cur_file) {
        while (cur_file->next) {
            cur_file = cur_file->next;
        }
        cur_file->next = file;
    }
    else {
        jc->files = file;
    }
    
    return SOAP_OK;
}

int 
processJobDescription(struct soap *s, struct soap_dom_element *dom,
        struct jobcard *jc)
{
    struct soap_dom_element *cur = dom->elts;
    int rc;

    printf("--%s\n", dom->name);

    while (cur) {
        if (isElement(cur, JSDL_NS, "JobIdentification")) {
            if ((rc = processJobIdentification(s, cur, jc)) != SOAP_OK) {
                return rc;
            }
        } else if (isElement(cur, JSDL_NS, "Application")) {
            if ((rc = processApplication(s, cur, jc)) != SOAP_OK) {
                return rc;
            }
        } else if (isElement(cur, JSDL_NS, "Resources")) {
            if ((rc = processResources(s, cur, jc)) != SOAP_OK) {
                return rc;
            }
        } else if (isElement(cur, JSDL_NS, "DataStaging")) {
            if ((rc = processDataStaging(s, cur, jc)) != SOAP_OK) {
                return rc;
            }
        } else {
            struct SOAP_ENV__Fault *fault;
            fault = bes_UnsupportedFault(s, ELEMENT_UNKNOWN, cur->name);
            if (!fault) {
                return soap_receiver_fault(s, MEM_ALLOC, NULL);
            }
            return bes_send_fault(s, fault);
        }
        cur = cur->next;
    }

    return SOAP_OK;
}

int 
processJobDefinition(struct soap *s, struct soap_dom_element *dom,
        struct jobcard *jc)
{
    struct soap_dom_element *cur = dom->elts;

    printf("-%s\n", dom->name);

    if (isElement(cur, JSDL_NS, "JobDescription")) {
        return processJobDescription(s, cur, jc);
    } else {
        struct SOAP_ENV__Fault *fault;
        fault = bes_UnsupportedFault(s, ELEMENT_UNKNOWN, cur->name);
        if (!fault) {
            return soap_receiver_fault(s, MEM_ALLOC, NULL);
        }
        return bes_send_fault(s, fault);
    }
}

int 
processActivityDocument(struct soap *s, struct soap_dom_element *dom,
        struct jobcard *jc)
{
    struct soap_dom_element *cur = dom->elts;

    printf("%s\n", dom->name);

    if (isElement(cur, JSDL_NS, "JobDefinition")) {
        return processJobDefinition(s, cur, jc);
    } else {
        struct SOAP_ENV__Fault *fault;
        fault = bes_UnsupportedFault(s, ELEMENT_UNKNOWN, cur->name);
        if (!fault) {
            return soap_receiver_fault(s, MEM_ALLOC, NULL);
        }
        return bes_send_fault(s, fault);
    }
}

struct soap_dom_element * 
getHPCProfileApplication(struct soap *s, struct jobcard *job)
{
    struct soap_dom_element *dom, *cur, *next;
    char *nstr;
    int i;

    nstr = soap_strdup(s, JSDL_HPCPA_NS);
    if (!nstr) {
        return NULL;
    }

    dom = (struct soap_dom_element*)soap_malloc(s,
            sizeof(struct soap_dom_element));
    if (!dom) {
        return NULL;
    }
    memset(dom, 0, sizeof(struct soap_dom_element));
    dom->nstr = nstr;
    dom->name = soap_strdup(s, "HPCProfileApplication");
    if (!dom->name) {
        return NULL;
    }

    cur = (struct soap_dom_element*)soap_malloc(s,
            sizeof(struct soap_dom_element));
    if (cur == NULL) {
        return NULL;
    }
    memset(cur, 0, sizeof(struct soap_dom_element));
    cur->nstr = nstr;
    cur->prnt = dom;
    cur->name = soap_strdup(s, "Executable");
    if (!cur->name) {
        return NULL;
    }
    cur->data = soap_strdup(s, job->executable);
    if (!cur->data) {
        return NULL;
    }
    dom->elts = cur;

    for (i = 0; i < job->num_args; i++ ) {
        next = (struct soap_dom_element*)soap_malloc(s,
                sizeof(struct soap_dom_element));
        if (next == NULL) {
            return NULL;
        }
        memset(next, 0, sizeof(struct soap_dom_element));
        next->nstr = nstr;
        next->prnt = dom;
        next->name = soap_strdup(s, "Argument");
        if (!next->name) {
            return NULL;
        }
        next->data = soap_strdup(s, job->args[i]);
        if (!next->data) {
            return NULL;
        }
        cur->next = next;
        cur = next;
    }

    if (job->input) {
        next = (struct soap_dom_element*)soap_malloc(s,
                sizeof(struct soap_dom_element));
        if (next == NULL) {
            return NULL;
        }
        memset(next, 0, sizeof(struct soap_dom_element));
        next->nstr = nstr;
        next->prnt = dom;
        next->name = soap_strdup(s, "Input");
        if (!next->name) {
            return NULL;
        }
        next->data = soap_strdup(s, job->input);
        if (!next->data) {
            return NULL;
        }
        cur->next = next;
        cur = next;
    }

    if (job->output) {
        next = (struct soap_dom_element*)soap_malloc(s,
                sizeof(struct soap_dom_element));
        if (next == NULL) {
            return NULL;
        }
        memset(next, 0, sizeof(struct soap_dom_element));
        next->nstr = nstr;
        next->prnt = dom;
        next->name = soap_strdup(s, "Output");
        if (!next->name) {
            return NULL;
        }
        next->data = soap_strdup(s, job->output);
        if (!next->data) {
            return NULL;
        }
        cur->next = next;
        cur = next;
    }

    if (job->error) {
        next = (struct soap_dom_element*)soap_malloc(s,
                sizeof(struct soap_dom_element));
        if (next == NULL) {
            return NULL;
        }
        memset(next, 0, sizeof(struct soap_dom_element));
        next->nstr = nstr;
        next->prnt = dom;
        next->name = soap_strdup(s, "Error");
        if (!next->name) {
            return NULL;
        }
        next->data = soap_strdup(s, job->error);
        if (!next->data) {
            return NULL;
        }
        cur->next = next;
        cur = next;
    }

    if (job->wd) {
        next = (struct soap_dom_element*)soap_malloc(s,
                sizeof(struct soap_dom_element));
        if (next == NULL) {
            return NULL;
        }
        memset(next, 0, sizeof(struct soap_dom_element));
        next->nstr = nstr;
        next->prnt = dom;
        next->name = soap_strdup(s, "WorkingDirectory");
        if (!next->name) {
            return NULL;
        }
        next->data = soap_strdup(s, job->wd);
        if (!next->data) {
            return NULL;
        }
        cur->next = next;
        cur = next;
    }

    if (job->username) {
        next = (struct soap_dom_element*)soap_malloc(s,
                sizeof(struct soap_dom_element));
        if (next == NULL) {
            return NULL;
        }
        memset(next, 0, sizeof(struct soap_dom_element));
        next->nstr = nstr;
        next->prnt = dom;
        next->name = soap_strdup(s, "UserName");
        if (!next->name) {
            return NULL;
        }
        next->data = soap_strdup(s, job->username);
        if (!next->data) {
            return NULL;
        }
        cur->next = next;
        cur = next;
    }

    return dom;
}

int 
getJSDLFromJobInfo(struct soap *s, struct jobcard *job,
        struct jsdl__JobDefinition_USCOREType **jsdl_return)
{
    struct jsdl__JobDefinition_USCOREType *jsdl;
    struct jsdl__JobDescription_USCOREType *jdesc;
    struct jsdl__JobIdentification_USCOREType *ident;
    struct jsdl__Application_USCOREType *app;
    struct jsdl__Resources_USCOREType *res;
    struct jsdl__CandidateHosts_USCOREType *hosts;
    struct jsdl__RangeValue_USCOREType *tcpu;
    struct jsdl__Exact_USCOREType *exact;
    char *cp, *nstr;
    int i;

    if (!job || !jsdl_return) {
        return SOAP_OK;
    }

    jsdl = (struct jsdl__JobDefinition_USCOREType*)soap_malloc(s,
            sizeof(struct jsdl__JobDefinition_USCOREType));
    jdesc = (struct jsdl__JobDescription_USCOREType*)soap_malloc(s,
            sizeof(struct jsdl__JobDescription_USCOREType));
    ident = (struct jsdl__JobIdentification_USCOREType*)soap_malloc(s,
            sizeof(struct jsdl__JobIdentification_USCOREType));
    app = (struct jsdl__Application_USCOREType*)soap_malloc(s,
            sizeof(struct jsdl__Application_USCOREType));
    res = (struct jsdl__Resources_USCOREType*)soap_malloc(s,
            sizeof(struct jsdl__Resources_USCOREType));
    hosts = (struct jsdl__CandidateHosts_USCOREType*)soap_malloc(s,
            sizeof(struct jsdl__CandidateHosts_USCOREType));
    tcpu = (struct jsdl__RangeValue_USCOREType*)soap_malloc(s,
            sizeof(struct jsdl__RangeValue_USCOREType));
    exact = (struct jsdl__Exact_USCOREType*)soap_malloc(s,
            sizeof(struct jsdl__Exact_USCOREType));
    if (!jsdl || !jdesc || !ident || !app || !res || !hosts || !tcpu || !exact) {
        return soap_receiver_fault(s, MEM_ALLOC, NULL);
    }
    memset(jsdl, 0, sizeof(struct jsdl__JobDefinition_USCOREType));
    memset(jdesc, 0, sizeof(struct jsdl__JobDescription_USCOREType));
    memset(ident, 0, sizeof(struct jsdl__JobIdentification_USCOREType));
    memset(app, 0, sizeof(struct jsdl__Application_USCOREType));
    memset(res, 0, sizeof(struct jsdl__Resources_USCOREType));
    memset(hosts, 0, sizeof(struct jsdl__CandidateHosts_USCOREType));
    memset(tcpu, 0, sizeof(struct jsdl__RangeValue_USCOREType));
    memset(exact, 0, sizeof(struct jsdl__Exact_USCOREType));

    ident->jsdl__JobName = soap_strdup(s, job->jobname);
    ident->__sizeJobProject = 1;
    ident->jsdl__JobProject = (char**)soap_malloc(s, sizeof(char*));
    cp = soap_strdup(s, job->jobproject);
    if (!ident->jsdl__JobName || !ident->jsdl__JobProject || !cp) {
        return soap_receiver_fault(s, MEM_ALLOC, NULL);
    }
    ident->jsdl__JobProject[0] = cp;
    jdesc->jsdl__JobIdentification = ident;

    if (job->num_hostnames) {
        hosts->__sizeHostName = job->num_hostnames;
        hosts->jsdl__HostName = (char**)soap_malloc(s, sizeof(char*)
                *job->num_hostnames);
        if (!hosts->jsdl__HostName) {
            return soap_receiver_fault(s, MEM_ALLOC, NULL);
        }
        for (i = 0; i < job->num_hostnames; i++) {
            hosts->jsdl__HostName[i] = soap_strdup(s, job->hostnames[i]);
            if (hosts->jsdl__HostName[i] == NULL) {
                return soap_receiver_fault(s, MEM_ALLOC, NULL);
            }
        }
        res->jsdl__CandidateHosts = hosts;
        exact->__item = job->num_hostnames;
    } else {
        exact->__item = 0;
    }
    tcpu->Exact = exact;
    tcpu->__sizeExact = 1;
    res->jsdl__TotalCPUCount = tcpu;
    if (job->exclusive) {
        res->jsdl__ExclusiveExecution = (enum xsd__boolean*)soap_malloc(s,
                sizeof(enum xsd__boolean));
        if (!res->jsdl__ExclusiveExecution) {
            return soap_receiver_fault(s, MEM_ALLOC, NULL);
        }
        *res->jsdl__ExclusiveExecution = true_;
    }
    jdesc->jsdl__Resources = res;

    app->__any = getHPCProfileApplication(s, job);
    if (app->__any == NULL) {
        return soap_receiver_fault(s, MEM_ALLOC, NULL);
    }
    jdesc->jsdl__Application = app;

    jsdl->jsdl__JobDescription = jdesc;

    *jsdl_return = jsdl;
    return SOAP_OK;
}

