/* ----------------------------------------------------------------
 * rm_lsf.c
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
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>

#include <lsf/lsf.h>
#include <lsf/lsbatch.h>

#include "rm.h"

int
rm_initialize(struct soap *s, char *servername)
{
    if (lsb_init("besserver")) {
        lsb_perror("rm_initialize: lsb_init");
        return -1;
    }
    return 0;
}

static int
createJobWrapperScript(struct jobcard *jc, char *osuser, char *scriptname, int namelen)
{
    static char fname[] = "createJobWrapperScript";
    char wrappername[MAXPATHLEN];
    FILE *wrapper;
    int fd, i;
    struct passwd *pw;
    uid_t service_uid;
    struct fileStage *file;
    char *cp;
    
    if (!jc || !osuser || !scriptname) {
        return BESE_OTHER;
    }
    
    if ((pw = getpwnam(osuser)) == NULL) {
        fprintf(stderr, "%s: couldn't get user %s from passwd\n", fname, osuser);
        return BESE_BAD_ARG;
    }
    
    service_uid = geteuid();
    
    if (seteuid(0)) {
        perror("createJobWrapperScript: seteuid 0 (1)");
        return BESE_SYS_ERR;
    }

    if (seteuid(pw->pw_uid)) {
        perror("createJobWrapperScript: seteuid user");
        return BESE_SYS_ERR;
    }

    if (strlen(pw->pw_dir) + strlen("/jobscript.XXXXXX") + 1 > MAXPATHLEN) {
        fprintf(stderr, "%s: cannot generate wrapper script: path too long\n", fname);
        return BESE_SYS_ERR;
    }

    sprintf(wrappername, "%s/jobscript.XXXXXX", pw->pw_dir);
    fd = mkstemp(wrappername);
    if (fd == -1) {
        perror("createJobWrapperScript: mkstemp");
        return BESE_SYS_ERR;
    }
    
    if (fchmod(fd, S_IRWXU)) {
        perror("createJobWrapperScript: fchmod");
        unlink(wrappername);
        return BESE_SYS_ERR;
    }
    
    wrapper = fdopen(fd, "w");
    if (wrapper == NULL) {
        perror("createJobWrapperScript: fdopen");
        unlink(wrappername);
        return BESE_SYS_ERR;
    }
    
    fprintf(wrapper, "#!/bin/sh\n");

    file = jc->files;
    while (file) {
        if (file->source) {
            cp = strstr(file->source, "ftp://");
            if (!cp) {
                fprintf(stderr, "%s: malformed source URI for file transfer %s\n", fname, file->source);
                fprintf(stderr, "%s: only support ftp URIs at this time\n", fname);
                unlink(wrappername);
                return BESE_OTHER;
            }
            cp = cp + strlen("ftp://");
            
            fprintf(wrapper, "%s -o %s ftp://", FTP_PROGRAM, file->filename);
            if (file->credential) {
                fprintf(wrapper, "%s:%s@", file->credential->username, file->credential->password);
            }
            fprintf(wrapper, "%s\n", cp);
        }
        file = file->next;
    }

    if (jc->executable) {
        fprintf(wrapper, "%s", jc->executable);
    }
    for (i = 0; i < jc->num_args; i++) {
        fprintf(wrapper, " %s", jc->args[i]);
    }
    fprintf(wrapper, "\n");
    
    fprintf(wrapper, "rc=$?\n");
    
    file = jc->files;
    while (file) {
        if (file->target) {
            cp = strstr(file->target, "ftp://");
            if (!cp) {
                fprintf(stderr, "%s: malformed target URI for file transfer %s\n", fname, file->target);
                fprintf(stderr, "%s: only support ftp URIs at this time\n", fname);
                unlink(wrappername);
                return BESE_OTHER;
            }
            cp = cp + strlen("ftp://");
            
            fprintf(wrapper, "%s -u ftp://", FTP_PROGRAM);
            if (file->credential) {
                fprintf(wrapper, "%s:%s@", file->credential->username, file->credential->password);
            }
            fprintf(wrapper, "%s %s\n", cp, file->filename);
        }
        file = file->next;
    }

    fprintf(wrapper, "rm $0\n");

    fprintf(wrapper, "exit $rc\n");
    
    fclose(wrapper);
    
    if (seteuid(0)) {
        perror("createJobWrapperScript: seteuid 0 (2)");
        unlink(wrappername);
        return BESE_SYS_ERR;
    }

    if (seteuid(service_uid)) {
        perror("createJobWrapperScript: seteuid service_uid");
        unlink(wrappername);
        return BESE_SYS_ERR;
    }

    strncpy(scriptname, wrappername, namelen-1);
    
    return BESE_OK;
}

static int
runBsubScriptAsUser(char *scriptname, char *user)
{
    static char fname[] = "runBsubScriptAsUser";
    FILE *fp;
    pid_t pid;
    int pfd[2], jobid = 0;
    struct passwd *pw;
    char *arg0, buf[512];
    
    if (!scriptname || !user) {
        return -1;
    }
    
    if ((pw = getpwnam(user)) == NULL) {
        fprintf(stderr, "%s: couldn't get user %s from passwd\n", fname, user);
        return -1;
    }

    if (pipe(pfd) < 0) {
        perror("runBsubScriptAsUser: pipe");
        return -1;
    }
    
    if ((pid = fork()) < 0) {
        perror("runBsubScriptAsUser: fork");
        return -1;
    }
    
    if (pid == 0) {
        /* child process */
        close(pfd[0]);
        if (pfd[1] != STDOUT_FILENO) {
            if (dup2(pfd[1], STDOUT_FILENO) != STDOUT_FILENO) {
                perror("runBsubScriptAsUser (child): dup2");
                _exit(1);
            }
            close(pfd[1]);
        }
        if (seteuid(0)) {
            perror("runBsubScriptAsUser (child): seteuid 0");
            _exit(1);
        }
        if (setgid(pw->pw_gid)) {
            perror("runBsubScriptAsUser (child): setgid");
            _exit(1);
        }
        if (setuid(pw->pw_uid)) {
            perror("runBsubScriptAsUser (child): setuid");
            _exit(1);
        }
        arg0 = strrchr(scriptname, '/');
        if (arg0) arg0++;
        execl(scriptname, arg0, NULL);
        perror("runBsubScriptAsUser (child): execl");
        _exit(1);
    }
            
    /* In the parent */
    close(pfd[1]);
    fp = fdopen(pfd[0], "r");
    if (fp == NULL) {
        perror("runBsubScriptAsUser: fdopen");
    }
    while (fgets(buf, 512, fp)) {
        sscanf(buf, "Job <%d> is submitted to default queue <%*s>.\n", &jobid);
    }
    fclose(fp);
    
    if (waitpid(pid, NULL, 0) < 0) {
        perror("runBsubScriptAsUser: waitpid");
        return -1;
    }
    
    return jobid;
}

int
rm_submitJob(struct soap *s, struct jobcard *jc, 
             char *osuser, char **return_jobid)
{
    static char fname[] = "rm_submitJob";
    char scriptname[MAXPATHLEN], wrappername[MAXPATHLEN];
    char buf[512];
    int fd, i, rc, jobid = 0, rr = 0;
    FILE *script;
    struct envvar *cur;

    fprintf(stderr, "In rm_submitJob...\n");

    if (!jc || !jc->executable) {
        fprintf(stderr, "%s: Need to have the executable name\n", fname);
        return BESE_OTHER;
    }
    if (!osuser) {
        fprintf(stderr, "%s: Need to have the os user\n", fname);
        return BESE_OTHER;
    }

    strcpy(scriptname, "/tmp/besserver.XXXXXX");
    fd = mkstemp(scriptname);
    if (fd == -1) {
        perror("rm_submitJob: mkstemp");
        return BESE_OTHER;
    }
    script = fdopen(fd, "w");
    if (script == NULL) {
        perror("rm_submitJob: fdopen");
        return BESE_OTHER;
    }

    fprintf(script, "#!/bin/sh\n");
    if (jc->wd)
        fprintf(script, "LSB_JOB_LONG_CWD=%s; export LSB_JOB_LONG_CWD\n",
                jc->wd);
    for (cur = jc->environment; cur; cur = cur->next) {
        fprintf(script, "%s=%s; export %s\n", cur->name, cur->val, cur->name);
    }
    fprintf(script, "bsub ");
    if (jc->appname) {
        fprintf(script, "-a %s ", jc->appname);
        if (LSF_VERSION >= 17) {
            fprintf(script, "-app %s ", jc->appname);
        }
    }
    if (jc->jobname)
        fprintf(script, "-J %s ", jc->jobname);
    if (jc->jobproject)
        fprintf(script, "-P %s ", jc->jobproject);
    if (jc->num_hostnames) {
        fprintf(script, "-m \"");
        for (i = 0; i < jc->num_hostnames; i++)
            fprintf(script, "%s ", jc->hostnames[i]);
        fprintf(script, "\" ");
    }
    if (jc->exclusive)
        fprintf(script, "-x ");
    if (jc->tcpu)
        fprintf(script, "-n %d ", jc->tcpu);
    if (jc->input)
        fprintf(script, "-i %s ", jc->input);
    if (jc->output)
        fprintf(script, "-o %s ", jc->output);
    if (jc->error)
        fprintf(script, "-e %s ", jc->error);
    if (jc->osname) {
        if (!rr) {
            fprintf(script, "-R \"");
            rr = 1;
        } else {
            fprintf(script, " && ");
        }
        fprintf(script, "osname == %s", jc->osname);
    }
    if (jc->osver) {
        if (!rr) {
            fprintf(script, "-R \"");
            rr = 1;
        } else {
            fprintf(script, " && ");
        }
        fprintf(script, "osver == %s", jc->osver);
    }
    if (jc->cpuarch) {
        if (!rr) {
            fprintf(script, "-R \"");
            rr = 1;
        } else {
            fprintf(script, " && ");
        }
        fprintf(script, "cpuarch == %s", jc->cpuarch);
    }
    if (rr) {
        fprintf(script, "\" ");
    }
    
    if ((rc = createJobWrapperScript(jc, osuser, wrappername, MAXPATHLEN)) != BESE_OK) {
        fclose(script);
        unlink(scriptname);
        return rc;
    }
    
    fprintf(script, "%s\n", wrappername);

    fclose(script);

    if (chmod(scriptname, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)) {
        perror("submitLSFJob: chmod");
        unlink(wrappername);
        unlink(scriptname);
        return BESE_OTHER;
    }

    jobid = runBsubScriptAsUser(scriptname, osuser);
    
    unlink(scriptname);

    if (jobid < 1) {
        return BESE_OTHER;
    }

    sprintf(buf, "%ld", jobid);
    *return_jobid = soap_strdup(s, buf);
    if (!*return_jobid) {
        return BESE_MEM_ALLOC;
    }
    
    return BESE_OK;
}

int
rm_terminateJob(struct soap *s, char *job_id, char *osuser)
{
    static char fname[] = "rm_terminateJob";
    struct passwd *pw;
    pid_t pid;
    int pfd[2], lsb_rc = LSBE_NO_ERROR;
    LS_LONG_INT jobid;
    
    if (!job_id) {
        fprintf(stderr, "%s: missing job id\n", fname);
        return BESE_BAD_ARG;
    }
    jobid = strtol(job_id, NULL, 0);
    if (errno == ERANGE || errno == EINVAL) {
        fprintf(stderr, "%s: job id should be an integer\n", fname);
        return BESE_BAD_ARG;
    }

    if (!osuser) {
        fprintf(stderr, "%s: need to provide os user\n", fname);
        return BESE_BAD_ARG;
    }
    
    if ((pw = getpwnam(osuser)) == NULL) {
        fprintf(stderr, "%s: couldn't get user %s from passwd\n", fname, osuser);
        return BESE_SYS_ERR;
    }

    if (pipe(pfd) < 0) {
        perror("rm_terminateJob: pipe");
        return BESE_SYS_ERR;
    }
    
    if ((pid = fork()) < 0) {
        perror("rm_terminateJob: fork");
        return BESE_SYS_ERR;
    }
    
    if (pid == 0) {
        /* child process */
        close(pfd[0]);

        if (seteuid(0)) {
            perror("rm_terminateJob (child): seteuid 0");
            _exit(1);
        }
        if (setgid(pw->pw_gid)) {
            perror("rm_terminateJob (child): setgid");
            _exit(1);
        }
        if (setuid(pw->pw_uid)) {
            perror("rm_terminateJob (child): setuid");
            _exit(1);
        }

        if (lsb_signaljob(jobid, SIGKILL)) {
            lsb_perror("rm_terminateJob (child)");
            lsb_rc = lsberrno;
        }
        
        if (write(pfd[1], (void*)&lsb_rc, sizeof(lsb_rc)) != sizeof(lsb_rc)) {
            perror("rm_terminateJob (child): write");
            _exit(1);
        }
        _exit(0);
    }
            
    /* In the parent */
    close(pfd[1]);
    if (read(pfd[0], (void*)&lsb_rc, sizeof(lsb_rc)) != sizeof(lsb_rc)) {
        perror("rm_terminateJob: read");
        lsb_rc = -1;
    }
    close(pfd[0]);
    
    if (waitpid(pid, NULL, 0) < 0) {
        perror("rm_terminateJob: waitpid");
        return BESE_SYS_ERR;
    }
    
    switch (lsb_rc) {
    case -1:
        return BESE_SYS_ERR;
        break;
    case LSBE_NO_ERROR:
        return BESE_OK;
        break;
    case LSBE_PERMISSION:
        return BESE_PERMISSION;
        break;
    case LSBE_NO_JOB:
        return BESE_NO_ACTIVITY;
        break;
    default:
        return BESE_BACKEND;
    }     
}

int
rm_getJobStatus(struct soap *s, char *job_id, char *osuser,
                struct bes__ActivityStatusType **job_status)
{
    struct jobInfoEnt *job;
    struct bes__ActivityStatusType *status;
    int rc;
    LS_LONG_INT jobid;
    
    if (!job_id || !job_status) {
        return BESE_BAD_ARG;
    }
    jobid = strtol(job_id, NULL, 0);
    if (errno == ERANGE || errno == EINVAL) {
        fprintf(stderr, "rm_getJobStatus: job id should be an integer\n");
        return BESE_BAD_ARG;
    }

    rc = lsb_openjobinfo(jobid, NULL, "all", NULL, NULL, ALL_JOB);
    if (rc == -1) {
        if (lsberrno == LSBE_NO_JOB) {
            return BESE_NO_ACTIVITY;
        }
        lsb_perror("rm_getJobStatus: lsb_openjobinfo");
        return BESE_BACKEND;
    }
    
    job = lsb_readjobinfo(NULL);
    if (job == NULL) {
        lsb_perror("rm_getJobStatus: lsb_readjobinfo");
        lsb_closejobinfo();
        return BESE_BACKEND;
    }
    
    status = (struct bes__ActivityStatusType*)soap_malloc(s, sizeof(struct bes__ActivityStatusType));
    if (status == NULL) {
        return BESE_MEM_ALLOC;
    }
    memset(status, 0, sizeof(struct bes__ActivityStatusType));

    if (IS_PEND(job->status)) {
        status->state = Pending;
    }
    else if (IS_START(job->status)) {
        status->state = Running;
    }
    else if (IS_FINISH(job->status)) {
        status->state = Finished;
    }

    lsb_closejobinfo();

    *job_status = status;

    return BESE_OK;
}

int
rm_getJobInfo(struct soap *s, char *job_id, char *osuser, struct jobcard **job_info)
{
    struct jobInfoEnt *job;
    struct jobcard *jc;
    char *command, *cp, **cpp;
    int rc, i;
    LS_LONG_INT jobid;
    
    if (!job_id || !job_info) {
        return BESE_BAD_ARG;
    }
    jobid = strtol(job_id, NULL, 0);
    if (errno == ERANGE || errno == EINVAL) {
        fprintf(stderr, "rm_getJobInfo: job id should be an integer\n");
        return BESE_BAD_ARG;
    }

    rc = lsb_openjobinfo(jobid, NULL, "all", NULL, NULL, ALL_JOB);
    if (rc == -1) {
        if (lsberrno == LSBE_NO_JOB) {
            return BESE_NO_ACTIVITY;
        }
        lsb_perror("rm_getJobInfo: lsb_openjobinfo");
        return BESE_BACKEND;
    }
    
    job = lsb_readjobinfo(NULL);
    if (job == NULL) {
        lsb_perror("rm_getJobInfo: lsb_readjobinfo");
        lsb_closejobinfo();
        return BESE_BACKEND;
    }
    
    jc = (struct jobcard*)soap_malloc(s, sizeof(struct jobcard));
    if (jc == NULL) {
        return BESE_MEM_ALLOC;
    }
    memset(jc, 0, sizeof(struct jobcard));

    if (job->submit.jobName) {
        jc->jobname = soap_strdup(s, job->submit.jobName);
    }
    if (job->submit.projectName) {
        jc->jobproject = soap_strdup(s, job->submit.projectName);
    }
    if (job->numExHosts) {
        jc->num_hostnames = job->numExHosts;
        jc->hostnames = (char**)soap_malloc(s, sizeof(char*)*job->numExHosts);
        if (!jc->hostnames) {
            return BESE_MEM_ALLOC;
        }
        for (i = 0; i < job->numExHosts; i++) {
            jc->hostnames[i] = soap_strdup(s, job->exHosts[i]);
        }
        jc->tcpu = job->numExHosts;
    }
    if (job->submit.options & SUB_EXCLUSIVE) {
        jc->exclusive = 1;
    }
    if (job->submit.inFile) {
        jc->input = soap_strdup(s, job->submit.inFile);
    }
    if (job->submit.outFile) {
        jc->output = soap_strdup(s, job->submit.outFile);
    }
    if (job->submit.errFile) {
        jc->error = soap_strdup(s, job->submit.errFile);
    }
    if (job->execCwd) {
        jc->wd = soap_strdup(s, job->execCwd);
    }
    if (job->execUsername) {
        jc->username = soap_strdup(s, job->execUsername);
    }

    /* first pass copies the command and counts arguments */
    command = soap_strdup(s, job->submit.command);
    if (!command) {
        return BESE_MEM_ALLOC;
    }
    cpp = &command;
    cp = strsep(cpp, " \t");
    jc->executable = soap_strdup(s, cp);
    if (!jc->executable) {
        return BESE_MEM_ALLOC;
    }
    while (cp = strsep(cpp, " \t")) {
        jc->num_args++;
    }
    
    /* this second pass copies the arguments */
    if (jc->num_args) {
        jc->args = (char**)soap_malloc(s, sizeof(char*)*jc->num_args);
        if (!jc->args) {
            return BESE_MEM_ALLOC;
        }
        command = soap_strdup(s, job->submit.command);
        if (!command) {
            return BESE_MEM_ALLOC;
        }
        cpp = &command;
        cp = strsep(cpp, " \t");
        i = 0;
        while (cp = strsep(cpp, " \t")) {
            jc->args[i++] = soap_strdup(s, cp);
        }
    }
    
    lsb_closejobinfo();

    *job_info = jc;

    return BESE_OK;
}

int
rm_getResourceList(struct soap *s, struct rm_filter *filter, 
                   struct rm_resource **resourcelist, int *num_resources)
{
    struct rm_resource *res, *end, *reslist;
    struct hostInfo *hinfo;
    char *cpuarch, *osname, *osver;
    int numhosts = 0, num_res = 0, i;

    hinfo = ls_gethostinfo(NULL, &numhosts, NULL, 0, 0);
    if (hinfo == NULL) {
        fprintf(stderr, "rm_getResourceList: ls_gethostinfo error: %s\n", ls_sysmsg());
        return BESE_BACKEND;
    }

    reslist = end = NULL;
    for (i = 0; i < numhosts; i++) {
        
        if (hinfo[i].maxCpus == 0) {
            /* host unavailable at this time */
            continue;
        }
        
        res = (struct rm_resource*)soap_malloc(s, sizeof(struct rm_resource));
        if (res == NULL) {
            return BESE_MEM_ALLOC;
        }
        memset(res, 0, sizeof(struct rm_resource));
        
        res->ResourceName = soap_strdup(s, hinfo[i].hostName);
        if (res->ResourceName == NULL) {
            return BESE_MEM_ALLOC;
        }
        
        res->CPUCount = (double*)soap_malloc(s, sizeof(double));
        if (res->CPUCount == NULL) {
            return BESE_MEM_ALLOC;
        }
        *res->CPUCount = (double)hinfo[i].maxCpus;

        res->PhysicalMemory = (double*)soap_malloc(s, sizeof(double));
        if (res->PhysicalMemory == NULL) {
            return BESE_MEM_ALLOC;
        }
        *res->PhysicalMemory = 1024.0*1024.0*(double)hinfo[i].maxMem;

        res->VirtualMemory = (double*)soap_malloc(s, sizeof(double));
        if (res->VirtualMemory == NULL) {
            return BESE_MEM_ALLOC;
        }
        *res->VirtualMemory = 1024.0*1024.0*(double)hinfo[i].maxSwap;

        if (reslist == NULL) reslist = res;
        if (end) end->next = res;
        end = res;
        num_res++;
    }
    
    if (resourcelist) *resourcelist = reslist;
    if (num_resources) *num_resources = num_res;
    return BESE_OK;
}

int 
rm_getJobList(struct soap *s, struct rm_filter *filter, struct rm_job **joblist, int *numjobs)
{
    struct rm_job *job, *end, *jlist;
    struct jobInfoEnt *jinfo;
    char jobid[128];
    int num_jobs, i;

    num_jobs = lsb_openjobinfo(0, NULL, "all", NULL, NULL, ALL_JOB);
    if (num_jobs == -1) {
        if (lsberrno != LSBE_NO_JOB) {
            fprintf(stderr, "rm_getJobList: lsb_openjobinfo: %s\n", lsb_sysmsg());
            return BESE_BACKEND;
        }
        else {
            num_jobs = 0;
            jlist = NULL;
        }
    }
    jlist = end = NULL;
    for (i = 0; i < num_jobs; i++) {
        jinfo = lsb_readjobinfo(NULL);
        if (jinfo == NULL) {
            fprintf(stderr, "rm_getJobList: lsb_readjobinfo: %s\n", lsb_sysmsg());
            lsb_closejobinfo();
            return BESE_BACKEND;
        }
        job = (struct rm_job*)soap_malloc(s, sizeof(struct rm_job));
        if (job == NULL) {
            lsb_closejobinfo();
            return BESE_MEM_ALLOC;
        }
        sprintf(jobid, "%ld", jinfo->jobId);
        job->jobid = soap_strdup(s, jobid);
        if (job->jobid == NULL) {
            lsb_closejobinfo();
            return BESE_MEM_ALLOC;
        }
        if (jlist == NULL) jlist = job;
        if (end) end->next = job;
        end = job;
    }
    lsb_closejobinfo();

    if (joblist) *joblist = jlist;
    if (numjobs) *numjobs = num_jobs;
    return BESE_OK;
}

int
rm_getClusterInfo(struct soap *s, struct rm_clusterInfo **clusterinfo)
{
    struct rm_clusterInfo *cinfo;
    char *clustername;

    if (!clusterinfo) {
        return BESE_BAD_ARG;
    }

    cinfo = (struct rm_clusterInfo*)soap_malloc(s, sizeof(struct rm_clusterInfo));
    if (cinfo == NULL) {
        return BESE_MEM_ALLOC;
    }
    memset(cinfo, 0, sizeof(struct rm_clusterInfo));

    cinfo->IsAcceptingNewActivities = true_;

    clustername = ls_getclustername();
    if (clustername == NULL) {
        fprintf(stderr, "rm_getClusterInfo: ls_getclustername: %s\n", ls_sysmsg());
        return BESE_BACKEND;
    }
    cinfo->CommonName = soap_strdup(s, clustername);
    if (cinfo->CommonName == NULL) {
        return BESE_MEM_ALLOC;
    }

    cinfo->LocalResourceManagerType = (char*)soap_malloc(s,
            strlen("http://www.platform.com/bes/2006/08/resourcemanager/LSF")
            + strlen(LSF_CURRENT_VERSION)+1);
    if (cinfo->LocalResourceManagerType == NULL) {
        return BESE_MEM_ALLOC;
    }
    sprintf(cinfo->LocalResourceManagerType, 
            "http://www.platform.com/bes/2006/08/resourcemanager/LSF%s",
            LSF_CURRENT_VERSION);

    *clusterinfo = cinfo;
    return BESE_OK;
}
