# Example Job Scripts

This page contains a small collection of example job scripts users may find useful for submitting their jobs on Polaris.
Additional information on PBS and how to submit these job scripts is available [here](./job-and-queue-scheduling.md).

A simple example using a similar script on Polaris is available in the 
[Getting Started Repo](https://github.com/argonne-lcf/GettingStarted/tree/master/Examples/Polaris/affinity_omp).

## CPU MPI-OpenMP Example

The following `submit.sh` example submits a 1-node job to Polaris with 16 MPI ranks per node and 2 OpenMP threads per rank. 

```bash
#!/bin/bash -l
#PBS -l select=1:system=polaris
#PBS -l place=scatter
#PBS -l walltime=0:30:00
#PBS -l filesystems=home:grand
#PBS -q debug
#PBS -A Catalyst

# Change to working directory
cd ${PBS_O_WORKDIR}

# MPI and OpenMP settings
NNODES=`wc -l < $PBS_NODEFILE`
NRANKS_PER_NODE=16
NDEPTH=2
NTHREADS=2

NTOTRANKS=$(( NNODES * NRANKS_PER_NODE ))
echo "NUM_OF_NODES= ${NNODES} TOTAL_NUM_RANKS= ${NTOTRANKS} RANKS_PER_NODE= ${NRANKS_PER_NODE} THREADS_PER_RANK= ${NTHREADS}"

mpiexec -n ${NTOTRANKS} --ppn ${NRANKS_PER_NODE} --depth=${NDEPTH} --cpu-bind depth --env OMP_NUM_THREADS=${NTHREADS} -env OMP_PLACES=threads ./hello_affinity
```

*NOTE: If you are a ```zsh``` user, you will need to ensure **ALL** submission and shell scripts include the ```-l``` flag following ```#!/bin/bash``` as seen in the example above to ensure your environment is being instantiated properly. ```zsh``` is **NOT** supported by HPE and support from ALCF will be best effort only.*

Each Polaris node has 1 Milan CPU with a total of 32 cores and each core supports 2 threads. The process affinity in this example is setup to map each MPI rank to 2 cores utilizing all one thread on each core. The OpenMP settings bind each OpenMP thread to one hardware thread within a core such that all 32 cores are utilized. Some additional notes on the contents of the script before the `mpiexec` command follow.

* `cd ${PBS_O_WORKDIR}` : change into the working directory from where `qsub` was executed.
* ``NNODES= `wc -l < $PBS_NODEFILE` ``: one method for determine the total number of nodes allocated to a job.
* `NRANKS_PER_NODE=16` : This is a helper variable to set the number of MPI ranks for each node to 16.
* `NDEPTH=2` : This is a helper variable to space MPI ranks 2 "slots" from each other. In this example, individual threads correspond to a slot. This will be used together with the `--cpu-bind` option from `mpiexec` and additional binding options are available (e.g. numa).
* `NTHREADS=2` : This is a helper variable to set the number of OpenMP threads per MPI rank. 
* `NTOTRANKS=$(( NNODES * NRANKS_PER_NODE))` : This is a helper variable calculating the total number of MPI ranks spanning all nodes in the job.

Information on the use of `mpiexec` is available via `man mpiexec`. Some notes on the specific options used in the above example follow.

* `-n ${NTOTRANKS}` : This is specifying the total number of MPI ranks to start.
* `--ppn ${NRANKS_PER_NODE}` : This is specifying the number of MPI ranks to start on each node.
* `--depth=${NDEPTH}` : This is specifying how many cores/threads to space MPI ranks apart on each node.
* `--cpu bind depth` : This is indicating the number of cores/threads will be bound to MPI ranks based on the `depth` argument.
* `--env OMP_NUM_THREADS=${NTHREADS}` : This is setting the environment variable `OMP_NUM_THREADS` : to determine the number of OpenMP threads per MPI rank.
* `--env OMP_PLACES=threads` : This is indicating how OpenMP should distribute threads across the resource, in this case across hardware threads.

## GPU MPI Example

Using the CPU job submission example above as a baseline, there are not many additional changes needed to enable an application to make use of the 4 NVIDIA A100 GPUs on each Polaris node. In the following 2-node example (because `#PBS -l select=2` indicates the number of nodes requested), 4 MPI ranks will be started on each node assigning 1 MPI rank to each GPU in a round-robin fashion. A simple example using a similar job submission script on Polaris is available in the [Getting Started Repo](https://github.com/argonne-lcf/GettingStarted/tree/master/Examples/Polaris/affinity_gpu).

```bash
#!/bin/bash -l
#PBS -l select=2:system=polaris
#PBS -l place=scatter
#PBS -l walltime=0:30:00
#PBS -l filesystems=home:eagle
#PBS -q debug
#PBS -A Catalyst

# Enable GPU-MPI (if supported by application)
export MPICH_GPU_SUPPORT_ENABLED=1

# Change to working directory
cd ${PBS_O_WORKDIR}

# MPI and OpenMP settings
NNODES=`wc -l < $PBS_NODEFILE`
NRANKS_PER_NODE=$(nvidia-smi -L | wc -l)
NDEPTH=8
NTHREADS=1

NTOTRANKS=$(( NNODES * NRANKS_PER_NODE ))
echo "NUM_OF_NODES= ${NNODES} TOTAL_NUM_RANKS= ${NTOTRANKS} RANKS_PER_NODE= ${NRANKS_PER_NODE} THREADS_PER_RANK= ${NTHREADS}"

# For applications that internally handle binding MPI/OpenMP processes to GPUs
mpiexec -n ${NTOTRANKS} --ppn ${NRANKS_PER_NODE} --depth=${NDEPTH} --cpu-bind depth --env OMP_NUM_THREADS=${NTHREADS} -env OMP_PLACES=threads ./hello_affinity

# For applications that need mpiexec to bind MPI ranks to GPUs
#mpiexec -n ${NTOTRANKS} --ppn ${NRANKS_PER_NODE} --depth=${NDEPTH} --cpu-bind depth --env OMP_NUM_THREADS=${NTHREADS} -env OMP_PLACES=threads ./set_affinity_gpu_polaris.sh ./hello_affinity
```

The OpenMP-related options are not needed if your application does not use
OpenMP. Nothing additional is required on the `mpiexec` command for
applications that internally manage GPU devices and handle the binding of
MPI/OpenMP processes to GPUs. A small helper script is available for those with
applications that rely on MPI to handle the binding of MPI ranks to GPUs. Some
notes on this helper script and other key differences with the early CPU
example follow.

!!! info "`export MPICH_GPU_SUPPORT_ENABLED=1`"

    For applications that support GPU-enabled MPI (i.e. use MPI to communicate
    data directly between GPUs), this environment variable is required to
    enable GPU support in Cray's MPICH. Omitting this will result in a
    segfault. Support for this also requires that the application was linked
    against the the GPU Transport Layer library (e.g. -lmpi_gtl_cuda), which is
    automatically included for users by the `craype-accel-nvidia80` module in
    the default environment on Polaris. If this gtl library is not properly
    linked, then users will see a error message indicating that upon executing
    the first MPI command that uses a device pointer.


!!! info "`./set_affinity_gpu_polaris.sh`"


    This script is useful for those applications that rely on MPI to bind MPI
    ranks to GPUs on each node. Such a script is not necessary when the
    application handles process-gpu binding. This script simply sets the
    environment variable `CUDA_VISIBLE_DEVICES` to a restricted set of GPUs
    (e.g. each MPI rank sees only one GPU). Otherwise, users would find that
    all MPI ranks on a node will target the first GPU likely having a negative
    impact on performance. An example for this script is available in the
    [Getting Started
    repo](https://github.com/argonne-lcf/GettingStarted/blob/master/Examples/Polaris/affinity_gpu/set_affinity_gpu_polaris.sh)
    and copied below.

### Setting MPI-GPU affinity

The `CUDA_VISIBLE_DEVICES` environment variable is provided for users to set which GPUs on a node are accessible to an application or MPI ranks started on a node.

A copy of the small helper script provided in the [Getting Started repo](https://github.com/argonne-lcf/GettingStarted/blob/master/Examples/Polaris/affinity_gpu/set_affinity_gpu_polaris.sh) is provided below for reference.

```bash
$ cat ./set_affinity_gpu_polaris.sh
#!/bin/bash -l
num_gpus=$(nvidia-smi -L | wc -l)
# need to assign GPUs in reverse order due to topology
# See Polaris Device Affinity Information https://www.alcf.anl.gov/support/user-guides/polaris/hardware-overview/machine-overview/index.html
gpu=$((${num_gpus} - 1 - ${PMI_LOCAL_RANK} % ${num_gpus}))
export CUDA_VISIBLE_DEVICES=$gpu
echo “RANK= ${PMI_RANK} LOCAL_RANK= ${PMI_LOCAL_RANK} gpu= ${gpu}”
exec "$@"
```

!!! note

    The `echo` command prints a helpful message for the user to confirm the
    desired mapping is achieved. Users are encouraged to edit this file as
    necessary for their particular needs.

!!! warning

    If planning large-scale runs with many thousands of MPI ranks, it is
    advised to comment out the `echo` command above so as not to have thousands
    of lines of output written to `stdout`.

### Using MPS on the GPUs

Documentation for the Nvidia Multi-Process Service (MPS) can be found [here](https://docs.nvidia.com/deploy/mps/index.html)

In the script below, note that if you are going to run this as a multi-node job you will need to do this on every compute node,
and you will need to ensure that the paths you specify for `CUDA_MPS_PIPE_DIRECTORY` and `CUDA_MPS_LOG_DIRECTORY` do not "collide" 
and end up with all the nodes writing to the same place.

An example is available in the [Getting Started Repo](https://github.com/argonne-lcf/GettingStarted/tree/master/Examples/Polaris/mps) and discussed below.
The local SSDs or `/dev/shm` or incorporation of the node name into the path would all be possible ways of dealing with that issue.

```bash
#!/bin/bash -l
export CUDA_MPS_PIPE_DIRECTORY=</path/writeable/by/you>
export CUDA_MPS_LOG_DIRECTORY=</path/writeable/by/you>
CUDA_VISIBLE_DEVICES=0,1,2,3 nvidia-cuda-mps-control -d
echo "start_server -uid $( id -u )" | nvidia-cuda-mps-control
```

to verify the control service is running:

```bash
$ nvidia-smi | grep -B1 -A15 Processes
```

and the output should look similar to this:

```bash
+-----------------------------------------------------------------------------+
| Processes:                                                                  |
|  GPU   GI   CI        PID   Type   Process name                  GPU Memory |
|        ID   ID                                                   Usage      |
|=============================================================================|
|    0   N/A  N/A     58874      C   nvidia-cuda-mps-server             27MiB |
|    1   N/A  N/A     58874      C   nvidia-cuda-mps-server             27MiB |
|    2   N/A  N/A     58874      C   nvidia-cuda-mps-server             27MiB |
|    3   N/A  N/A     58874      C   nvidia-cuda-mps-server             27MiB |
+-----------------------------------------------------------------------------+
```

to shut down the service:

`echo "quit" | nvidia-cuda-mps-control`

to verify the service shut down properly:

`nvidia-smi | grep -B1 -A15 Processes`

and the output should look like this:

```bash
+-----------------------------------------------------------------------------+
| Processes:                                                                  |
|  GPU   GI   CI        PID   Type   Process name                  GPU Memory |
|        ID   ID                                                   Usage      |
|=============================================================================|
|  No running processes found                                                 |
+-----------------------------------------------------------------------------+
```

### Using MPS in Multi-node Jobs

As stated earlier, it is important to start the MPS control service on each node in a job that requires it.  An example is available in the [Getting Started Repo](https://github.com/argonne-lcf/GettingStarted/tree/master/Examples/Polaris/mps). The helper script `enable_mps_polaris.sh` can be used to start the MPS on a node.

```bash
#!/bin/bash -l

export CUDA_MPS_PIPE_DIRECTORY=/tmp/nvidia-mps
export CUDA_MPS_LOG_DIRECTORY=/tmp/nvidia-log
CUDA_VISIBLE_DEVICES=0,1,2,3 nvidia-cuda-mps-control -d
echo "start_server -uid $( id -u )" | nvidia-cuda-mps-control
```
The helper script `disable_mps_polaris.sh` can be used to disable MPS at appropriate points during a job script, if needed.

```bash
#!/bin/bash -l

echo quit | nvidia-cuda-mps-control
```
In the example job script `submit.sh` below, MPS is first enabled on all nodes in the job using `mpiexec -n ${NNODES} --ppn 1` to launch the enablement script using a single MPI rank on each compute node. The application is then run as normally. If desired, a similar one-rank-per-node `mpiexec` command can be used to disable MPS on all the nodes in a job.

```bash
#!/bin/bash -l
#PBS -l select=1:system=polaris
#PBS -l place=scatter
#PBS -l walltime=0:30:00
#PBS -q debug 
#PBS -A Catalyst
#PBS -l filesystems=home:grand:eagle

cd ${PBS_O_WORKDIR}

# MPI example w/ 8 MPI ranks per node spread evenly across cores
NNODES=`wc -l < $PBS_NODEFILE`
NRANKS_PER_NODE=8
NDEPTH=8
NTHREADS=1

NTOTRANKS=$(( NNODES * NRANKS_PER_NODE ))
echo "NUM_OF_NODES= ${NNODES} TOTAL_NUM_RANKS= ${NTOTRANKS} RANKS_PER_NODE= ${NRANKS_PER_NODE} THREADS_PER_RANK= ${NTHREADS}"

# Enable MPS on each node allocated to job
mpiexec -n ${NNODES} --ppn 1 ./enable_mps_polaris.sh

mpiexec -n ${NTOTRANKS} --ppn ${NRANKS_PER_NODE} --depth=${NDEPTH} --cpu-bind depth ./hello_affinity

mpiexec -n ${NTOTRANKS} --ppn ${NRANKS_PER_NODE} --depth=${NDEPTH} --cpu-bind depth ./set_affinity_gpu_polaris.sh ./hello_affinity

# Disable MPS on each node allocated to job
mpiexec -n ${NNODES} --ppn 1 ./disable_mps_polaris.sh
```

## Single-node Ensemble Calculations Example

In the script below, a set of four applications are launched simultaneously on a single node.
Each application runs on 8 MPI ranks and targets a specific GPU using the `CUDA_VISIBLE_DEVICES` environment variable.
In the first instance, MPI ranks 0-7 will spawn on CPUs 24-31, and GPU 0 is used.
This pairing of CPUs and GPU is based on output of the `nvidia-smi topo-m` command showing which CPUs share a NUMA domain with each GPU.
It is important to background processes using `&` and to `wait` for all runs to complete before exiting the script or continuing on with additional work.
Note, multiple applications can run on the same set of CPU resources, but it may not be optimal depending on the workload.
An example is available in the [Getting Started Repo](https://github.com/argonne-lcf/GettingStarted/blob/master/Examples/Polaris/ensemble/submit_4x8.sh).

```bash
#!/bin/bash -l
#PBS -l select=1:system=polaris
#PBS -l place=scatter
#PBS -l walltime=0:30:00
#PBS -q debug 
#PBS -A Catalyst
#PBS -l filesystems=home:grand:eagle

#cd ${PBS_O_WORKDIR}

# MPI example w/ 8 MPI ranks per node spread evenly across cores
NNODES=`wc -l < $PBS_NODEFILE`
NRANKS_PER_NODE=8
NTHREADS=1

nvidia-smi topo -m

NTOTRANKS=$(( NNODES * NRANKS_PER_NODE ))
echo "NUM_OF_NODES= ${NNODES} TOTAL_NUM_RANKS= ${NTOTRANKS} RANKS_PER_NODE= ${NRANKS_PER_NODE} THREADS_PER_RANK= ${NTHREADS}"

export CUDA_VISIBLE_DEVICES=0
mpiexec -n ${NTOTRANKS} --ppn ${NRANKS_PER_NODE} --cpu-bind list:24:25:26:27:28:29:30:31 ./hello_affinity & 

export CUDA_VISIBLE_DEVICES=1
mpiexec -n ${NTOTRANKS} --ppn ${NRANKS_PER_NODE} --cpu-bind list:16:17:18:19:20:21:22:23 ./hello_affinity & 

export CUDA_VISIBLE_DEVICES=2
mpiexec -n ${NTOTRANKS} --ppn ${NRANKS_PER_NODE} --cpu-bind list:8:9:10:11:12:13:14:15 ./hello_affinity & 

export CUDA_VISIBLE_DEVICES=3
mpiexec -n ${NTOTRANKS} --ppn ${NRANKS_PER_NODE} --cpu-bind list:0:1:2:3:4:5:6:7 ./hello_affinity & 

wait
```

## Multi-node Ensemble Calculations Example
To run multiple concurrent applications on distinct sets of nodes, one simply needs to provide appropriate hostfiles to the `mpiexec` command. The `split` unix command is one convenient way to create several unique hostfiles, each containing a subset of nodes available to the job. In the 8-node example below, a total of four applications will be launched on separate sets of nodes. The `$PBS_NODEFILE` file will be split into several hostfiles, each containing two lines (nodes). These smaller hostfiles are then used as the argument to the `--hostfile` argument of `mpiexec` to the launch applications. It is important to background processes using `&` and to `wait` for applications to finish running before leaving the script or continuing on with additional work. Note, multiple applications can run on the same set of CPU resources, but it may not be optimal depending on the workload. An example is available in the [Getting Started Repo](https://github.com/argonne-lcf/GettingStarted/blob/master/Examples/Polaris/ensemble/submit_multinode.sh).

```bash
#!/bin/bash -l
#PBS -l select=8:system=polaris
#PBS -l place=scatter
#PBS -l walltime=0:30:00
#PBS -q debug-scaling 
#PBS -A Catalyst
#PBS -l filesystems=home:grand:eagle

cd ${PBS_O_WORKDIR}

# MPI example w/ multiple runs per batch job
NNODES=`wc -l < $PBS_NODEFILE`

# Settings for each run: 2 nodes, 4 MPI ranks per node spread evenly across cores
# User must ensure there are enough nodes in job to support all concurrent runs
NUM_NODES_PER_MPI=2
NRANKS_PER_NODE=4
NDEPTH=8
NTHREADS=1

NTOTRANKS=$(( NUM_NODES_PER_MPI * NRANKS_PER_NODE ))
echo "NUM_OF_NODES= ${NNODES} NUM_NODES_PER_MPI= ${NUM_NODES_PER_MPI} TOTAL_NUM_RANKS= ${NTOTRANKS} RANKS_PER_NODE= ${NRANKS_PER_NODE} THREADS_PER_RANK= ${NTHREADS}"

# Increase value of suffix-length if more than 99 jobs
split --lines=${NUM_NODES_PER_MPI} --numeric-suffixes=1 --suffix-length=2 $PBS_NODEFILE local_hostfile.

for lh in local_hostfile*
do
  echo "Launching mpiexec w/ ${lh}"
  mpiexec -n ${NTOTRANKS} --ppn ${NRANKS_PER_NODE} --hostfile ${lh} --depth=${NDEPTH} --cpu-bind depth ./hello_affinity & 
  sleep 1s
done

wait
```
