/* ----------------------------------------------------------------
 * jsdl.c
 *
 * Copyright (C) 2006-2009, Platform Computing Corporation. All Rights Reserved.
 *
 *
 * Client library of the OGSA Basic Execution Services
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
#include <stdlib.h>
#include <string.h>
#include "jsdl.hpp"
#include "bes.hpp"
#include "namespaces.h"
#include "soapStub.h"


char *jsdl_operating_system_names[] = {
    "Unknown", "MACOS", "ATTUNIX", "DGUX", "DECNT", "Tru64_UNIX", "OpenVMS",
    "HPUX", "AIX", "MVS", "OS400", "OS_2", "JavaVM", "MSDOS", "WIN3x", "WIN95",
    "WIN98", "WINNT", "WINCE", "NCR3000", "NetWare", "OSF", "DC_OS", 
    "Reliant_UNIX", "SCO_UnixWare", "SCO_OpenServer", "Sequent", "IRIX", 
    "Solaris", "SunOS", "U6000", "ASERIES", "TandemNSK", "TandemNT", 
    "BS2000", "LINUX", "Lynx", "XENIX", "VM", "Interactive_UNIX", "BSDUNIX", 
    "FreeBSD", "NetBSD", "GNU_Hurd", "OS9", "MACH_Kernel", "Inferno", "QNX", 
    "EPOC", "IxWorks", "VxWorks", "MiNT", "BeOS", "HP_MPE", "NextStep", 
    "PalmPilot", "Rhapsody", "Windows_2000", "Dedicated", "OS_390", "VSE", 
    "TPF", "Windows_R_Me", "Caldera_Open_UNIX", "OpenBSD", "Not_Applicable", 
    "Windows_XP", "z_OS", "other", NULL,
};

char *jsdl_processor_architectures[] = {
    "sparc", "powerpc", "x86", "x86_32", "x86_64",
    "parisc", "mips", "ia64", "arm", "other", NULL,
};


int 
jsdl_addArg(struct jsdl_job_definition *jsdl, char *arg)
{
    struct jsdl_hpcp_application *app;
    char *cp, **cpp;
    int i;

    if (!jsdl || !arg) {
        return BESE_BAD_ARG;
    }

    app = (struct jsdl_hpcp_application*)jsdl->Application;

    cp = strdup(arg);
    if (cp == NULL) {
        return BESE_MEM_ALLOC;
    }

    cpp = (char**)malloc(sizeof(char*)*(app->num_args+1));
    if (cpp == NULL) {
        free(cp);
        return BESE_MEM_ALLOC;
    }

    for (i = 0; i < app->num_args; i++)
        cpp[i] = app->Argument[i];
    cpp[i] = cp;

    if (app->Argument) {
        free(app->Argument);
    }

    app->Argument = cpp;
    app->num_args++;

    return BESE_OK;
}

int 
jsdl_addEnv(struct jsdl_job_definition *jsdl, struct soap_dom_element *dom)
{
    struct jsdl_hpcp_application *app;
    struct jsdl_envvar *newvar, *cur;
    struct soap_dom_attribute *attr;

    if (!jsdl || !dom) {
        return BESE_BAD_ARG;
    }

    app = (struct jsdl_hpcp_application*)jsdl->Application;

    newvar = (struct jsdl_envvar*)malloc(sizeof(struct jsdl_envvar));
    if (newvar == NULL) {
        return BESE_MEM_ALLOC;
    }

    newvar->val = strdup(dom->data);
    if (newvar->val == NULL) {
        free(newvar);
        return BESE_MEM_ALLOC;
    }

    newvar->name = NULL;
    for (attr = dom->atts; attr; attr = attr->next) {
        if (!strcmp(attr->name, "name")) {
            newvar->name = strdup(attr->data);
            if (newvar->name == NULL) {
                free(newvar->val);
                free(newvar);
                return BESE_MEM_ALLOC;
            }
        }
    }
    if (newvar->name == NULL) {
        free(newvar->val);
        free(newvar);
        return BESE_XML_FORMAT;
    }

    cur = app->Environment;
    while (cur->next) {
        cur = cur->next;
    }
    cur->next = newvar;

    return BESE_OK;
}

int 
jsdl_addHost(struct jsdl_job_definition *jsdl, char *host)
{
    char *cp, **cpp;
    int i;

    if (!jsdl || !host) {
        return BESE_BAD_ARG;
    }

    cp = strdup(host);
    if (cp == NULL) {
        return BESE_MEM_ALLOC;
    }

    cpp = (char**)malloc(sizeof(char*)*(jsdl->num_hosts+1));
    if (cpp == NULL) {
        free(cp);
        return BESE_MEM_ALLOC;
    }

    for (i = 0; i < jsdl->num_hosts; i++)
        cpp[i] = jsdl->HostName[i];
    cpp[i] = cp;

    if (jsdl->HostName) {
        free(jsdl->HostName);
    }

    jsdl->HostName = cpp;
    jsdl->num_hosts++;

    return BESE_OK;
}

int
jsdl_processCredential(struct soap_dom_element *dom,
                       struct hpcp_credential **cred)
{
    struct soap_dom_element *cur = dom->elts;
    struct soap_dom_element *user, *pass;
    struct hpcp_credential *new_cred = NULL;

    while (cur) {
        if (isElement(cur, WSSE_NS, "UsernameToken")) {
            user = cur->elts;
            if (!isElement(user, WSSE_NS, "Username")) {
                return BESE_XML_FORMAT;
            }
            pass = user->next;
            if (!isElement(pass, WSSE_NS, "Password")) {
                return BESE_XML_FORMAT;
            }
            new_cred = (struct hpcp_credential*)malloc(sizeof(struct hpcp_credential));
            if (new_cred == NULL) {
                return BESE_MEM_ALLOC;
            }
            new_cred->username = strdup(user->data);
            if (new_cred->username == NULL) {
                free(new_cred);
                return BESE_MEM_ALLOC;
            }
            new_cred->password = strdup(pass->data);
            if (!new_cred->password) {
                free(new_cred->username);
                free(new_cred);
                return BESE_MEM_ALLOC;
            }
            *cred = new_cred;
        }
        else {
            return BESE_UNSUPPORTED;
        }
        cur = cur->next;
    }
    return BESE_OK;
}

void
jsdl_freeHPCProfileApplication(struct jsdl_hpcp_application *app)
{
    struct jsdl_envvar *cur, *next;
    int i;

    if (!app) return;
    if (app->Executable) free(app->Executable);
    if (app->Argument) {
        for (i = 0; i < app->num_args; i++) {
            if (app->Argument[i]) free(app->Argument[i]);
        }
        free(app->Argument);
    }
    if (app->Input) free(app->Input);
    if (app->Output) free(app->Output);
    if (app->Error) free(app->Error);
    if (app->WorkingDirectory) free(app->WorkingDirectory);
    if (app->UserName) free(app->UserName);
    cur = app->Environment;
    while (cur) {
        if (cur->name) free(cur->name);
        if (cur->val) free(cur->val);
        next = cur;
        free(cur);
        cur = next;
    }
    free(app);

    return;
}

int 
jsdl_processHPCProfileApplication(struct soap_dom_element *dom,
                                  struct jsdl_job_definition *jsdl)
{
    struct jsdl_hpcp_application *app;
    struct soap_dom_element *cur = dom->elts;
    int rc;

    if (!dom || !jsdl) {
        return BESE_BAD_ARG;
    }

    app = (struct jsdl_hpcp_application*)malloc(sizeof(struct jsdl_hpcp_application));
    if (app == NULL) {
        return BESE_MEM_ALLOC;
    }
    memset(app, 0, sizeof(struct jsdl_hpcp_application));

    while (cur) {
        if (isElement(cur, JSDL_HPCPA_NS, "Executable")) {
            if (cur->data && strlen(cur->data)) {
                app->Executable = strdup(cur->data);
                if (app->Executable == NULL) {
                    jsdl_freeHPCProfileApplication(app);
                    return BESE_MEM_ALLOC;
                }
            } 
            else {
                jsdl_freeHPCProfileApplication(app);
                return BESE_XML_FORMAT;
            }
        } 
        else if (isElement(cur, JSDL_HPCPA_NS, "Argument")) {
            if ((rc = jsdl_addArg(jsdl, cur->data)) != BESE_OK) {
                jsdl_freeHPCProfileApplication(app);
                return rc;
            }
        } 
        else if (isElement(cur, JSDL_HPCPA_NS, "Input")) {
            if (cur->data && strlen(cur->data)) {
                app->Input = strdup(cur->data);
                if (app->Input == NULL) {
                    jsdl_freeHPCProfileApplication(app);
                    return BESE_MEM_ALLOC;
                }
            }
            else {
                jsdl_freeHPCProfileApplication(app);
                return BESE_XML_FORMAT;
            }
        } 
        else if (isElement(cur, JSDL_HPCPA_NS, "Output")) {
            if (cur->data && strlen(cur->data)) {
                app->Output = strdup(cur->data);
                if (app->Output == NULL) {
                    jsdl_freeHPCProfileApplication(app);
                    return BESE_MEM_ALLOC;
                }
            }
            else {
                jsdl_freeHPCProfileApplication(app);
                return BESE_XML_FORMAT;
            }
        } 
        else if (isElement(cur, JSDL_HPCPA_NS, "Error")) {
            if (cur->data && strlen(cur->data)) {
                app->Error = strdup(cur->data);
                if (app->Error == NULL) {
                    jsdl_freeHPCProfileApplication(app);
                    return BESE_MEM_ALLOC;
                }
            }
            else {
                jsdl_freeHPCProfileApplication(app);
                return BESE_XML_FORMAT;
            }
        } 
        else if (isElement(cur, JSDL_HPCPA_NS, "WorkingDirectory")) {
            if (cur->data && strlen(cur->data)) {
                app->WorkingDirectory = strdup(cur->data);
                if (app->WorkingDirectory == NULL) {
                    jsdl_freeHPCProfileApplication(app);
                    return BESE_MEM_ALLOC;
                }
            }
            else {
                jsdl_freeHPCProfileApplication(app);
                return BESE_XML_FORMAT;
            }
        } 
        else if (isElement(cur, JSDL_HPCPA_NS, "Environment")) {
            if ((rc = jsdl_addEnv(jsdl, cur)) != BESE_OK) {
                jsdl_freeHPCProfileApplication(app);
                return rc;
            }
        } 
        else if (isElement(cur, JSDL_HPCPA_NS, "UserName")) {
            if (cur->data && strlen(cur->data)) {
                app->UserName = strdup(cur->data);
                if (app->UserName == NULL) {
                    jsdl_freeHPCProfileApplication(app);
                    return BESE_MEM_ALLOC;
                }
            }
            else {
                jsdl_freeHPCProfileApplication(app);
                return BESE_XML_FORMAT;
            }
        }
        cur = cur->next;
    }

    jsdl->ApplicationType = JSDL_HPC_PROFILE_APPLICATION;
    jsdl->Application = (void*)app;

    return BESE_OK;
}

int 
jsdl_processJobIdentification(struct soap_dom_element *dom,
                              struct jsdl_job_definition *jsdl)
{
    struct soap_dom_element *cur = dom->elts;
    int rc;

    while (cur) {
        if (isElement(cur, JSDL_NS, "JobName")) {
            jsdl->JobName = strdup(cur->data);
            if (jsdl->JobName == NULL) {
                return BESE_MEM_ALLOC;
            }
        } 
        else if (isElement(cur, JSDL_NS, "Description")) {
            ;
        } 
        else if (isElement(cur, JSDL_NS, "JobAnnotation")) {
            jsdl->JobAnnotation = strdup(cur->data);
            if (jsdl->JobAnnotation == NULL) {
                return BESE_MEM_ALLOC;
            }
        } 
        else if (isElement(cur, JSDL_NS, "JobProject")) {
            jsdl->JobProject = strdup(cur->data);
            if (jsdl->JobProject == NULL) {
                return BESE_MEM_ALLOC;
            }
        }
        cur = cur->next;
    }

    return BESE_OK;
}

int 
jsdl_processApplication(struct soap_dom_element *dom,
                        struct jsdl_job_definition *jsdl)
{
    struct soap_dom_element *cur = dom->elts;
    int rc;


    while (cur) {
        if (isElement(cur, JSDL_NS, "ApplicationName")) {
            jsdl->ApplicationName = strdup(cur->data);
            if (jsdl->ApplicationName == NULL) {
                return BESE_MEM_ALLOC;
            }
        } 
        else if (isElement(cur, JSDL_NS, "ApplicationVersion")) {
            jsdl->ApplicationVersion = strdup(cur->data);
            if (jsdl->ApplicationVersion == NULL) {
                return BESE_MEM_ALLOC;
            }
        } 
        else if (isElement(cur, JSDL_HPCPA_NS, "HPCProfileApplication")) {
            if ((rc = jsdl_processHPCProfileApplication(cur, jsdl)) != BESE_OK) {
                return rc;
            }
        }
        cur = cur->next;
    }

    return BESE_OK;
}

int
jsdl_newRangeValue(struct jsdl_range_value **rangeval)
{
    struct jsdl_range_value *rval;

    if (rangeval == NULL) {
        return BESE_BAD_ARG;
    }

    rval = (struct jsdl_range_value*)malloc(sizeof(struct jsdl_range_value));
    if (rval == NULL) {
        return BESE_MEM_ALLOC;
    }
    memset(rval, 0, sizeof(struct jsdl_range_value));

    *rangeval = rval;

    return BESE_OK;
}

int
jsdl_addUpperBound(struct jsdl_range_value *rval, double value, int exclusive)
{
    struct jsdl_bound *upper;

    if (rval == NULL) {
        return BESE_BAD_ARG;
    }

    upper = (struct jsdl_bound*)malloc(sizeof(struct jsdl_bound));
    if (upper == NULL) {
        return BESE_MEM_ALLOC;
    }
    upper->value = value;
    upper->exclusive = exclusive;

    if (rval->UpperBoundedRange) {
        free(rval->UpperBoundedRange);
    }
    
    rval->UpperBoundedRange = upper;

    return BESE_OK;
}

int
jsdl_addLowerBound(struct jsdl_range_value *rval, double value, int exclusive)
{
    struct jsdl_bound *lower;

    if (rval == NULL) {
        return BESE_BAD_ARG;
    }

    lower = (struct jsdl_bound*)malloc(sizeof(struct jsdl_bound));
    if (lower == NULL) {
        return BESE_MEM_ALLOC;
    }
    lower->value = value;
    lower->exclusive = exclusive;

    if (rval->LowerBoundedRange) {
        free(rval->LowerBoundedRange);
    }
    
    rval->LowerBoundedRange = lower;

    return BESE_OK;
}

int 
jsdl_addExact(struct jsdl_range_value *rval, double value, double epsilon)
{
    struct jsdl_exact *exact, *cur;
    
    if (rval == NULL) {
        return BESE_BAD_ARG;
    }

    exact = (struct jsdl_exact*)malloc(sizeof(struct jsdl_exact));
    if (exact == NULL) {
        return BESE_MEM_ALLOC;
    }
    exact->next = NULL;
    exact->value = value;
    exact->epsilon = epsilon;

    if (cur = rval->Exact) {
        while (cur->next) {
            cur = cur->next;
        }
        cur->next = exact;
    }
    else {
        rval->Exact = exact;
    }

    return BESE_OK;
}

int 
jsdl_addRange(struct jsdl_range_value *rval, 
              double lowerBound, 
              int lowerExclusive,
              double upperBound,
              int upperExclusive)
{
    struct jsdl_range *range, *cur;
    struct jsdl_bound *lower, *upper;
    
    if (rval == NULL) {
        return BESE_BAD_ARG;
    }

    range = (struct jsdl_range*)malloc(sizeof(struct jsdl_range));
    lower = (struct jsdl_bound*)malloc(sizeof(struct jsdl_bound));
    upper = (struct jsdl_bound*)malloc(sizeof(struct jsdl_bound));
    if (range == NULL || lower == NULL || upper == NULL) {
        return BESE_MEM_ALLOC;
    }
    
    lower->value = lowerBound;
    lower->exclusive = lowerExclusive;
    upper->value = upperBound;
    upper->exclusive = upperExclusive;
    range->LowerBound = lower;
    range->UpperBound = upper;
    range->next = NULL;
    if (cur = rval->Range) {
        while (cur->next) {
            cur = cur->next;
        }
        cur->next = range;
    }
    else {
        rval->Range = range;
    }

    return BESE_OK;
}

void
jsdl_freeRangeValue(struct jsdl_range_value *value)
{
    struct jsdl_exact *exact, *next_exact;
    struct jsdl_range *range, *next_range;

    if (value == NULL)
        return;
   
    if (value->UpperBoundedRange) free(value->UpperBoundedRange);

    if (value->LowerBoundedRange) free(value->LowerBoundedRange);
    
    exact = value->Exact;
    while (exact) {
        next_exact = exact->next;
        free(exact);
        exact = next_exact;
    }

    range = value->Range;
    while (range) {
        next_range = range->next;
        free(range);
        range = next_range;
    }

    free(value);

    return;
}

int
jsdl_processRange(struct soap_dom_element *dom,
                  struct jsdl_range **range)
{
    struct jsdl_range *r;
    struct jsdl_bound *lower, *upper;
    struct soap_dom_element *cur;
    struct soap_dom_attribute *attr;

    if (!dom || !range) {
        return BESE_BAD_ARG;
    }
    
    r = (struct jsdl_range*)malloc(sizeof(struct jsdl_range));
    if (r == NULL) {
        return BESE_MEM_ALLOC;
    }
    memset(r, 0, sizeof(struct jsdl_range));

    cur = dom->elts;

    if (!isElement(cur, JSDL_NS, "LowerBound")) {
        free(r);
        return BESE_XML_FORMAT;
    }

    lower = (struct jsdl_bound*)malloc(sizeof(struct jsdl_bound));
    if (lower == NULL) {
        free(r);
        return BESE_MEM_ALLOC;
    }
    lower->value = strtod(cur->data, NULL);
    lower->exclusive = 0;
    for (attr = cur->atts; attr; attr = attr->next) {
        if (!strcmp(attr->name, "exclusiveBound")) {
            if (!strcmp(attr->data, "true")) {
                lower->exclusive = 1;
            }
        }
    }

    cur = cur->next;

    if (!isElement(cur, JSDL_NS, "UpperBound")) {
        free(r);
        free(lower);
        return BESE_XML_FORMAT;
    }
    
    upper = (struct jsdl_bound*)malloc(sizeof(struct jsdl_bound));
    if (upper == NULL) {
        free(r);
        free(lower);
        return BESE_MEM_ALLOC;
    }
    upper->value = strtod(cur->data, NULL);
    upper->exclusive = 0;
    for (attr = cur->atts; attr; attr = attr->next) {
        if (!strcmp(attr->name, "exclusiveBound")) {
            if (!strcmp(attr->data, "true")) {
                upper->exclusive = 1;
            }
        }
    }

    r->next = NULL;
    r->LowerBound = lower;
    r->UpperBound = upper;
    *range = r;

    return BESE_OK;
}

int 
jsdl_processRangeValue(struct soap_dom_element *dom, 
                       struct jsdl_range_value **value)
{
    struct jsdl_range_value *val;
    struct jsdl_exact *new_exact, *exact = NULL;
    struct jsdl_range *new_range, *range = NULL;
    struct jsdl_bound *bound;
    struct soap_dom_element *cur;
    struct soap_dom_attribute *attr;
    char *endptr;
    int rc;

    if (!dom || !value) {
        return BESE_BAD_ARG;
    }

    val = (struct jsdl_range_value*)malloc(sizeof(struct jsdl_range_value));
    if (val == NULL) {
        return BESE_MEM_ALLOC;
    }
    memset(val, 0, sizeof(struct jsdl_range_value));

    cur = dom->elts;

    while (cur) {
        if (isElement(cur, JSDL_NS, "UpperBoundedRange")) {
            bound = (struct jsdl_bound*)malloc(sizeof(struct jsdl_bound));
            if (bound == NULL) {
                jsdl_freeRangeValue(val);
                return BESE_MEM_ALLOC;
            }
            bound->value = strtod(cur->data, NULL);
            for (attr = cur->atts; attr; attr = attr->next) {
                if (!strcmp(attr->name, "exclusiveBound")) {
                    if (!strcmp(attr->data, "true")) {
                        bound->exclusive = 1;
                    }
                }
            }
            val->UpperBoundedRange = bound;
        }
        else if (isElement(cur, JSDL_NS, "LowerBoundedRange")) {
            bound = (struct jsdl_bound*)malloc(sizeof(struct jsdl_bound));
            if (bound == NULL) {
                jsdl_freeRangeValue(val);
                return BESE_MEM_ALLOC;
            }
            bound->value = strtod(cur->data, NULL);
            for (attr = cur->atts; attr; attr = attr->next) {
                if (!strcmp(attr->name, "exclusiveBound")) {
                    if (!strcmp(attr->data, "true")) {
                        bound->exclusive = 1;
                    }
                }
            }
            val->LowerBoundedRange = bound;
        }
        else if (isElement(cur, JSDL_NS, "Exact")) {
            new_exact = (struct jsdl_exact*)malloc(sizeof(struct jsdl_exact));
            if (new_exact == NULL) {
                jsdl_freeRangeValue(val);
                return BESE_MEM_ALLOC;
            }
            new_exact->next = NULL;
            new_exact->value = strtod(cur->data, NULL);
            new_exact->epsilon = 0.0;
            for (attr = cur->atts; attr; attr = attr->next) {
                if (!strcmp(attr->name, "epsilon")) {
                    new_exact->epsilon = strtod(attr->data, NULL);
                    break;
                }
            }
            if (exact) {
                exact->next = new_exact;
            }
            else {
                val->Exact = new_exact;
            }
            exact = new_exact;
        }
        else if (isElement(cur, JSDL_NS, "Range")) {
            if (rc = jsdl_processRange(cur, &new_range)) {
                jsdl_freeRangeValue(val);
                return rc;
            }
            if (range) {
                range->next = new_range;
            }
            else {
                val->Range = new_range;
            }
            range = new_range;
        }

        cur = cur->next;
    }

    *value = val;

    return BESE_OK;
}

int 
jsdl_processCandidateHosts(struct soap_dom_element *dom,
                           struct jsdl_job_definition *jsdl)
{
    struct soap_dom_element *cur = dom->elts;
    int rc;

    while (cur) {
        if (isElement(cur, JSDL_NS, "HostName")) {
            if ((rc = jsdl_addHost(jsdl, cur->data)) != BESE_OK) {
                return rc;
            }
        }
        cur = cur->next;
    }

    return BESE_OK;
}

void
jsdl_freeFileSystem(struct jsdl_file_system *fs)
{
    if (fs == NULL)
        return;

    if (fs->name) free(fs->name);
    if (fs->MountPoint) free(fs->MountPoint);
    if (fs->DiskSpace) jsdl_freeRangeValue(fs->DiskSpace);

    free(fs);

}

int
jsdl_processFileSystem(struct soap_dom_element *dom,
                       struct jsdl_file_system *jsdl)
{
    return BESE_OK;
}

int 
jsdl_processOperatingSystem(struct soap_dom_element *dom,
                            struct jsdl_job_definition *jsdl)
{
    struct jsdl_operating_system *os;
    struct soap_dom_element *cur = dom->elts;
    int i, rc;

    if (!dom || !jsdl) {
        return BESE_BAD_ARG;
    }

    os = (struct jsdl_operating_system*)malloc(sizeof(struct jsdl_operating_system));
    if (os == NULL) {
        return BESE_MEM_ALLOC;
    }
    memset(os, 0, sizeof(struct jsdl_operating_system));

    while (cur) {
        if (isElement(cur, JSDL_NS, "OperatingSystemType")) {
            if (isElement(cur->elts, JSDL_NS, "OperatingSystemName")) {
                for (i = 0; jsdl_operating_system_names[i]; i++) {
                    if (!strcmp(jsdl_operating_system_names[i], cur->elts->data)) {
                        os->OperatingSystemName = strdup(cur->elts->data);
                        if (os->OperatingSystemName == NULL) {
                            free(os);
                            return BESE_MEM_ALLOC;
                        }
                    }
                }
                if (os->OperatingSystemName == NULL) {
                    free(os);
                    return BESE_XML_FORMAT;
                }
            }
            else {
                return BESE_XML_FORMAT;
            }
        } 
        else if (isElement(cur, JSDL_NS, "OperatingSystemVersion")) {
            os->OperatingSystemVersion = strdup(cur->data);
            if (os->OperatingSystemVersion == NULL) {
                if (os->OperatingSystemName) free(os->OperatingSystemName);
                free(os);
                return BESE_MEM_ALLOC;
            }
        } 
        else if (isElement(cur, JSDL_NS, "Description")) {
            ;
        }
        cur = cur->next;
    }

    jsdl->OperatingSystemType = os;

    return BESE_OK;
}

int 
jsdl_processCPUArchitecture(struct soap_dom_element *dom,
                            struct jsdl_job_definition *jsdl)
{
    struct jsdl_cpu_architecture *arch;
    struct soap_dom_element *cur = dom->elts;
    int i, rc;

    if (!dom || !jsdl) {
        return BESE_BAD_ARG;
    }

    arch = (struct jsdl_cpu_architecture*)malloc(sizeof(struct jsdl_cpu_architecture));
    if (arch == NULL) {
        return BESE_MEM_ALLOC;
    }
    memset(arch, 0, sizeof(struct jsdl_cpu_architecture));

    while (cur) {
        if (isElement(cur, JSDL_NS, "CPUArchitectureName")) {
            for (i = 0; jsdl_processor_architectures[i]; i++) {
                if (!strcmp(jsdl_processor_architectures[i], cur->data)) {
                    arch->CPUArchitectureName = strdup(cur->data);
                    if (arch->CPUArchitectureName == NULL) {
                        free(arch);
                        return BESE_MEM_ALLOC;
                    }
                }
            }
            if (arch->CPUArchitectureName == NULL) {
                free(arch);
                return BESE_XML_FORMAT;
            }
        }
        cur = cur->next;
    }

    jsdl->CPUArchitecture = arch;

    return BESE_OK;
}

int 
jsdl_processResources(struct soap_dom_element *dom,
                      struct jsdl_job_definition *jsdl)
{
    struct soap_dom_element *cur = dom->elts;
    int rc;

    while (cur) {
        if (isElement(cur, JSDL_NS, "CandidateHosts")) {
            if ((rc = jsdl_processCandidateHosts(cur, jsdl)) != BESE_OK) {
                return rc;
            }
        } 
        else if (isElement(cur, JSDL_NS, "ExclusiveExecution")) {
            if (!strcmp(cur->data, "true")) {
                jsdl->ExclusiveExecution = 1;
            }
        } 
        else if (isElement(cur, JSDL_NS, "OperatingSystem")) {
            if ((rc = jsdl_processOperatingSystem(cur, jsdl)) != BESE_OK) {
                return rc;
            }
        } 
        else if (isElement(cur, JSDL_NS, "CPUArchitecture")) {
            if ((rc = jsdl_processCPUArchitecture(cur, jsdl)) != BESE_OK) {
                return rc;
            }
        } 
        else if (isElement(cur, JSDL_NS, "IndividualCPUCount")) {
            if ((rc = jsdl_processRangeValue(cur, &jsdl->IndividualCPUCount)) != BESE_OK) {
                return rc;
            }
        } 
        else if (isElement(cur, JSDL_NS, "IndividualPhysicalMemory")) {
            if ((rc = jsdl_processRangeValue(cur, &jsdl->IndividualPhysicalMemory)) != BESE_OK) {
                return rc;
            }
        } 
        else if (isElement(cur, JSDL_NS, "IndividualVirtualMemory")) {
            if ((rc = jsdl_processRangeValue(cur, &jsdl->IndividualVirtualMemory)) != BESE_OK) {
                return rc;
            }
        } 
        else if (isElement(cur, JSDL_NS, "TotalCPUCount")) {
            if ((rc = jsdl_processRangeValue(cur, &jsdl->TotalCPUCount)) != BESE_OK) {
                return rc;
            }
        } 
        else if (isElement(cur, JSDL_NS, "TotalResourceCount")) {
            if ((rc = jsdl_processRangeValue(cur, &jsdl->TotalResourceCount)) != BESE_OK) {
                return rc;
            }
        }
        cur = cur->next;
    }

    return BESE_OK;
}

void
jsdl_freeDataStaging(struct jsdl_data_staging *file)
{
    if (file == NULL)
        return;
    if (file->name) free(file->name);
    if (file->FileName) free(file->FileName);
    if (file->FileSystemName) free(file->FileSystemName);
    if (file->SourceURI) free(file->SourceURI);
    if (file->TargetURI) free(file->TargetURI);
    if (file->Credential) {
        if (file->Credential->username) free(file->Credential->username);
        if (file->Credential->password) free(file->Credential->password);
        if (file->Credential->raw_token) free(file->Credential->raw_token);
        free(file->Credential);
    }
    if (file->next) jsdl_freeDataStaging(file->next);
    free(file);
}

int 
jsdl_processDataStaging(struct soap_dom_element *dom,
                        struct jsdl_job_definition *jsdl)
{
    struct soap_dom_element *cur = dom->elts;
    struct jsdl_data_staging *file, *cur_file;
    struct hpcp_credential *cred;
    int rc;
    
    file = (struct jsdl_data_staging *)malloc(sizeof(struct jsdl_data_staging));
    if (file == NULL) {
        return BESE_MEM_ALLOC;
    }
    memset(file, 0, sizeof(struct jsdl_data_staging));
    
    
    while (cur) {
        if (isElement(cur, JSDL_NS, "FileName")) {
            file->FileName = strdup(cur->data);
            if (file->FileName == NULL) {	
                jsdl_freeDataStaging(file);
                return BESE_MEM_ALLOC;
            }
        }
        else if (isElement(cur, JSDL_NS, "FileSystemName")) {
            file->FileSystemName = strdup(cur->data);
            if (file->FileSystemName == NULL) {	
                jsdl_freeDataStaging(file);
                return BESE_MEM_ALLOC;
            }
        }
        else if (isElement(cur, JSDL_NS, "CreationFlag")) {
            if (!strcmp(cur->data, "overwrite")) {
                file->CreationFlag = jsdl_overwrite;
            }
            else if (!strcmp(cur->data, "append")) {
                file->CreationFlag = jsdl_append;
            }
            else if (!strcmp(cur->data, "dontOverwrite")) {
                file->CreationFlag = jsdl_dontOverwrite;
            }
        }
        else if (isElement(cur, JSDL_NS, "DeleteOnTermination")) {
            if (!strcmp(cur->data, "true")) {
                file->DeleteOnTermination = 1;
            }
        }
        else if (isElement(cur, JSDL_NS, "Source")) {
            if (isElement(cur->elts, JSDL_NS, "URI")) {
                file->SourceURI = strdup(cur->elts->data);
                if (file->SourceURI == NULL) {
                    jsdl_freeDataStaging(file);
                    return BESE_MEM_ALLOC;
                }
            }
            else {
                jsdl_freeDataStaging(file);
                return BESE_XML_FORMAT;
            }
        }
        else if (isElement(cur, JSDL_NS, "Target")) {
            if (isElement(cur->elts, JSDL_NS, "URI")) {
                file->TargetURI = strdup(cur->elts->data);
                if (file->TargetURI == NULL) {
                    jsdl_freeDataStaging(file);
                    return BESE_MEM_ALLOC;
                }
            }
            else {
                jsdl_freeDataStaging(file);
                return BESE_XML_FORMAT;
            }
        }
        else if (isElement(cur, HPCP_AC_NS, "Credential")) {
            cred = NULL;
            if ((rc = jsdl_processCredential(cur, &cred)) != BESE_OK) {
                jsdl_freeDataStaging(file);
                return rc;
            }
            file->Credential = cred;
        }
        
        cur = cur->next;
    }

    cur_file = jsdl->DataStaging;
    if (cur_file) {
        while (cur_file->next) {
            cur_file = cur_file->next;
        }
        cur_file->next = file;
    }
    else {
        jsdl->DataStaging = file;
    }
    
    return BESE_OK;
}

int 
jsdl_processJobDescription(struct soap_dom_element *dom, 
                           struct jsdl_job_definition *jsdl)
{
    struct soap_dom_element *cur = dom->elts;
    int rc;

    while (cur) {
        if (isElement(cur, JSDL_NS, "JobIdentification")) {
            if ((rc = jsdl_processJobIdentification(cur, jsdl)) != BESE_OK) {
                return rc;
            }
        } else if (isElement(cur, JSDL_NS, "Application")) {
            if ((rc = jsdl_processApplication(cur, jsdl)) != BESE_OK) {
                return rc;
            }
        } else if (isElement(cur, JSDL_NS, "Resources")) {
            if ((rc = jsdl_processResources(cur, jsdl)) != BESE_OK) {
                return rc;
            }
        } else if (isElement(cur, JSDL_NS, "DataStaging")) {
            if ((rc = jsdl_processDataStaging(cur, jsdl)) != BESE_OK) {
                return rc;
            }
        }
        cur = cur->next;
    }

    return BESE_OK;
}

int 
jsdl_processJobDefinition(struct soap_dom_element *dom,
                          struct jsdl_job_definition **jsdl)
{
    struct soap_dom_element *cur = dom->elts;
    struct jsdl_job_definition *jd;
    int rc;

    if (jsdl == NULL || dom == NULL) {
        return BESE_BAD_ARG;
    }

    jd = (struct jsdl_job_definition*)malloc(sizeof(struct jsdl_job_definition));
    if (jd == NULL) {
        return BESE_MEM_ALLOC;
    }
    memset(jd, 0, sizeof(struct jsdl_job_definition));

    if (isElement(cur, JSDL_NS, "JobDescription")) {
        rc = jsdl_processJobDescription(cur, jd);
        if (rc != BESE_OK) {
            jsdl_freeJobDefinition(jd);
            return rc;
        }
        *jsdl = jd;
        return BESE_OK;
    } else {
        return BESE_XML_FORMAT;
    }
}

void
jsdl_freeJobDefinition(struct jsdl_job_definition *jsdl)
{
    struct jsdl_data_staging *file, *next_file;
    int i;

    if (!jsdl) return;

    if (jsdl->JobName) free(jsdl->JobName);
    if (jsdl->JobAnnotation) free(jsdl->JobAnnotation);
    if (jsdl->JobProject) free(jsdl->JobProject);

    if (jsdl->ApplicationName) free(jsdl->ApplicationName);
    if (jsdl->ApplicationVersion) free(jsdl->ApplicationVersion);
    if (jsdl->Application) {
        if (jsdl->ApplicationType == JSDL_HPC_PROFILE_APPLICATION) 
            jsdl_freeHPCProfileApplication((struct jsdl_hpcp_application*)jsdl->Application);
    }

    if (jsdl->HostName) {
        for (i = 0; i < jsdl->num_hosts; i++) 
            if (jsdl->HostName[i]) free(jsdl->HostName[i]);
        free(jsdl->HostName);
    }
    jsdl_freeFileSystem(jsdl->FileSystem);
    if (jsdl->OperatingSystemType) {
        if (jsdl->OperatingSystemType->OperatingSystemName)
            free(jsdl->OperatingSystemType->OperatingSystemName);
        if (jsdl->OperatingSystemType->other) 
            free(jsdl->OperatingSystemType->other);
        if (jsdl->OperatingSystemType->OperatingSystemVersion) 
            free(jsdl->OperatingSystemType->OperatingSystemVersion);
        free(jsdl->OperatingSystemType);
    }
    if (jsdl->CPUArchitecture) {
        if (jsdl->CPUArchitecture->CPUArchitectureName)
            free(jsdl->CPUArchitecture->CPUArchitectureName);
        if (jsdl->CPUArchitecture->other) 
            free(jsdl->CPUArchitecture->other);
        free(jsdl->CPUArchitecture);
    }
    jsdl_freeRangeValue(jsdl->IndividualCPUSpeed);
    jsdl_freeRangeValue(jsdl->IndividualCPUTime);
    jsdl_freeRangeValue(jsdl->IndividualCPUCount);
    jsdl_freeRangeValue(jsdl->IndividualNetworkBandwidth);
    jsdl_freeRangeValue(jsdl->IndividualPhysicalMemory);
    jsdl_freeRangeValue(jsdl->IndividualVirtualMemory);
    jsdl_freeRangeValue(jsdl->IndividualDiskSpace);
    jsdl_freeRangeValue(jsdl->TotalCPUTime);
    jsdl_freeRangeValue(jsdl->TotalCPUCount);
    jsdl_freeRangeValue(jsdl->TotalPhysicalMemory);
    jsdl_freeRangeValue(jsdl->TotalVirtualMemory);
    jsdl_freeRangeValue(jsdl->TotalDiskSpace);
    jsdl_freeRangeValue(jsdl->TotalResourceCount);

    jsdl_freeDataStaging(jsdl->DataStaging);

    free(jsdl);
}

int
jsdl_newJobDefinition(enum jsdl_application_type app_type, struct jsdl_job_definition **jsdl)
{
    struct jsdl_job_definition *jd;
    struct jsdl_posix_application *posix;
    struct jsdl_hpcp_application *hpcp;
    void *app;
   
    if (jsdl == NULL) {
        return BESE_BAD_ARG;
    }

    switch (app_type) {

    case JSDL_POSIX_APPLICATION:
        posix = (struct jsdl_posix_application*)malloc(sizeof(struct jsdl_posix_application));
        if (posix == NULL) {
            return BESE_MEM_ALLOC;
        }
        memset(posix, 0, sizeof(struct jsdl_posix_application));
        app = (void*)posix;
        break;

    case JSDL_HPC_PROFILE_APPLICATION:
        hpcp = (struct jsdl_hpcp_application*)malloc(sizeof(struct jsdl_hpcp_application));
        if (hpcp == NULL) {
            return BESE_MEM_ALLOC;
        }
        memset(hpcp, 0, sizeof(struct jsdl_hpcp_application));
        app = (void*)hpcp;
        break;

    default:
        return BESE_BAD_ARG;;
    }

    jd = (struct jsdl_job_definition*)malloc(sizeof(struct jsdl_job_definition));
    if (jd == NULL) {
        free(app);
        return BESE_MEM_ALLOC;
    }
    memset(jd, 0, sizeof(struct jsdl_job_definition));

    jd->Application = app;
    jd->ApplicationType = app_type;

    *jsdl = jd;

    return BESE_OK;
}

/*
 * Functions below here are used to turn a jsdl_job_definition
 * into a gSOAP DOM tree
 */
struct soap_dom_element *
jsdl_generateDomElement(struct soap *s, char *nstr, char *name)
{
    struct soap_dom_element *dom;
    
    dom = (struct soap_dom_element*)soap_malloc(s, sizeof(struct soap_dom_element));
    if (dom == NULL) {
        return NULL;
    }
    memset(dom, 0, sizeof(struct soap_dom_element));
    dom->nstr = soap_strdup(s, nstr);
    dom->name = soap_strdup(s, name);
    if (!dom->nstr || !dom->name) {
        return NULL;
    }

    return dom;
}

struct soap_dom_attribute *
jsdl_generateDomAttribute(struct soap *s, char *name)
{
    struct soap_dom_attribute *attr;

    attr = (struct soap_dom_attribute*)soap_malloc(s, sizeof(struct soap_dom_attribute));
    if (attr == NULL) {
        return NULL;
    }
    memset(attr, 0, sizeof(struct soap_dom_attribute));
    attr->name = soap_strdup(s, name);
    if (attr->name == NULL) {
        return NULL;
    }

    return attr;
}

struct soap_dom_element *
jsdl_generateOperatingSystem(struct soap *s, struct jsdl_operating_system *jsdl_os)
{
    struct soap_dom_element *os, *ostype, *osname, *osver;
    
    if (jsdl_os == NULL) {
        return NULL;
    }
    
    os = jsdl_generateDomElement(s, JSDL_NS, "OperatingSystem");
    if (os == NULL) {
        return NULL;
    }
    
    osname = NULL;
    if (jsdl_os->OperatingSystemName) {
        ostype = jsdl_generateDomElement(s, JSDL_NS, "OperatingSystemType");
        if (ostype == NULL) {
            return NULL;
        }
        ostype->prnt = os;
        osname = jsdl_generateDomElement(s, JSDL_NS, "OperatingSystemName");
        if (osname == NULL) {
            return NULL;
        }
        osname->prnt = ostype;
        osname->data = soap_strdup(s, jsdl_os->OperatingSystemName);
        if (osname->data == NULL) {
            return NULL;
        }
        ostype->elts = osname;
    }
    
    if (jsdl_os->OperatingSystemVersion) {
        osver = jsdl_generateDomElement(s, JSDL_NS, "OperatingSystemVersion");
        if (osver == NULL) {
            return NULL;
        }
        osver->prnt = os;
        osver->data = soap_strdup(s, jsdl_os->OperatingSystemVersion);
        if (osver->data == NULL) {
            return NULL;
        }
        if (ostype) {
            ostype->next = osver;
        }
        else {
            os->elts = osver;
        }
    }
    
    return os;
}

struct soap_dom_element *
jsdl_generateCPUArchitecture(struct soap *s, struct jsdl_cpu_architecture *arch)
{
    struct soap_dom_element *cpuname, *cpu;
    
    if (arch == NULL) {
        return NULL;
    }
    
    cpu = jsdl_generateDomElement(s, JSDL_NS, "CPUArchitecture");
    if (cpu == NULL) {
        return NULL;
    }
    
    if (arch->CPUArchitectureName) {
        cpuname = jsdl_generateDomElement(s, JSDL_NS, "CPUArchitectureName");
        if (cpuname == NULL) {
            return NULL;
        }
        cpuname->prnt = cpu;
        cpuname->data = soap_strdup(s, arch->CPUArchitectureName);
        if (cpuname->data == NULL) {
            return NULL;
        }
        cpu->elts = cpuname;
    }
    
    return cpu;
}

struct soap_dom_element *
jsdl_generateRangeValue(struct soap *s, struct jsdl_range_value *rangeval, char *name)
{
    struct soap_dom_element *rval, *first, *cur, *dom;
    struct jsdl_exact *exact;
    struct jsdl_range *range;
    char buf[512];

    if (rangeval == NULL || name == NULL) {
        return NULL;
    }
    
    rval = jsdl_generateDomElement(s, JSDL_NS, name);
    if (rval == NULL) {
        return NULL;
    }

    first = cur = NULL;

    if (rangeval->UpperBoundedRange) {
        dom = jsdl_generateDomElement(s, JSDL_NS, "UpperBoundedRange");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = rval;
        sprintf(buf, "%f", rangeval->UpperBoundedRange->value);
        dom->data = soap_strdup(s, buf);
        if (dom->data == NULL) {
            return NULL;
        }
        if (rangeval->UpperBoundedRange->exclusive) {
            dom->atts = jsdl_generateDomAttribute(s, "exclusiveBound");
            if (dom->atts == NULL) {
                return NULL;
            }
            dom->atts->data = soap_strdup(s, "true");
            if (dom->atts->data == NULL) {
                return NULL;
            }
        }
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }
        
    if (rangeval->LowerBoundedRange) {
        dom = jsdl_generateDomElement(s, JSDL_NS, "LowerBoundedRange");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = rval;
        sprintf(buf, "%f", rangeval->LowerBoundedRange->value);
        dom->data = soap_strdup(s, buf);
        if (dom->data == NULL) {
            return NULL;
        }
        if (rangeval->LowerBoundedRange->exclusive) {
            dom->atts = jsdl_generateDomAttribute(s, "exclusiveBound");
            if (dom->atts == NULL) {
                return NULL;
            }
            dom->atts->data = soap_strdup(s, "true");
            if (dom->atts->data == NULL) {
                return NULL;
            }
        }
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    exact = rangeval->Exact;
    while (exact) {
        dom = jsdl_generateDomElement(s, JSDL_NS, "Exact");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = rval;
        sprintf(buf, "%f", exact->value);
        dom->data = soap_strdup(s, buf);
        if (dom->data == NULL) {
            return NULL;
        }
        if (exact->epsilon) {
            dom->atts = jsdl_generateDomAttribute(s, "epsilon");
            if (dom->atts == NULL) {
                return NULL;
            }
            sprintf(buf, "%f", exact->epsilon);
            dom->atts->data = soap_strdup(s, buf);
            if (dom->atts->data == NULL) {
                return NULL;
            }
        }
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
        exact = exact->next;
    }

    range = rangeval->Range;
    while (range) {
        if (!range->LowerBound || !range->UpperBound) {
            return NULL;
        }
        dom = jsdl_generateDomElement(s, JSDL_NS, "Range");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = rval;
        dom->elts = jsdl_generateDomElement(s, JSDL_NS, "LowerBound");
        if (dom->elts == NULL) {
            return NULL;
        }
        dom->elts->prnt = dom;
        sprintf(buf, "%f", range->LowerBound->value);
        dom->elts->data = soap_strdup(s, buf);
        if (dom->elts->data == NULL) {
            return NULL;
        }
        if (range->LowerBound->exclusive) {
            dom->elts->atts = jsdl_generateDomAttribute(s, "exclusiveBound");
            if (dom->elts->atts == NULL) {
                return NULL;
            }
            dom->elts->atts->data = soap_strdup(s, "true");
            if (dom->elts->atts->data == NULL) {
                return NULL;
            }
        }
        dom->elts->next = jsdl_generateDomElement(s, JSDL_NS, "UpperBound");
        if (dom->elts->next == NULL) {
            return NULL;
        }
        dom->elts->next->prnt = dom;
        sprintf(buf, "%f", range->UpperBound->value);
        dom->elts->next->data = soap_strdup(s, buf);
        if (dom->elts->next->data == NULL) {
            return NULL;
        }
        if (range->UpperBound->exclusive) {
            dom->elts->next->atts = jsdl_generateDomAttribute(s, "exclusiveBound");
            if (dom->elts->next->atts == NULL) {
                return NULL;
            }
            dom->elts->next->atts->data = soap_strdup(s, "true");
            if (dom->elts->next->atts->data == NULL) {
                return NULL;
            }
        }
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
        range = range->next;
    }

    if (!first) {
        return NULL;
    }

    rval->elts = first;
        
    return rval;
}
    
struct soap_dom_element *
jsdl_generateFileSystem(struct soap *s, struct jsdl_file_system *fs)
{
    struct soap_dom_element *new_fs, *first, *cur, *dom;
    
    if (fs == NULL) {
        return NULL;
    }

    new_fs = jsdl_generateDomElement(s, JSDL_NS, "FileSystem");
    if (new_fs == NULL) {
        return NULL;
    }
    
    if (fs->name) {
        new_fs->atts = jsdl_generateDomAttribute(s, "name");
        if (new_fs->atts == NULL) {
            return NULL;
        }
        new_fs->atts->data = soap_strdup(s, fs->name);
        if (new_fs->atts->data == NULL) {
            return NULL;
        }
    }
    
    first = cur = NULL;

    if (fs->type) {
        dom = jsdl_generateDomElement(s, JSDL_NS, "FileSystemType");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = new_fs;
        switch (fs->type) {
        case jsdl_swap:
            dom->data = "swap";
            break;
        case jsdl_temporary:
            dom->data = "temporary";
            break;
        case jsdl_spool:
            dom->data = "spool";
            break;
        case jsdl_normal:
            dom->data = "normal";
            break;
        default:
            return NULL;
        }
        first = dom;
        cur = dom;
    }

    if (fs->MountPoint) {
        dom = jsdl_generateDomElement(s, JSDL_NS, "MountPoint");
        if (dom == NULL) {
            return NULL;
        }
        dom->data = soap_strdup(s, fs->MountPoint);
        if (dom->data == NULL) {
            return NULL;
        }
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (fs->DiskSpace) {
        dom = jsdl_generateRangeValue(s, fs->DiskSpace, "DiskSpace");
        if (dom == NULL) {
            return NULL;
        }
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    new_fs->elts = first;

    return new_fs;
}

struct soap_dom_element *
jsdl_generateCandidateHosts(struct soap *s, struct jsdl_job_definition *jd)
{
    struct soap_dom_element *cand, *first, *cur, *dom;
    int i;

    if (jd == NULL || jd->num_hosts == 0) {
        return NULL;
    }

    cand = jsdl_generateDomElement(s, JSDL_NS, "CandidateHosts");
    if (cand == NULL) {
        return NULL;
    }
    
    first = cur = NULL;

    for (i = 0; i < jd->num_hosts; i++) {
        dom = jsdl_generateDomElement(s, JSDL_NS, "HostName");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = cand;
        dom->data = soap_strdup(s, jd->HostName[i]);
        if (dom->data == NULL) {
            return NULL;
        }
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    cand->elts = first;

    return cand;
}

struct soap_dom_element * 
jsdl_generateHPCProfileApplication(struct soap *s, struct jsdl_job_definition *jd)
{
    struct soap_dom_element *hpcpa, *first, *cur, *dom;
    struct jsdl_hpcp_application *app;
    struct jsdl_envvar *envvar;
    char *nstr;
    int i;

    if (jd == NULL || jd->Application == NULL) {
        return NULL;
    }

    app = (struct jsdl_hpcp_application*)jd->Application;

    hpcpa = jsdl_generateDomElement(s, JSDL_HPCPA_NS, "HPCProfileApplication");
    if (hpcpa == NULL) {
        return NULL;
    }
    
    first = cur = NULL;

    if (app->Executable) {
        dom = jsdl_generateDomElement(s, JSDL_HPCPA_NS, "Executable");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = hpcpa;
        dom->data = soap_strdup(s, app->Executable);
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    for (i = 0; i < app->num_args; i++ ) {
        dom = jsdl_generateDomElement(s, JSDL_HPCPA_NS, "Argument");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = hpcpa;
        dom->data = soap_strdup(s, app->Argument[i]);
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (app->Input) {
        dom = jsdl_generateDomElement(s, JSDL_HPCPA_NS, "Input");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = hpcpa;
        dom->data = soap_strdup(s, app->Input);
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (app->Output) {
        dom = jsdl_generateDomElement(s, JSDL_HPCPA_NS, "Output");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = hpcpa;
        dom->data = soap_strdup(s, app->Output);
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (app->Error) {
        dom = jsdl_generateDomElement(s, JSDL_HPCPA_NS, "Error");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = hpcpa;
        dom->data = soap_strdup(s, app->Error);
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (app->WorkingDirectory) {
        dom = jsdl_generateDomElement(s, JSDL_HPCPA_NS, "WorkingDirectory");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = hpcpa;
        dom->data = soap_strdup(s, app->WorkingDirectory);
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (app->Environment) {
        envvar = app->Environment;
        while (envvar) {
            dom = jsdl_generateDomElement(s, JSDL_HPCPA_NS, "Environment");
            if (dom == NULL) {
                return NULL;
            }
            dom->prnt = hpcpa;
            dom->atts = jsdl_generateDomAttribute(s, "name");
            if (dom->atts == NULL) {
                return NULL;
            }
            dom->atts->data = soap_strdup(s, envvar->name);
            dom->data = soap_strdup(s, envvar->val);
            if (dom->atts->data == NULL || dom->data == NULL) {
                return NULL;
            }
            if (!first) {
                first = dom;
            }
            else {
                cur->next = dom;
            }
            cur = dom;
            envvar = envvar->next;
        }
    }

    if (app->UserName) {
        dom = jsdl_generateDomElement(s, JSDL_HPCPA_NS, "UserName");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = hpcpa;
        dom->data = soap_strdup(s, app->UserName);
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (first == NULL) {
        return NULL;
    }

    hpcpa->elts = first;

    return hpcpa;
}

struct soap_dom_element * 
jsdl_generatePosixApplication(struct soap *s, struct jsdl_job_definition *jd)
{
    struct soap_dom_element *posix, *first, *cur, *dom;
    struct jsdl_posix_application *app;
    struct jsdl_envvar *envvar;
    char *nstr;
    int i;

    if (jd == NULL || jd->Application == NULL) {
        return NULL;
    }

    app = (struct jsdl_posix_application*)jd->Application;

    posix = jsdl_generateDomElement(s, JSDL_POSIX_NS, "POSIXApplication");
    if (posix == NULL) {
        return NULL;
    }
    
    first = cur = NULL;

    if (app->Executable) {
        dom = jsdl_generateDomElement(s, JSDL_POSIX_NS, "Executable");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = posix;
        dom->data = soap_strdup(s, app->Executable);
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    for (i = 0; i < app->num_args; i++ ) {
        dom = jsdl_generateDomElement(s, JSDL_POSIX_NS, "Argument");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = posix;
        dom->data = soap_strdup(s, app->Argument[i]);
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (app->Input) {
        dom = jsdl_generateDomElement(s, JSDL_POSIX_NS, "Input");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = posix;
        dom->data = soap_strdup(s, app->Input);
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (app->Output) {
        dom = jsdl_generateDomElement(s, JSDL_POSIX_NS, "Output");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = posix;
        dom->data = soap_strdup(s, app->Output);
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (app->Error) {
        dom = jsdl_generateDomElement(s, JSDL_POSIX_NS, "Error");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = posix;
        dom->data = soap_strdup(s, app->Error);
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (app->WorkingDirectory) {
        dom = jsdl_generateDomElement(s, JSDL_POSIX_NS, "WorkingDirectory");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = posix;
        dom->data = soap_strdup(s, app->WorkingDirectory);
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (app->Environment) {
        envvar = app->Environment;
        while (envvar) {
            dom = jsdl_generateDomElement(s, JSDL_POSIX_NS, "Environment");
            if (dom == NULL) {
                return NULL;
            }
            dom->prnt = posix;
            dom->atts = jsdl_generateDomAttribute(s, "name");
            if (dom->atts == NULL) {
                return NULL;
            }
            dom->atts->data = soap_strdup(s, envvar->name);
            dom->data = soap_strdup(s, envvar->val);
            if (dom->atts->data == NULL || dom->data == NULL) {
                return NULL;
            }
            if (!first) {
                first = dom;
            }
            else {
                cur->next = dom;
            }
            cur = dom;
            envvar = envvar->next;
        }
    }

    if (app->UserName) {
        dom = jsdl_generateDomElement(s, JSDL_POSIX_NS, "UserName");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = posix;
        dom->data = soap_strdup(s, app->UserName);
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (app->GroupName) {
        dom = jsdl_generateDomElement(s, JSDL_POSIX_NS, "GroupName");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = posix;
        dom->data = soap_strdup(s, app->GroupName);
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (first == NULL) {
        return NULL;
    }

    posix->elts = first;

    return posix;
}

struct soap_dom_element*
jsdl_generateJobIdentification(struct soap *s, struct jsdl_job_definition *jd)
{
    struct soap_dom_element *id, *first, *cur, *dom;

    id = jsdl_generateDomElement(s, JSDL_NS, "JobIdentification");
    if (id == NULL) {
        return NULL;
    }

    cur = first = NULL;

    if (jd->JobName) {
        dom = jsdl_generateDomElement(s, JSDL_NS, "JobName");
        if (dom == NULL) {
            return NULL;
        }
        dom->data = soap_strdup(s, jd->JobName);
        dom->prnt = id;
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (jd->JobAnnotation) {
        dom = jsdl_generateDomElement(s, JSDL_NS, "JobAnnotation");
        if (dom == NULL) {
            return NULL;
        }
        dom->data = soap_strdup(s, jd->JobAnnotation);
        dom->prnt = id;
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (jd->JobProject) {
        dom = jsdl_generateDomElement(s, JSDL_NS, "JobProject");
        if (dom == NULL) {
            return NULL;
        }
        dom->data = soap_strdup(s, jd->JobProject);
        dom->prnt = id;
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (first == NULL) {
        return NULL;
    }
    id->elts = first;

    return id;
}

struct soap_dom_element*
jsdl_generateApplication(struct soap *s, struct jsdl_job_definition *jd)
{
    struct soap_dom_element *app, *first, *cur, *dom;

    app = jsdl_generateDomElement(s, JSDL_NS, "Application");
    if (app == NULL) {
        return NULL;
    }

    cur = first = NULL;

    if (jd->ApplicationName) {
        dom = jsdl_generateDomElement(s, JSDL_NS, "ApplicationName");
        if (dom == NULL) {
            return NULL;
        }
        dom->data = soap_strdup(s, jd->ApplicationName);
        dom->prnt = app;
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (jd->ApplicationVersion) {
        dom = jsdl_generateDomElement(s, JSDL_NS, "ApplicationVersion");
        if (dom == NULL) {
            return NULL;
        }
        dom->data = soap_strdup(s, jd->ApplicationVersion);
        dom->prnt = app;
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }
    
    if (jd->Application) {
        if (jd->ApplicationType == JSDL_POSIX_APPLICATION) {
            dom = jsdl_generatePosixApplication(s, jd);
        }
        else if (jd->ApplicationType == JSDL_HPC_PROFILE_APPLICATION) {
            dom = jsdl_generateHPCProfileApplication(s, jd);
        }
        else {
            return NULL;
        }
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = app;
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (first == NULL) {
        return NULL;
    }
    app->elts = first;

    return app;
}

struct soap_dom_element*
jsdl_generateResources(struct soap *s, struct jsdl_job_definition *jd)
{
    struct soap_dom_element *res, *first, *cur, *dom;
    struct jsdl_file_system *fs;

    res = jsdl_generateDomElement(s, JSDL_NS, "Resources");
    if (res == NULL) {
        return NULL;
    }

    cur = first = NULL;

    if (jd->num_hosts) {
        dom = jsdl_generateCandidateHosts(s, jd);
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = res;
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    fs = jd->FileSystem;
    while (fs) {
        dom = jsdl_generateFileSystem(s, fs);
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = res;
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
        fs = fs->next;
    }
    
    if (jd->ExclusiveExecution) {
        dom = jsdl_generateDomElement(s, JSDL_NS, "ExclusiveExecution");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = res;
        dom->data = soap_strdup(s, "true");
        if (dom->data == NULL) {
            return NULL;
        }
        if (!first) {
            first = dom;
        } 
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (jd->OperatingSystemType) {
        dom = jsdl_generateOperatingSystem(s, jd->OperatingSystemType);
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = res;
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }
                                  
    if (jd->CPUArchitecture) {
        dom = jsdl_generateCPUArchitecture(s, jd->CPUArchitecture);
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = res;
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (jd->IndividualCPUSpeed) {
        dom = jsdl_generateRangeValue(s, jd->IndividualCPUSpeed, "IndividualCPUSpeed");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = res;
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (jd->IndividualCPUTime) {
        dom = jsdl_generateRangeValue(s, jd->IndividualCPUTime, "IndividualCPUTime");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = res;
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (jd->IndividualCPUCount) {
        dom = jsdl_generateRangeValue(s, jd->IndividualCPUCount, "IndividualCPUCount");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = res;
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (jd->IndividualNetworkBandwidth) {
        dom = jsdl_generateRangeValue(s, jd->IndividualNetworkBandwidth, "IndividualNetworkBandwidth");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = res;
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (jd->IndividualPhysicalMemory) {
        dom = jsdl_generateRangeValue(s, jd->IndividualPhysicalMemory, "IndividualPhysicalMemory");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = res;
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (jd->IndividualVirtualMemory) {
        dom = jsdl_generateRangeValue(s, jd->IndividualVirtualMemory, "IndividualVirtualMemory");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = res;
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (jd->IndividualDiskSpace) {
        dom = jsdl_generateRangeValue(s, jd->IndividualDiskSpace, "IndividualDiskSpace");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = res;
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (jd->TotalCPUTime) {
        dom = jsdl_generateRangeValue(s, jd->TotalCPUTime, "TotalCPUTime");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = res;
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (jd->TotalCPUCount) {
        dom = jsdl_generateRangeValue(s, jd->TotalCPUCount, "TotalCPUCount");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = res;
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (jd->TotalPhysicalMemory) {
        dom = jsdl_generateRangeValue(s, jd->TotalPhysicalMemory, "TotalPhysicalMemory");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = res;
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (jd->TotalVirtualMemory) {
        dom = jsdl_generateRangeValue(s, jd->TotalVirtualMemory, "TotalVirtualMemory");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = res;
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (jd->TotalDiskSpace) {
        dom = jsdl_generateRangeValue(s, jd->TotalDiskSpace, "TotalDiskSpace");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = res;
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (jd->TotalResourceCount) {
        dom = jsdl_generateRangeValue(s, jd->TotalResourceCount, "TotalResourceCount");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = res;
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (first == NULL) {
        return NULL;
    }
    res->elts = first;

    return res;
}

struct soap_dom_element *
jsdl_generateHPCPCredential(struct soap *s, struct hpcp_credential *cred)
{
    struct soap_dom_element *new_cred, *token, *user, *pass;

    if (cred == NULL || cred->username == NULL) {
        return NULL;
    }
    
    new_cred = jsdl_generateDomElement(s, HPCP_AC_NS, "Credential");
    token = jsdl_generateDomElement(s, WSSE_NS, "UsernameToken");
    user = jsdl_generateDomElement(s, WSSE_NS, "Username");
    if (new_cred == NULL || token == NULL || user == NULL) {
        return NULL;
    }
    
    user->data = soap_strdup(s, cred->username);
    if (user->data == NULL) {
        return NULL;
    }

    new_cred->elts = token;
    token->prnt = new_cred;
    token->elts = user;
    user->prnt = token;
    
    if (cred->password) {
        pass = jsdl_generateDomElement(s, WSSE_NS, "Password");
        if (pass == NULL) {
            return NULL;
        }
        pass->data = soap_strdup(s, cred->password);
        if (pass->data == NULL) {
            return NULL;
        }
        pass->prnt = token;
        user->next = pass;
    }

    return new_cred;
}

struct soap_dom_element *
jsdl_generateDataStaging(struct soap *s, struct jsdl_data_staging *ds)
{
    struct soap_dom_element *data, *first, *cur, *dom;

    if (ds == NULL) {
        return NULL;
    }

    data = jsdl_generateDomElement(s, JSDL_NS, "DataStaging");
    if (data == NULL) {
        return NULL;
    }

    if (ds->name) {
        data->atts = jsdl_generateDomAttribute(s, "name");
        if (data->atts == NULL) {
            return NULL;
        }
        data->atts->data = soap_strdup(s, ds->name);
        if (data->atts->data == NULL) {
            return NULL;
        }
    }
    
    cur = first = NULL;

    if (ds->FileName) {
        dom = jsdl_generateDomElement(s, JSDL_NS, "FileName");
        if (dom == NULL) {
            return NULL;
        }
        dom->data = soap_strdup(s, ds->FileName);
        if (dom->data == NULL) {
            return NULL;
        }
        dom->prnt = data;
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (ds->FileSystemName) {
        dom = jsdl_generateDomElement(s, JSDL_NS, "FileSystemName");
        if (dom == NULL) {
            return NULL;
        }
        dom->data = soap_strdup(s, ds->FileSystemName);
        if (dom->data == NULL) {
            return NULL;
        }
        dom->prnt = data;
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (ds->CreationFlag) {
        dom = jsdl_generateDomElement(s, JSDL_NS, "CreationFlag");
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = data;
        switch (ds->CreationFlag) {
        case jsdl_overwrite:
            dom->data = "overwrite";
            break;
        case jsdl_dontOverwrite:
            dom->data = "dontOverwrite";
            break;
        case jsdl_append:
            dom->data = "append";
            break;
        default:
            return NULL;
        }
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (ds->DeleteOnTermination) {
        dom = jsdl_generateDomElement(s, JSDL_NS, "DeleteOnTermination");
        if (dom == NULL) {
            return NULL;
        }
        dom->data = soap_strdup(s, "true");
        if (dom->data == NULL) {
            return NULL;
        }
        dom->prnt = data;
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (ds->SourceURI) {
        dom = jsdl_generateDomElement(s, JSDL_NS, "SourceURI");
        if (dom == NULL) {
            return NULL;
        }
        dom->data = soap_strdup(s, ds->SourceURI);
        if (dom->data == NULL) {
            return NULL;
        }
        dom->prnt = data;
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (ds->TargetURI) {
        dom = jsdl_generateDomElement(s, JSDL_NS, "TargetURI");
        if (dom == NULL) {
            return NULL;
        }
        dom->data = soap_strdup(s, ds->TargetURI);
        if (dom->data == NULL) {
            return NULL;
        }
        dom->prnt = data;
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (ds->Credential) {
        dom = jsdl_generateHPCPCredential(s, ds->Credential);
        if (dom == NULL) {
            return NULL;
        }
        dom->prnt = data;
        if (!first) {
            first = dom;
        }
        else {
            cur->next = dom;
        }
        cur = dom;
    }

    if (first == NULL) {
        return NULL;
    }

    data->elts = first;

    return data;
}

int
jsdl_generateJobDefinitionDOM(struct jsdl_job_definition *jd, struct soap_dom_element **jsdl_dom)
{
    struct soap_dom_element *job_def, *job_desc; 
    struct soap_dom_element *id, *app, *res, *data;
    struct soap_dom_element *first, *cur;
    struct jsdl_data_staging *ds;
    struct soap *s;

    if (jd == NULL || jsdl_dom == NULL) {
        return BESE_BAD_ARG;
    }

    s = soap_new1(SOAP_DOM_TREE|SOAP_C_UTFSTRING);
    if (s == NULL) {
        return BESE_MEM_ALLOC;
    }

    job_def = jsdl_generateDomElement(s, JSDL_NS, "JobDefinition");
    if (job_def == NULL) {
        return BESE_MEM_ALLOC;
    }
    
    job_desc = jsdl_generateDomElement(s, JSDL_NS, "JobDescription");
    if (job_desc == NULL) {
        return BESE_MEM_ALLOC;
    }
    job_desc->prnt = job_def;
    
    cur = first = NULL;

    id = jsdl_generateJobIdentification(s, jd);
    if (id) {
        id->prnt = job_desc;
        first = id;
        cur = id;
    }

    app = jsdl_generateApplication(s, jd);
    if (app) {
        app->prnt = job_desc;
        if (!first) {
            first = app;
        }
        else {
            cur->next = app;
        }
        cur = app;
    }

    res = jsdl_generateResources(s, jd);
    if (res) {
        app->prnt = job_desc;
        if (!first) {
            first = res;
        }
        else {
            cur->next = res;
        }
        cur = res;
    }

    ds = jd->DataStaging;
    while (ds) {
        data = jsdl_generateDataStaging(s, ds);
        if (data) {
            data->prnt = job_desc;
            if (!first) {
                first = data;
            }
            else {
                cur->next = data;
            }
            cur = data;
        }
        ds = ds->next;
    }

    job_desc->elts = first;
    job_def->elts = job_desc;

    *jsdl_dom = job_def;

    return BESE_OK;
}

void
jsdl_freeJobDefinitionDOM(struct soap_dom_element *dom)
{
    struct soap *s;

    if (dom == NULL) {
        return;
    }
    s = dom->soap;
    soap_end(s);
    soap_free(s);
}
