/* ----------------------------------------------------------------
 * rm.h
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
#ifndef _RM_H
#define _RM_H

#include "soapH.h"
#include "job.h"

struct rm_resource {
    char *ResourceName;
    char *OperatingSystemName;
    char *OperatingSystemVersion;
    char *CPUArchitecture;
    double *CPUCount;
    double *CPUSpeed;
    double *PhysicalMemory;
    double *VirtualMemory;
    struct rm_resource *next;
};

struct rm_filter {
    char *user;
    char *state;
    long startRange;
    long endRange;
    char *startTime;
    char *endTime;
    char *CompactResources;
    struct rm_filter *next;
};

struct rm_job {
    char *jobid;
    struct rm_job *next;
};

struct rm_clusterInfo {
    enum xsd__boolean IsAcceptingNewActivities;
    char *CommonName;
    char *LongDescription;
    int num_extensions;
    char **BESExtensions;
    char *LocalResourceManagerType;
};

#ifdef CXX
extern "C" int rm_initialize(struct soap*, char*);
extern "C" int rm_submitJob(struct soap*, struct jobcard*, char*, char**);
extern "C" int rm_terminateJob(struct soap*, char*, char *);
extern "C" int rm_getJobInfo(struct soap*, char*, char*, struct jobcard**);
extern "C" int rm_getJobStatus(struct soap*, char*, char*, struct bes__ActivityStatusType**);
extern "C" int rm_getResourceList(struct soap *, struct rm_filter*, struct rm_resource**, int*);
extern "C" int rm_getJobsList(struct soap*, struct rm_filter*, struct rm_job**, int*);
extern "C" int rm_getClusterInfo(struct soap*, struct rm_clusterInfo**);
#else
int rm_initialize(struct soap*, char*);
int rm_submitJob(struct soap*, struct jobcard*, char*, char**);
int rm_terminateJob(struct soap*, char*, char *);
int rm_getJobInfo(struct soap*, char*, char*, struct jobcard**);
int rm_getJobStatus(struct soap*, char*, char*, struct bes__ActivityStatusType**);
int rm_getResourceList(struct soap *, struct rm_filter*, struct rm_resource**, int*);
int rm_getJobsList(struct soap*, struct rm_filter*, struct rm_job**, int*);
int rm_getClusterInfo(struct soap*, struct rm_clusterInfo**);
#endif


#endif /* _RM_H */

