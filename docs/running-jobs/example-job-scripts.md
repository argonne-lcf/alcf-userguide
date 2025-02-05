# Example Job Scripts

This page contains a small collection of example job scripts users may find useful for submitting their jobs on Polaris. Additional information on PBS and how to submit these job scripts is available [here](./job-and-queue-scheduling.md).

A simple example using a similar script on Polaris is available in the [Getting Started Repo](https://github.com/argonne-lcf/GettingStarted/tree/master/Examples/Polaris/affinity_omp).

!!! warning "Comments in PBS scripts"

    Since `#` is required prior to each PBS directive, comments should be added **after** the directives have been listed in your submission script. If you try to add comments within the directive list, you *could* experience submission issues due to PBS attempting to read your comment as an additional directive. This includes adding comments on the same line as a directive (i.e., `#PBS -q <queue_name>  #comment`).

## CPU MPI-OpenMP Examples

The following `submit.sh` example submits a 1-node job to Polaris with 16 MPI ranks per node and 2 OpenMP threads per rank. See [Queues](./job-and-queue-scheduling.md#queues) for details on practical limits to node counts and job times for different sizes of jobs.

The [`hello_affinity`](https://github.com/argonne-lcf/GettingStarted/tree/master/Examples/Polaris/affinity_gpu) program is a compiled C++ code, which is built via `make -f Makefile.nvhpc` in the linked directory after cloning the [Getting Started](https://github.com/argonne-lcf/GettingStarted) repository.

```bash linenums="1" title="submit.sh"
#!/bin/bash -l
#PBS -l select=1:system=polaris
#PBS -l place=scatter
#PBS -l walltime=0:30:00
#PBS -l filesystems=home:eagle
#PBS -q debug
#PBS -A Catalyst

# Change to working directory
cd ${PBS_O_WORKDIR}  # (1)!

# MPI and OpenMP settings
NNODES=`wc -l < $PBS_NODEFILE` # (2)!
NRANKS_PER_NODE=16 # (3)!
NDEPTH=2 # (4)!
NTHREADS=2 # (5)!

NTOTRANKS=$(( NNODES * NRANKS_PER_NODE )) # (6)!
echo "NUM_OF_NODES= ${NNODES} TOTAL_NUM_RANKS= ${NTOTRANKS} RANKS_PER_NODE= ${NRANKS_PER_NODE} THREADS_PER_RANK= ${NTHREADS}"

mpiexec -n ${NTOTRANKS} --ppn ${NRANKS_PER_NODE} --depth=${NDEPTH} --cpu-bind depth --env OMP_NUM_THREADS=${NTHREADS} -env OMP_PLACES=threads ./hello_affinity
```

1. `cd ${PBS_O_WORKDIR}`: change into the working directory from where `qsub` was executed.
2. ``NNODES= `wc -l < $PBS_NODEFILE` ``: one method for determining the total number of nodes allocated to a job.
3. `NRANKS_PER_NODE=16`: This is a helper variable to set the number of MPI ranks for each node to 16.
4. `NDEPTH=2`: This is a helper variable to space MPI ranks 2 "slots" from each other. In this example, individual threads correspond to a slot. This will be used together with the `--cpu-bind` option from `mpiexec` and additional binding options are available (e.g. `numa`, `socket`, `core`, etc.).
5. `NTHREADS=2`: This is a helper variable to set the number of OpenMP threads per MPI rank.
6. `NTOTRANKS=$(( NNODES * NRANKS_PER_NODE))`: This is a helper variable calculating the total number of MPI ranks spanning all nodes in the job.

The following function in the `hello_affinity` source code is essential for uniquely identifying the CUDA device even when Multi-Instance GPU (MIG) is enabled, as each physical device will be partitioned into multiple virtual devices, each with unique UUIDs differentiated by the last few characters:

<!-- BEGIN COMPARISON of mkdocs-codeinclude-plugin and pymdownx.snippets -->
<!--codeinclude-->
<!-- [Identifying physical or virtual GPU by UUID](../../GettingStarted/Examples/Polaris/affinity_gpu/main.cpp) block:uuid_print -->
<!--/codeinclude-->

<!-- Snippets paths are relative to base location, by default the current working directory (relative to mkdocs.yml?). You can specify a new base location by setting the base_path. base_path is a list of paths. When evaluating paths, they are done in the order specified. The specified snippet will be evaluated against each base path and the first base path that yields a valid snippet will be returned. -->

=== "Identifying physical or virtual GPU by UUID"
```c++
---8<---
./GettingStarted/Examples/Polaris/affinity_gpu/main.cpp:15:25
---8<---
```

<!--- example of other pymdownx.snippets syntax. ; temp disables it -->
---8<--- "; docs/running-jobs/pbs-qsub-options-table.md"

<!-- END COMPARISON -->

!!! warning inline end "Zsh users"
    If you are a `zsh` user, you will need to ensure **all** PBS job submission and shell scripts include the `-l` flag following `#!/bin/bash` as seen in the example above to ensure your environment is being instantiated properly. `zsh` is **not** officially supported by HPE and support from ALCF will be best effort only.

Each Polaris compute node has 1 Milan CPU with a total of 32 physical cores, with each core supporting 2 hardware threads (for a total of 64 logical cores).

The process affinity in this example is set up to map each MPI rank to 2 physical cores. Each MPI rank spawns 2 OpenMP threads, so 1 thread per physical core. The OpenMP settings bind each OpenMP thread to a single hardware thread within a core, such that all 32 physical cores are utilized. CPU core IDs `32` to `63` are not mapped to any MPI rank, since they correspond to simultaneous multithreaded (SMT) sibling hardware threads that share the execution resources of the core ids `0` to `31`, respectively.

* `cd ${PBS_O_WORKDIR}`: change into the working directory from where `qsub` was executed.
* ``NNODES= `wc -l < $PBS_NODEFILE` ``: one method for determining the total number of nodes allocated to a job.
* `NRANKS_PER_NODE=16`: This is a helper variable to set the number of MPI ranks for each node to 16.
* `NDEPTH=2`: This is a helper variable to space MPI ranks 2 "slots" from each other. In this example, individual threads correspond to a slot. This will be used together with the `--cpu-bind` option from `mpiexec` and additional binding options are available (e.g. `numa`, `socket`, `core`, etc.).
* `NTHREADS=2`: This is a helper variable to set the number of OpenMP threads per MPI rank.
* `NTOTRANKS=$(( NNODES * NRANKS_PER_NODE))`: This is a helper variable calculating the total number of MPI ranks spanning all nodes in the job.

Information on the use of `mpiexec` is available via `man mpiexec`. Some notes on the specific options used in the above example follow.

* `-n ${NTOTRANKS}`: This is specifying the total number of MPI ranks to start.
* `--ppn ${NRANKS_PER_NODE}`: This is specifying the number of MPI ranks to start on each node.
* `--depth=${NDEPTH}`: This is specifying how many cores/threads to space MPI ranks apart on each node.
* `--cpu-bind depth`: This is indicating the number of cores/threads will be bound to MPI ranks based on the `depth` argument.
* `--env OMP_NUM_THREADS=${NTHREADS}`: This is setting the environment variable `OMP_NUM_THREADS` to determine the number of OpenMP threads per MPI rank.
* `--env OMP_PLACES=threads`: This is indicating how OpenMP should distribute threads across the resource, in this case across hardware threads.

### Hardware threads

This example is similar to the previous, but it exhausts all 64 logical cores available on each compute node CPU. We double the number of MPI ranks to 32, one per each physical core. Using `--cpu-bind=core`, the `--depth` flag value becomes interpreted by Cray MPICH as spacing in number of **physical cores**, so `NDEPTH=1` ensures that rank 0 is bound to CPU core IDs `(0,32)`, the 2 SMT sibling hardware threads that share the first physical core.
<!-- NOTE about use of "sibling" terminology for logical cores/hardware threads : "SMT sibling hardware threads" as in sysfs thread_siblings_list; not core_siblings_list, which lists all logical core siblings in a socket, as in /proc/cpuinfo siblings entry -->

```bash linenums="1" title="submit_hw_threads.sh"
#!/bin/bash -l
#PBS -l select=1:system=polaris
#PBS -l place=scatter
#PBS -l walltime=0:30:00
#PBS -l filesystems=home:eagle
#PBS -q debug
#PBS -A Catalyst

# Change to working directory
cd ${PBS_O_WORKDIR}

# MPI and OpenMP settings
NNODES=`wc -l < $PBS_NODEFILE`
NRANKS_PER_NODE=32
NDEPTH=1
NTHREADS=2

NTOTRANKS=$(( NNODES * NRANKS_PER_NODE ))
echo "NUM_OF_NODES= ${NNODES} TOTAL_NUM_RANKS= ${NTOTRANKS} RANKS_PER_NODE= ${NRANKS_PER_NODE} THREADS_PER_RANK= ${NTHREADS}"

mpiexec -n ${NTOTRANKS} --ppn ${NRANKS_PER_NODE} --depth=${NDEPTH} --cpu-bind core --env OMP_NUM_THREADS=${NTHREADS} -env OMP_PLACES=threads ./hello_affinity
```

Many HPC applications do not benefit from utilizing the CPU's SMT2 capabilities, and such software may achieve better performance by using the previous script such that each of the 32 physical cores only runs a single OpenMP thread.

## GPU MPI Examples

Using the CPU job submission examples above as a baseline, there are not many additional changes needed to enable an application to make use of the 4 NVIDIA A100 GPUs on each Polaris node. In the following 2-node example (because `#PBS -l select=2` indicates the number of nodes requested), 4 MPI ranks will be started on each node assigning 1 MPI rank to each GPU in a round-robin fashion. A simple example using a similar job submission script on Polaris is available in the [Getting Started Repo](https://github.com/argonne-lcf/GettingStarted/blob/master/Examples/Polaris/affinity_gpu/).

```bash linenums="1" title="submit_gpu.sh"
#!/bin/bash -l
#PBS -l select=2:system=polaris
#PBS -l place=scatter
#PBS -l walltime=0:30:00
#PBS -l filesystems=home:eagle
#PBS -j oe
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

The affinity options `NDEPTH=8;` and `--cpu-bind depth` or `core` are set to ensure that each MPI rank is bound to a separate NUMA node. If OpenMP threading is desired, set `NTHREADS=8` for each MPI rank to spawn 1 thread per physical core (all in the same NUMA domain that the rank is bound to). The OpenMP-related options are not needed if your application does not use OpenMP. Nothing additional is required on the `mpiexec` command for applications that internally manage GPU devices and handle the binding of MPI/OpenMP processes to GPUs. A small helper script is available for those with applications that rely on MPI to handle the binding of MPI ranks to GPUs. Some notes on this helper script and other key differences with the early CPU example follow.
<!-- NOTE: "-d 8 --cpu-bind=core" equiv to "-d 16 --cpu-bind=numa", so it is not quite the same as -d 8 --cp-bind=depth. E.g. in the former 2, rank0 has logical cores (0-7,32-39) so if NTHREADS=8, the behavior will be the same as "-d 8 --cpu-bind=depth -->

!!! info "`export MPICH_GPU_SUPPORT_ENABLED=1`"

    For applications that support GPU-enabled MPI (i.e. use MPI to communicate data directly between GPUs), this environment variable is required to enable GPU support in Cray's MPICH. Omitting this will result in a segfault. Support for this also requires that the application was linked against the GPU Transport Layer library (e.g. -lmpi_gtl_cuda), which is automatically included for users by the `craype-accel-nvidia80` module in the default environment on Polaris. If this gtl library is not properly linked, then users will see an error message indicating that upon executing the first MPI command that uses a device pointer.

!!! info "`./set_affinity_gpu_polaris.sh`"

    This script is useful for those applications that rely on MPI to bind MPI ranks to GPUs on each node. Such a script is not necessary when the application handles process-gpu binding. This script simply sets the environment variable `CUDA_VISIBLE_DEVICES` to a restricted set of GPUs (e.g. each MPI rank sees only one GPU). Otherwise, users would find that all MPI ranks on a node will target the first GPU likely having a negative impact on performance. An example for this script is available in the [Getting Started repo](https://github.com/argonne-lcf/GettingStarted/blob/master/Examples/Polaris/affinity_gpu/set_affinity_gpu_polaris.sh) and copied below.

### Hardware threads

```bash linenums="1" title="submit_gpu_hw_threads.sh"
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
NDEPTH=16
NTHREADS=16

NTOTRANKS=$(( NNODES * NRANKS_PER_NODE ))
echo "NUM_OF_NODES= ${NNODES} TOTAL_NUM_RANKS= ${NTOTRANKS} RANKS_PER_NODE= ${NRANKS_PER_NODE} THREADS_PER_RANK= ${NTHREADS}"

# For applications that internally handle binding MPI/OpenMP processes to GPUs
mpiexec -n ${NTOTRANKS} --ppn ${NRANKS_PER_NODE} --depth=${NDEPTH} --cpu-bind numa --env OMP_NUM_THREADS=${NTHREADS} -env OMP_PLACES=threads ./hello_affinity

# For applications that need mpiexec to bind MPI ranks to GPUs
#mpiexec -n ${NTOTRANKS} --ppn ${NRANKS_PER_NODE} --depth=${NDEPTH} --cpu-bind numa --env OMP_NUM_THREADS=${NTHREADS} -env OMP_PLACES=threads ./set_affinity_gpu_polaris.sh ./hello_affinity
```

As in the previous hardware threads example, the MPI ranks are spaced apart assuming the user wants to utilize all 64 logical cores (achieved by setting `NTHREADS=$NDEPTH=16` and `--cpu-bind numa` here).

In this script, we have added `-j oe` to the list of PBS options; `-j oe` combines stdout and stderr to the same file and uses the stdout filename provided (if provided). `-j eo` would do the same but use the stderr filename provided. Without these options, separate files containing stdout and stderr of the job are produced.

Here we compare two bare-bones PBS submission scripts for a CUDA example with and without MPI:

<!--codeinclude-->
[No MPI](../../ALCFBeginnersGuide/polaris/examples/01_example_cu.sh)
[With MPI](../../ALCFBeginnersGuide/polaris/examples/01_example_mpi.sh)
<!--/codeinclude-->

### Setting GPU affinity for each MPI rank

The `CUDA_VISIBLE_DEVICES` environment variable is provided for users to set which GPUs on a node are accessible to an application or MPI ranks started on a node.

A copy of the small helper script provided in the [Getting Started repo](https://github.com/argonne-lcf/GettingStarted/blob/master/Examples/Polaris/affinity_gpu/set_affinity_gpu_polaris.sh) is provided below for reference:

<!--codeinclude-->
[GPU affinity script](../../GettingStarted/Examples/Polaris/affinity_gpu/set_affinity_gpu_polaris.sh)
<!--/codeinclude-->

!!! note

    The `echo` command prints a helpful message for the user to confirm the desired mapping is achieved. Users are encouraged to edit this file as necessary for their particular needs.

!!! warning

    If planning large-scale runs with many thousands of MPI ranks, it is advised to comment out the `echo` command above so as not to have thousands of lines of output written to `stdout`.

### Using MPS on the GPUs

Documentation for the NVIDIA Multi-Process Service (MPS) can be found [here](https://docs.nvidia.com/deploy/mps/index.html)

In the script below, note that if you are going to run this as a multi-node job you will need to do this on every compute node, and you will need to ensure that the paths you specify for `CUDA_MPS_PIPE_DIRECTORY` and `CUDA_MPS_LOG_DIRECTORY` do not "collide" and end up with all the nodes writing to the same place.

An example is available in the [Getting Started Repo](https://github.com/argonne-lcf/GettingStarted/tree/master/Examples/Polaris/mps) and discussed below. The local SSDs or `/dev/shm` or incorporation of the node name into the path would all be possible ways of dealing with that issue.

```bash
#!/bin/bash -l
export CUDA_MPS_PIPE_DIRECTORY=</path/writeable/by/you>
export CUDA_MPS_LOG_DIRECTORY=</path/writeable/by/you>
CUDA_VISIBLE_DEVICES=0,1,2,3 nvidia-cuda-mps-control -d
echo "start_server -uid $( id -u )" | nvidia-cuda-mps-control
```

To verify the control service is running:

```bash linenums="1"
nvidia-smi | grep -B1 -A15 Processes
```

And the output should look similar to this:

``` { .bash .no-copy}
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

To shut down the service:

`echo "quit" | nvidia-cuda-mps-control`

To verify the service shut down properly:

`nvidia-smi | grep -B1 -A15 Processes`

And the output should look like this:

``` { .bash .no-copy}
+-----------------------------------------------------------------------------+
| Processes:                                                                  |
|  GPU   GI   CI        PID   Type   Process name                  GPU Memory |
|        ID   ID                                                   Usage      |
|=============================================================================|
|  No running processes found                                                 |
+-----------------------------------------------------------------------------+
```

### Using MPS in Multi-node Jobs

As stated earlier, it is important to start the MPS control service on each node in a job that requires it. An example is available in the [Getting Started Repo](https://github.com/argonne-lcf/GettingStarted/tree/master/Examples/Polaris/mps). The helper script `enable_mps_polaris.sh` can be used to start the MPS on a node.

```bash linenums="1"
#!/bin/bash -l

export CUDA_MPS_PIPE_DIRECTORY=/tmp/nvidia-mps
export CUDA_MPS_LOG_DIRECTORY=/tmp/nvidia-log
CUDA_VISIBLE_DEVICES=0,1,2,3 nvidia-cuda-mps-control -d
echo "start_server -uid $( id -u )" | nvidia-cuda-mps-control
```

The helper script `disable_mps_polaris.sh` can be used to disable MPS at appropriate points during a job script, if needed.

```bash linenums="1"
#!/bin/bash -l

echo quit | nvidia-cuda-mps-control
```

In the example job script `submit.sh` below, MPS is first enabled on all nodes in the job using `mpiexec -n ${NNODES} --ppn 1` to launch the enablement script using a single MPI rank on each compute node. The application is then run as normally. If desired, a similar one-rank-per-node `mpiexec` command can be used to disable MPS on all the nodes in a job.

```bash linenums="1"
#!/bin/bash -l
#PBS -l select=1:system=polaris
#PBS -l place=scatter
#PBS -l walltime=0:30:00
#PBS -q debug
#PBS -A Catalyst
#PBS -l filesystems=home:eagle

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

In the script below, a set of four applications are launched simultaneously on a single node. Each application runs on 8 MPI ranks and targets a specific GPU using the `CUDA_VISIBLE_DEVICES` environment variable. In the first instance, MPI ranks 0-7 will spawn on CPUs 24-31, and GPU 0 is used. This pairing of CPUs and GPU is based on output of the `nvidia-smi topo-m` command showing which CPUs share a NUMA domain with each GPU. It is important to background processes using `&` and to `wait` for all runs to complete before exiting the script or continuing on with additional work. Note, multiple applications can run on the same set of CPU resources, but it may not be optimal depending on the workload. An example is available in the [Getting Started Repo](https://github.com/argonne-lcf/GettingStarted/blob/master/Examples/Polaris/ensemble/submit_4x8.sh).

```bash linenums="1"
#!/bin/bash -l
#PBS -l select=1:system=polaris
#PBS -l place=scatter
#PBS -l walltime=0:30:00
#PBS -q debug
#PBS -A Catalyst
#PBS -l filesystems=home:eagle

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

```bash linenums="1"
#!/bin/bash -l
#PBS -l select=8:system=polaris
#PBS -l place=scatter
#PBS -l walltime=0:30:00
#PBS -q debug-scaling
#PBS -A Catalyst
#PBS -l filesystems=home:eagle

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

## Job array example

In situations where you wish to repeat a job multiple times with a small change each time, such as in a parameter space study, a job array may be an option. Unlike the multi-node ensemble case above, each subjob in a job array is its own job and will have its own initialization and tear-down by PBS. Also, a job array will not block all nodes for the length of the longest running task, as is the case for an ensemble job. Jobs on Polaris cannot share nodes with other jobs, so job arrays on Polaris cannot be used to distribute work to different CPU cores or GPUs on a node. In that case, an ensemble job or using `mpiexec` as a parallel launcher can accomplish that goal.

Both ensemble jobs and job arrays become unwieldy and inefficient for very large numbers of tasks. They either have limits to the number of tasks that can be created at once (job arrays) or are unable to refill idle nodes when tasks complete (ensemble jobs). In such cases, a [workflow management tool](../polaris/workflows/balsam.md) that can manage the running of tasks is recommended.

### Job array submission scripts

An example job array submission script:

```bash linenums="1" title="submit_array.sh"
#!/bin/bash -l
#PBS -l select=1:system=polaris
#PBS -l place=scatter
#PBS -l walltime=0:10:00
#PBS -q preemptable
#PBS -A datascience
#PBS -l filesystems=home:eagle
#PBS -j oe
#PBS -r y
#PBS -J 0-7:2

cd ${PBS_O_WORKDIR}

# Create a unique subdirectory for subjob with PBS_ARRAY_INDEX
SUBDIRECTORY="${PBS_ARRAY_INDEX}"
mkdir -p ${SUBDIRECTORY}
cd ${SUBDIRECTORY}

# File name where stdout and stderr of application will be directed in subjob subdirectory
OUT_FILE="subjob_${PBS_ARRAY_INDEX}.out"

echo "Running subjob ${SUBDIRECTORY}"
echo "Directing application output to ${SUBDIRECTORY}/${OUT_FILE}"

# MPI example w/ 16 MPI ranks per node spread evenly across cores
NNODES=`wc -l < $PBS_NODEFILE`
NRANKS_PER_NODE=16
NDEPTH=4
NTHREADS=1

NTOTRANKS=$(( NNODES * NRANKS_PER_NODE ))
echo "NUM_OF_NODES= ${NNODES} TOTAL_NUM_RANKS= ${NTOTRANKS} RANKS_PER_NODE= ${NRANKS_PER_NODE} THREADS_PER_RANK= ${NTHREADS}"

APP_PATH=${PBS_O_WORKDIR}/hello_affinity

mpiexec -n ${NTOTRANKS} --ppn ${NRANKS_PER_NODE} --depth=${NDEPTH} --cpu-bind depth ${APP_PATH} &> ${OUT_FILE}
```

There are two required options for job arrays in PBS: `-r` and `-J`.

The `-r` option must be set like this:
```bash
#PBS -r y
```

The `-J` option sets the number of subjobs in the array and the value of their array indices. The example script above will run 4 subjobs and space their array indices in increments of 2, so the array indices will be 0, 2, 4, and 6.

The form the `-J` option takes is
```bash
#PBS -J <start_index>-<end_index>:<skip_index>%<num_concurrent>
```
* `<start_index>` is the index of the first job in the array
* `<end_index>` is the index of the last job in the array
* `<skip_index>` is the number of index integers to skip between subjobs
* `<num_concurrent>` is the maximum number of subjobs that will run concurrently at one time

Within a subjob, the environment variable `PBS_ARRAY_INDEX` will contain the index of the subjob in the array. It can be used in the job script to set the value or paths of inputs or outputs.

### Interacting with job arrays

The status of job arrays can be queried with the command:
```shell
qstat -t
```

When interacting with a job array with commands like `qdel` or `qalter`, include the brackets with the jobid, e.g.:
```shell
qdel 1991684[]
```

### Limits on job arrays

The number of subjobs in a job array is limited by the number of jobs that can be submitted to the queue.

On Polaris, for the debug queue, that is 1, for preemptable, that is 20, and for prod that is 10.

The limit for prod on Polaris is 10 because 10 is the maximum number of jobs that can be routed by prod to one of the execution queues (small, medium, or large). One note, PBS will allow job array submissions of up to 100 subjobs in prod, however, these job arrays will not run because they will not route to an execution queue. This is a known issue on Polaris.
