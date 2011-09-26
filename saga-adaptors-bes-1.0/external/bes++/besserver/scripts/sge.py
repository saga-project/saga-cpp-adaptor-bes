#!/usr/bin/python

#Usage $./sge.py action parameter file.out

import pexpect
import sys
import re
import os
import stat

def submitJob(scriptFile):
    """Submits the scriptFile using qsub and returns
    the jobid"""
    submit = pexpect.spawn('qsub %s'%(scriptFile));
    output = submit.read();
    submit.close()
    pattern = re.compile('Your job \d* .* has been submitted.\r\n');
    jobid = re.compile(' \d* ');
    if not pattern.match(output):
        return "0";
    return jobid.search(output).group().strip();
    
def statusJob(jobid):
    """Gets the status of the job given and returns
    a character"""
    qstat = pexpect.spawn("qstat")
    output = qstat.read()
    qstat.close()
    jobRE = re.compile(jobid)
    for line in output.splitlines():
        job = jobRE.search(line)
        if job!= None:
            break
    if job != None:
        return 'u'
    listAttributes = job.group().split()
    #State is the fifth element
    if len(listAttributes) >= 5:
        return listAttributes[4]
    return 'u'

def deleteJob(jobid):
    """Deletes the job -jobid- from the queue.
    return 0 if ok or 1 if not."""
    qdel = pexpect.spawn('qdel %s'%(jobid));
    output = qdel.read();
    qdel.close()
    if output == '':
        #job is already registered for deletion
        return "0"
    pattern = re.compile('has registered the job \d* for deletion')
    if pattern.search(output) != None:
        return "0"
    pattern = re.compile('has deleted job \d*')
    if pattern.search(output) != None:
        return "0"
    #Jobid is not known
    return "1"

def activityDocument(jobid):
    """Returns a list of environment variables for the job with jobid.
    If the job does not exist returns "list=NULL" """
    qstat = pexpect.spawn('qstat -j %s'%(jobid));
    output = qstat.read();
    qstat.close()
    if output == '':
        #job is already registered for deletion
        return "list=NULL"
    pattern = re.compile('Following jobs do not exist:')
    if pattern.search(output) != None:
        return "list=NULL"
    pattern = re.compile('env_list:\s*\S*')
    list = pattern.search(output)
    if list != None:
        return list.group().rsplit()[1]
    else:
        return "list=NULL"

def factoryCompactDocument(fileOut):
    """Writes in fileOut the document with the factory attributes.
    Returns '0' if exited correctly."""
    resource = open(fileOut, 'w')
    
    node_info = pexpect.run("qstat -f")
    hostRE = re.compile(" \d+/\d+ ")

    total_used = 0
    total_available = 0
    for line in node_info.splitlines()[3:]:
        cpus = hostRE.search(line)
        if cpus != None:
            cpu_str = cpus.group()
            used = int(cpu_str[:cpu_str.rfind('/')])
            available = int(cpu_str[cpu_str.rfind('/')+1:]) - used
            total_used += used
            total_available += available
    resource.write("clusterA 0 " + str(total_available) + " free\n")
    resource.write("clusterU 0 " + str(total_used) + " job-busy\n")

    resource.close()
    return "0"


def factoryDocument(fileOut):
    """Writes in fileOut the document with the factory attributes.
    Returns '0' if exited correctly."""
    resource = open(fileOut, 'w')
    
    node_info = pexpect.run("qhost")

    for line in node_info.splitlines()[3:]:
        host = line.split()
        if host[5] != "-":
            state = "busy"
        else:
            state = "free"
        mem = float(host[4][:-1]) * 1024 *1024
        mem = int(mem)
        resourceInfo = host[0] + " " + str(mem)+ " " + host[2] + " " + state + "\n"
        resource.write(resourceInfo)
    
    resource.close()

    return "0"

def writeOutput(output, filename):
    fileO = open(filename,'w')
    fileO.write(output)
    fileO.close()
    os.chmod(filename,stat.S_IRUSR|stat.S_IWUSR|stat.S_IXUSR)
    return

if len(sys.argv) >3:
    action = sys.argv[1];
    parameter = sys.argv[2];
    fileOut = sys.argv[3];

    if action == "delete":
        output = deleteJob(parameter)
    elif action == "status":
        output = statusJob(parameter)
    elif action == "submit":
        output = submitJob(parameter)
    elif action == "activity":
        output = activityDocument(parameter)
    elif action == "factory":
        output =  factoryDocument(parameter)
    elif action == "compact":
        output =  factoryCompactDocument(parameter)
    else:
        output = "Incorrect action"

    writeOutput(output,fileOut);
    
else:
    print "Incorrect number of parameters."
    print "$./sge.py action parameter output_file"
    print "   action: delete, submit, status or activity"
    print "   parameter: jobid or scriptFile"

