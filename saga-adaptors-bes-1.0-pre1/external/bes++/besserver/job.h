/* ----------------------------------------------------------------
 * job.h
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
#ifndef _JOB_H
#define _JOB_H

#include "soapH.h"

#include "namespaces.h"
#include "faults.h"

#define FTP_PROGRAM "/usr/bin/ftp"

struct envvar
{
    struct envvar *next;
    char *name;
    char *val;
};

struct credential 
{
    char *username;
    char *password;
};

struct fileStage
{
    struct fileStage *next;
    char *filename;
    char *filesystemname;
    enum jsdl__CreationFlagEnumeration creationflag;
    enum xsd__boolean del;
    char *source;
    char *target;
    struct credential *credential;
};

struct jobcard
{
    char *jobname;
    char *jobproject;

    int num_hostnames;
    char **hostnames;
    int exclusive;
    char *osname;
    char *osver;
    char *cpuarch;
    int icpu;
    int ipmem;
    int ivmem;
    int tcpu;
    int tres;

    char *appname;
    char *appversion;

    char *executable;
    int num_args;
    char **args;
    char *input;
    char *output;
    char *error;
    char *wd;
    struct envvar *environment;
    char *username;
    
    struct fileStage *files;
};

int processJobDefinition(struct soap*, struct soap_dom_element*, struct jobcard*);
int getJSDLFromJobInfo(struct soap*, struct jobcard*, struct jsdl__JobDefinition_USCOREType**);

#endif /* _JOB_H */
