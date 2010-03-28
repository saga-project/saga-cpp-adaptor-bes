#ifndef _CONFIG_H

/* You can set project global definitions and includes here */
#define BESSERVER_TOP "/Users/merzky/links/saga/BES/bespp/besserver/"
#define QSUB "/usr/pbs/bin/qsub"
#define QSUBEXEC "qsub" 

/* Uncomment the next line if you are building on Mac OS X */
#define MACOSX

/* You probably don't need to change these. */
/* Setting the VERSION_STRING macro determines what is output */
/* when using the -V option to besclient and besserver */
#define VERSION_STRING "BES++ 1.1.0, 2009-01-12"
#define STAGE_SCRIPT BESSERVER_TOP "/scripts/stage.py"
#define CRED_SCRIPT BESSERVER_TOP "/scripts/cred.py"
#define PBS_SCRIPT BESSERVER_TOP "/scripts/pbs.py"
#define SGE_SCRIPT BESSERVER_TOP "/scripts/sge.py"

/* Don't change this one */
#define COPYRIGHT "Copyright (C) 2006-2009 Platform Computing Corporation"

#endif /* _CONFIG_H */

