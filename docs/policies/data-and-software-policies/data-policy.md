# Data Policy

## ALCF Data Confidentiality

The Argonne Leadership Computing Facility (ALCF) network is an open-research network. Because our resources and networks are open to many users and cannot be protected at a partitioned level, we cannot guarantee complete security for any data that resides here. It is up to users to provide the security they need.

Data is not encrypted at rest. Data transferred via SSH (i.e., scp) is encrypted in transmission using SSH’s mechanisms (e.g., AES256, etc.). Data transferred via Globus (GridFTP) isn't normally fully encrypted. The GridFTP control channel is encrypted, but the data channel by default is not (though the authentication processes for both channels are encrypted). If you need full encryption of the data stream, you need to explicitly select "encrypt transfer" in the "Transfer & Timer Options" in the Globus UI or use equivalent options in the CLI or transfer API if you're using those. More information here: [https://docs.globus.org/faq/security](https://docs.globus.org/faq/security/).

The basic level of protection provided is UNIX file level permissions; it is the user's responsibility to ensure that file permissions and umasks are set to match their needs.

!!! warning

	The default permissions and umasks are group and world readable. For help determining or setting file permissions or umasks, or creating a UNIX group, contact [support@alcf.anl.gov](mailto:support@alcf.anl.gov).

## ALCF Staff with Root Privileges

ALCF resource administrators with root privileges are not constrained by the file permissions, and they have the capability to open and/or copy all files on the system. They can also assume a user’s identity on the system. There is no audit trail for access, touching, or moving data; however, ALCF staff does not view or modify project data unless directed by a PI or project member to help debug a problem. Data may be touched or accessed by the filesystem itself if data needs to be repaired or verified for integrity after a filesystem event (e.g., a fsck).

The ALCF resources are Federal resources and are the property of the United States Government. Any or all uses of this system and all files on this system may be intercepted, monitored, recorded, copied, audited, inspected, and disclosed to authorized site, Department of Energy, and law enforcement personnel, as well as authorized officials of other agencies, both domestic and foreign.

Administrators use elevated privileges for maintenance and system management. Following are instances where ALCF staff might look at your files:

- We maintain copies of all job submission error, output, and log files and may review them to determine if a job failure was due to user error or a system failure.
- If you request our assistance via any mechanism (for example, support ticket, direct personal email, in-person, etc.), be aware we may need to view your files using elevated privileges to aid us in resolving your issue.

## Use of Proprietary/Licensed Software

All software used on ALCF computers must be appropriately acquired and used according to the appropriate licensing. Possession or use of illegally copied software is prohibited. Likewise, users shall not copy copyrighted software, except as permitted by the owner of the copyright. Currently, the use of export-controlled codes is prohibited.

## Prohibited Data

The ALCF computer systems are operated as research systems and contain only data related to scientific research. Use of ALCF resources to store, manipulate, or remotely access any sensitive or national security information is prohibited unless documented and approved by the PI and ALCF leadership.

This includes, but is not limited to, personally identifiable information (data that falls under the Privacy Act of 1974, 5 U.S.C. 552a), controlled unclassified information (CUI) to include unclassified controlled nuclear information (UCNI), naval nuclear propulsion information (NNPI), International Traffic in Arms Relations (ITAR), the design or development of nuclear, biological, or chemical weapons, or any weapons of mass destruction. The use of ALCF resources for personal or non-work-related activities is also prohibited.

## Export Control

All principal investigators using ALCF resources and ALCF staff members working with project teams are responsible for knowing whether their project generates any of these prohibited data types or information that falls under Export Control. For questions, contact ALCF Support at [support@alcf.anl.gov](mailto:support@alcf.anl.gov).

## Data Storage Systems

Data stored for any length of time on ALCF resources should only be data directly related to work done on any of the ALCF leadership computing systems. Specific policies apply to the three types of data storage systems maintained at ALCF. Read these policies carefully and plan accordingly in terms of space, usage, and data protection.

### Home File System Space (agile-home, gecko-home)

The home file system (/home) is intended to hold your executable files, configuration files, etc. It is NOT meant to hold the output from your application runs (use the data/parallel file system for that purpose). The home file system space is generally moderate in size and is the best protected. Because of its size, backups are practical to accomplish. The system performs tape backups, enabling the recovery of files more than seven days old or recovery from a catastrophic disk failure. Users should email [support@alcf.anl.gov](mailto:support@alcf.anl.gov) if they need assistance. The table below indicates the capabilities and characteristics of each file system.

**AI Testbed home**

`/home/` shared across the ALCF AI testbed systems, including the AI testbed's login and compute nodes, is different from mira-home. Default user quota on the AI testbed's home is 1 TB storage and 1,000,000 files. This space is backed up.

### Team Project or Campaign File System (Eagle, Flare)

The team project/campaign file system is intended primarily for results output from your computational runs on the ALCF computing systems. This space is accessible to the team members of your project that have an ALCF account. Default storage quota is 1 TB and the default period is 1 year. Consider this space intermediate-term storage. Once any active production and/or analysis is complete and you no longer need regular access to the data, archive it within the ALCF (explained below) or transfer it to your home institution or move it to Eagle to share it with the broader community (explained below).

This space has redundancy in the servers and storage but is so large that replication, snapshots, and backups are not practical. Eagle is a Lustre global parallel file system. All new projects will be given storage allocations on Eagle. More information on Lustre File Striping Basics: Lustre File Striping Basics.

**Pullback Policy:** Projects that do not use a minimum of 50% of their allocated space after 6 months will be subject to a quota limit reduction.

**AI Testbed projects file system**

The team project/campaign file system /projects mounted on AI Testbed's login and compute nodes is intended to facilitate project collaboration and is accessible to the team members of your project that have an ALCF account. Default group storage quota is 2 TB and 2,000,000 files. Please note that this space isn't backed up. Our policy is that data will be purged from disk 6 months after project completion.

### Shared Community Project or Campaign File System (Eagle, Flare)

These Lustre global parallel file systems have community sharing abilities and are useful for sharing the project/campaign data with the broader research community via Globus. This space does not have redundancy in the servers or storage and is so large that replication, snapshots, and backups are not practical. The table below indicates the capabilities and characteristics of each file system. Default storage quota is 1 TB and the default period is 2 years. More information on Lustre file striping can be found in this [presentation](https://www.alcf.anl.gov/sites/default/files/2021-05/IO-optimization_mcpheeters.pdf).

**Data Pullback Policy:** 
Projects that do not use a minimum of 50% of their allocated space after 6 months will be subject to a quota limit reduction.

**Access Termination Policy:** 
Project endpoints that have exhibited no activity* for a period of 6 months will be disabled and the storage space will be reclaimed. Notification will be sent to the PI and project members 30 days prior to and the day of the action.

Activity is defined as, but not limited to:

- Creation of the Globus endpoint
- Globus transfers to and from the endpoint
- atime audits of data files indicating access
- Other factors may include DOIs and citations referring to the project

### Archive Space

The archive space is intended for offline storage of results you wish to retain but either have no immediate need to access or no room in your parallel file system space. Archiving capabilities are available via HPSS. The primary HPSS access is via HSI. HTAR is available, but its path length and file size limitations often cause it to fail. Globus Online and GridFTP are clients that can also be used with HPSS.  Due to the possibility of data corruption, users can request 2 copies for particularly critical data. Such requests will be handled on a case-by-case basis.

## Data Storage Policies

### Disk Capacity and Retention Policies

| ----                                           | /home       | lus/eagle/projects, /eagle, /grand, lus/flare/projects or /flare                                                                            |
|------------------------------------------------|-------------|----------------------------------------------------------------------------------------------------------|
| Default Quota ^1^                              | 50 GB       | 1 TB / 1 million files                                                                                   |
| Quota Enforcement ^2^                          | hard/soft   | hard/soft                                                                                                |
| Disk Redundancy ^3^                            | dual parity | dual parity                                                                                              |
| File Server Snapshots ^6^ (frequency/retained) | none        | none                                                                                                     |
| File Server Metadata Redundancy                | yes         | yes                                                                                                      |
| File Server Metadata Replication ^4^           | yes         | yes                                                                                                      |
| File Server Data Replication ^5^               | yes         | no                                                                                                       |
| Data Purged from Disk                          | n/a         | After 6 months of inactivity (see Access termination policy listed in the section above) ^8^             |

### Tape Capacity and Retention Policies

| ----                                           | /home | lus/eagle/projects, /eagle, /grand, lus/flare/projects or /flare  |
|------------------------------------------------|-------|-------------------------------------------------------------------|
| Automatic Backup to Tape? ^6^                  | yes   | no                                                                |
| Archived to Tape Before Deleted from Disk? ^8^ | yes   | no                                                                |

1. While quotas are subject to negotiation on a case-by-case basis, disk space is a finite resource and projects must exercise good data management practices for their own sake and the sake of other users of the facility. With Lustre, it has become necessary to enforce file quotas as well, which are also negotiable.
2. “Hard quota enforcement” means a job will fail when writing output if you exceed the hard quota limit. "Soft quota enforcement" means you may exceed the soft quota limit (but never the higher hard quota value) for up to seven days. If you do not drop back below the soft quota limit within seven days, writes will begin to fail.
3. Hard drives are in redundancy groups of 10 disks (8 data + 2 parity). In other words, three out of 10 drives would have to fail before data loss occurred.
4. Metadata (i.e., information listing which blocks are part of which files) is written twice to two different storage arrays. Thus, even if an entire array were lost, the metadata would be preserved.
5. Refers to the fact that data (user output) is written twice with each block on two different storage arrays, so that even if an entire array were lost, the data would be preserved.
6. “Yes” denotes that ALCF does regular backups without intervention from the user. Currently gecko-home is unable to be backed up.  
7. The project directory is available on disk for the stipulated period but project quotas are reduced immediately following project end date. Access to the directory will be removed after 180 days. Requests to restore/extend access or reset the quota are reviewed on a case-by-case basis. 
8. Users who wish to retain data must archive or transfer their data elsewhere at the end of the project. Users need an active ALCF account to access archived data on HPSS. See Account Retention Policy for more information. The user is responsible for archiving the data to HPSS or copying it to another facility as desired. Data will be retained on tape for 2 years, at which time it is eligible for removal (subject to change). 
