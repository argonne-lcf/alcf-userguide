# Compiling and Linking on Sophia

## Overview
Sophia has AMD processors on the login nodes (sophia-login-01,02) and AMD
processors and NVIDIA A100 GPUs on the compute nodes (see [Machine
Overview](../hardware-overview/machine-overview.md) page). The login nodes can
be used to create containers and launch jobs.

**Note:** Until the cross-compiling environment is set up or dedicated build
nodes get added, the compute nodes will have to be used for compiling. Do not
compile codes on the login nodes. To launch an interactive job and acquire a
compute node for compiling, use

```
qsub -I -q workq -A myProjectShortName -n 1 -t HH:MM:SS
```

The default programming environment on the Sophia compute nodes is the GNU compiler tools coupled with NVIDIA’s CUDA toolkit. 

For non-GPU codes:

- gcc
- g++
- gfortran

For CUDA codes, please note that there is a new driver(v470) and default cuda
toolkit (v12.4)

- nvcc

Default Nvidia installed software will just be in your PATH on compute nodes
(not on login nodes).

```which nvcc```



***NEEDS UPDATING: everything from here down:***


For MPI, the latest MPI is in /usr/mpi/gcc/openmpi-4.1.5a1

  - mpicc
  - mpicxx/mpic++/mpiCC
  - mpifort/mpif77/mpif90

On the login nodes, GNU compilers are available.


## Modules on Sophia
Available modules can be listed via the command:
```
module avail
```
Loaded modules in your environment can be listed via the command:
```
module list
```
To load new modules use:
```
module load <module_name>
```

**Usage:** csh and zsh users do not have to do anything special to their environments. bash users, however, will need to add the following to any job scripts:
```
#!/bin/bash
. /etc/profile
```
bash users are also encouraged to modify their ~/.bashrc to ensure the ubuntu system /etc/bash.bashrc file is sourced properly:
```
# Source global definitions
if [ -f /etc/bashrc ]
then
    . /etc/bashrc
elif [ -f /etc/bash.bashrc ]
then
    . /etc/bash.bashrc
fi
```
