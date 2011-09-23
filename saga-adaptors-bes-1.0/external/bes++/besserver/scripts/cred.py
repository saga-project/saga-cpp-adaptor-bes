#!/usr/bin/python

#Usage $./cred.py server username password

import pexpect
import sys
import re
import time

server=sys.argv[1]
username=sys.argv[2]
password=sys.argv[3]

if server[:10]=="myproxy://":
   server=server[10:]
   
if server.find(':')==-1:
   port=0
else:
   port=server[server.find(':')+1:]
   server=server[:server.find(':')]

if port==0:
   myproxy=pexpect.spawn('myproxy-get-delegation -s%s -l%s\n'%(server,username));
else:
   myproxy=pexpect.spawn('myproxy-get-delegation -s%s -p%s -l%s\n'%(server,port,username));
myproxy.expect('pass phrase:');
myproxy.sendline(password);
myproxy.expect(pexpect.EOF)

print "Credential obtained from " + server

