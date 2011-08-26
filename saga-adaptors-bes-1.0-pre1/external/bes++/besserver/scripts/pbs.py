#!/usr/bin/python

#Usage $./pbs.py action parameter file.out

import pexpect
import sys
import re
import os
import stat

pbs_bin_location = "/usr/pbs/bin"

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
    qstat = pexpect.spawn('/bin/bash -c "qstat | grep %s"'%(jobid))
    output = qstat.read()
    qstat.close()
    if output == '':
        return 'u'
    listAttributes = output.rsplit();
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

def factoryDocument(queue, fileOut):
    """Writes in fileOut the document with the factory attributes.
    Returns '0' if exited correctly."""
    #Compose the file with the information about resources
    pbsnodes = pexpect.spawn(pbs_bin_location+"/pbsnodes -a")
    nodesInformation = pbsnodes.read()
    pbsnodes.close()
    output = open(fileOut + "RS", 'w')
    hostRE = re.compile("Mom = "+ queue +"\S*")
    memRE = re.compile("resources_available.mem = \S*")
    cpuRE = re.compile("resources_available.ncpus = \S*")
    stateRE = re.compile("state = \S*")
    start = True
    hostInfo = ""
    
    for line in nodesInformation.splitlines():
        hostname = hostRE.search(line)
        if hostname != None:
            if start:
                hostInfo = hostInfo + hostname.group().split()[2] + " "
                start = False
            else:
                hostInfo = hostInfo + "\n" + hostname.group().split()[2] + " "
        else:
            mem = memRE.search(line)
            if (mem != None):
               hostInfo = hostInfo + str(float(mem.group().split()[2][:-2]) / 1024) + " "
            else:
               cpu = cpuRE.search(line)
               if (cpu != None):
                  hostInfo = hostInfo + cpu.group().split()[2]
               else:
		  state = stateRE.search(line)
                  if state != None:
                     if state.group().split()[2] == "free":
                        hostInfo = hostInfo + "free "
   	             elif state.group().split()[2] == "job-busy":
                        hostInfo = hostInfo + "job-busy "
                     else:
                        hostInfo = hostInfo + "unavailable"

    for line in hostInfo.splitlines():
	available = re.compile("unavailable")
	hostAvailable = available.search(line)
	if hostAvailable == None:
		attributes = line.split()
		output.write(attributes[0] + "\n" + attributes[2] + "\n" + attributes[3] + "\n" + attributes[1] + "\n")
    output.close()
    
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
        output =  factoryDocument(parameter, fileOut)
    else:
        output = "Incorrect action"

    writeOutput(output,fileOut);
    
else:
    print "Incorrect number of parameters."
    print "$./pbs.py action parameter output_file"
    print "   action: delete, submit, status or activity"
    print "   parameter: jobid or scriptFile"

