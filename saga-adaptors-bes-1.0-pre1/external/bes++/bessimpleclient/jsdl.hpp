/* ----------------------------------------------------------------
 * jsdl.h
 *   
 *     Header file of BES++ client library
 *
 * Copyright (C) 2006-2009, Platform Computing Corporation. All Rights Reserved.
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
#ifndef _JSDL_H
#define _JSDL_H

#include <stdsoap2.h>

enum jsdl_application_type 
{
  JSDL_POSIX_APPLICATION       = 1,
  JSDL_HPC_PROFILE_APPLICATION = 2,
};

struct jsdl_envvar 
{
  struct jsdl_envvar * next;
  char               * name;
  char               * val;
};

struct jsdl_hpcp_application 
{
  char               *  Executable;
  int                   num_args;
  char               ** Argument;
  char               *  Input;
  char               *  Output;
  char               *  Error;
  char               *  WorkingDirectory;
  struct jsdl_envvar *  Environment;
  char               *  UserName;
};

struct jsdl_posix_application 
{
  char               *  Executable;
  int                   num_args;
  char               ** Argument;
  char               *  Input;
  char               *  Output;
  char               *  Error;
  char               *  WorkingDirectory;
  struct jsdl_envvar *  Environment;
  long                  WallTimeLimit;
  long                  FileSizeLimit;
  long                  CoreDumpLimit;
  long                  DataSegmentLimit;
  long                  LockedMemoryLimit;
  long                  MemoryLimit;
  long                  OpenDescriptorsLimit;
  long                  PipeSizeLimit;
  long                  StackSizeLimit;
  long                  CPUTimeLimit;
  long                  ProcessCountLimit;
  long                  VirtualMemoryLimit;
  long                  ThreadCountLimit;
  char               *  UserName;
  char               *  GroupName;
};

struct jsdl_bound 
{
  double value;
  int    exclusive;
};

struct jsdl_exact 
{
  struct jsdl_exact * next;
  double              value;
  double              epsilon;
};

struct jsdl_range 
{
  struct jsdl_range * next;
  struct jsdl_bound * LowerBound;
  struct jsdl_bound * UpperBound;
};

struct jsdl_range_value 
{
  struct jsdl_bound * UpperBoundedRange;
  struct jsdl_bound * LowerBoundedRange;
  struct jsdl_exact * Exact;
  struct jsdl_range * Range;
};

enum jsdl_file_system_types 
{
  jsdl_nofstype  = 0,
  jsdl_swap      = 1,
  jsdl_temporary = 2,
  jsdl_spool     = 3,
  jsdl_normal    = 4
};

struct jsdl_file_system 
{
  struct jsdl_file_system     * next;
  char                        * name;
  enum jsdl_file_system_types   type;
  char                        * MountPoint;
  struct jsdl_range_value     * DiskSpace;
};

struct jsdl_operating_system 
{
  char * OperatingSystemName;
  char * other;
  char * OperatingSystemVersion;
};

struct jsdl_cpu_architecture 
{
  char * CPUArchitectureName;
  char * other;
};

enum jsdl_creation_flags 
{
  jsdl_nocreationflag  = 0,
  jsdl_overwrite       = 1,
  jsdl_dontOverwrite   = 2,
  jsdl_append          = 3,
};

struct hpcp_credential 
{
  char * username;
  char * password;
  char * raw_token;
};

struct jsdl_data_staging 
{
  struct jsdl_data_staging * next;
  char                     * name;
  char                     * FileName;
  char                     * FileSystemName;
  enum jsdl_creation_flags   CreationFlag;
  int                        DeleteOnTermination;
  char                     * SourceURI;
  char                     * TargetURI;
  struct hpcp_credential   * Credential;
};

struct jsdl_job_definition 
{
  /* JobIdentification part of the JSDL Document */
  char * JobName;
  char * JobAnnotation;
  char * JobProject;

  /* Application part of the JSDL Document */
  char                       * ApplicationName;
  char                       * ApplicationVersion;
  enum jsdl_application_type   ApplicationType;
  void                       * Application;

  /* Resources part of the JSDL document */
  int                             num_hosts;
  char                         ** HostName;
  struct jsdl_file_system      *  FileSystem;
  int                             ExclusiveExecution;
  struct jsdl_operating_system *  OperatingSystemType;
  struct jsdl_cpu_architecture *  CPUArchitecture;
  struct jsdl_range_value      *  IndividualCPUSpeed;
  struct jsdl_range_value      *  IndividualCPUTime;
  struct jsdl_range_value      *  IndividualCPUCount;
  struct jsdl_range_value      *  IndividualNetworkBandwidth;
  struct jsdl_range_value      *  IndividualPhysicalMemory;
  struct jsdl_range_value      *  IndividualVirtualMemory;
  struct jsdl_range_value      *  IndividualDiskSpace;
  struct jsdl_range_value      *  TotalCPUTime;
  struct jsdl_range_value      *  TotalCPUCount;
  struct jsdl_range_value      *  TotalPhysicalMemory;
  struct jsdl_range_value      *  TotalVirtualMemory;
  struct jsdl_range_value      *  TotalDiskSpace;
  struct jsdl_range_value      *  TotalResourceCount;

  /* DataStaging part of the JSDL document */
  struct jsdl_data_staging     *  DataStaging;
};

int  jsdl_newJobDefinition         (enum   jsdl_application_type, struct jsdl_job_definition**);
int  jsdl_newRangeValue            (struct jsdl_range_value **);
int  jsdl_addUpperBound            (struct jsdl_range_value *, double, int);
int  jsdl_addLowerBound            (struct jsdl_range_value *, double, int);
int  jsdl_addExact                 (struct jsdl_range_value *, double, double);
int  jsdl_addRange                 (struct jsdl_range_value *, double, int, double, int);
int  jsdl_processJobDefinition     (struct soap_dom_element *, struct jsdl_job_definition **);
void jsdl_freeJobDefinition        (struct jsdl_job_definition *);
int  jsdl_generateJobDefinitionDOM (struct jsdl_job_definition *, struct soap_dom_element **);
void jsdl_freeJobDefinitionDOM     (struct soap_dom_element *);

#endif /* _JSDL_H */

