#!/usr/bin/python

#Usage $./stageFile.py mode uri file username password
#mode: in or out

import pexpect
import sys
import re

def getHTTP(file,url):
   #print 'wget --quiet --output-document=%s %s\n'%(file,url)
   http=pexpect.spawn('wget --quiet --output-document=%s %s\n'%(file,url))
   http.expect(pexpect.EOF)
   
def getGRIDFTP(file,url):
   print 'globus-url-copy %s file://%s'%(url,file)
   gftp=pexpect.spawn('globus-url-copy %s file://%s'%(url,file))
   gftp.expect(pexpect.EOF)
   
def getFTP(file,server,remoteFile,username,password):
   port=''
   servername = server
   if server.find(':')>0:
      port = server[server.find(':')+1:]
      servername = server[:server.find(':')]
   #print 'ftp %s %s'%(servername,port)
   ftp=pexpect.spawn('ftp %s %s'%(servername,port))
   ftp.expect('Name .*: ')
   ftp.sendline(username)
   ftp.expect('assword:')
   ftp.sendline(password)
   ftp.expect('ftp> ')
   ftp.sendline('get %s %s'%(remoteFile,file))
   ftp.expect('ftp> ')
   ftp.sendline('bye')
   ftp.expect(pexpect.EOF)
   
def getSCP(file,server,remoteFile,username,password):
   scp=pexpect.spawn('scp %s@%s:%s %s'%(username,server,remoteFile,file))
   scp.expect('assword:')
   scp.sendline(password)
   scp.expect(pexpect.EOF)

def putFTP(file,server,remoteFile,username,password):
   port=''
   servername = server
   if server.find(':')>0:
      port = server[server.find(':')+1:]
      servername = server[:server.find(':')]
   ftp=pexpect.spawn('ftp %s %s'%(servername,port))
   ftp.expect('Name .*: ')
   ftp.sendline(username)
   ftp.expect('assword:')
   ftp.sendline(password)
   ftp.expect('ftp> ')
   ftp.sendline('put %s %s'%(file,remoteFile))
   ftp.expect('ftp> ')
   ftp.sendline('bye')
   ftp.expect(pexpect.EOF)
   
def putSCP(file,server,remoteFile,username,password): 
   scp=pexpect.spawn('scp %s %s@%s:%s'%(file,username,server,remoteFile))
   scp.expect('assword:')
   scp.sendline(password)
   scp.expect(pexpect.EOF)

def putGRIDFTP(file,url):
   gftp=pexpect.spawn('globus-url-copy file://%s %s'%(file,url))
   gftp.expect(pexpect.EOF)

if len(sys.argv)>5:
   password=sys.argv[5]
else:
   password=""
if len(sys.argv)>4:
   username=sys.argv[4]
else:
   username=""

file=sys.argv[3]
url=sys.argv[2]
protocol=sys.argv[2][:sys.argv[2].find(':')]
server=sys.argv[2][len(protocol)+3:sys.argv[2].find('/',len(protocol)+4)]
remoteFile=sys.argv[2][len(protocol)+len(server)+3:]
mode=sys.argv[1]

if mode == 'in':
   if protocol== 'http':
      getHTTP(file,url)
   elif protocol== 'ftp':
      getFTP(file,server,remoteFile,username,password)
   elif protocol== 'scp':
      getSCP(file,server,remoteFile,username,password)
   elif protocol=='gsiftp':
      getGRIDFTP(file,url);
   print "File "+file+" staged in from " +url
elif mode=='out':
   if protocol== 'ftp':
      putFTP(file,server,remoteFile,username,password)
   elif protocol== 'scp':
      putSCP(file,server,remoteFile,username,password)
   elif protocol=='gsiftp':
      putGRIDFTP(file,url);
   print "File "+file+" staged out to " +url
      
   
