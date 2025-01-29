# oneCCL

oneAPI Collective Communications Library (oneCCL) provides an efficient implementation of communication patterns used in deep learning. oneCCL is governed by the UXL Foundation and is an implementation of the oneAPI specification.

oneCCL can be used through:

1. Native C++ SYCL mode
2. Horovod
3. PyTorch Distributed Data Parallel (DDP)

## Aurora oneCCL environment

```bash
kaushikvelusamy@aurora-uan-0012:~> module load frameworks
(/opt/aurora/24.180.0/frameworks/aurora_nre_models_frameworks-2024.2.1_u1) kaushikvelusamy@aurora-uan-0012:~> echo $CCL_ROOT
/opt/aurora/24.180.0/CNDA/oneapi/ccl/2021.13.1_20240808.145507
```

<!-- --8<-- [start:onecclenv] -->
**oneCCL environment variables**

We have identified a set of environment settings that typically provide better performance or address potential application hangs and crashes at large scale. This particular setup is still **experimental**, and it might change as the environment variable settings are refined. Users are encouraged to check this page regularly.

```bash
export CCL_PROCESS_LAUNCHER=pmix  
export CCL_ATL_TRANSPORT=mpi
export CCL_ALLREDUCE_SCALEOUT=direct:0-1048576;rabenseifner:1048577-max  # currently best allreduce algorithm at large scale
export CCL_BCAST=double_tree # currently best bcast algorithm at large scale

export CCL_KVS_MODE=mpi
export CCL_CONFIGURATION_PATH=""
export CCL_CONFIGURATION=cpu_gpu_dpcpp
export CCL_KVS_CONNECTION_TIMEOUT=600 

export CCL_ZE_CACHE_OPEN_IPC_HANDLES_THRESHOLD=1024
export CCL_KVS_USE_MPI_RANKS=1

export MPI_PROVIDER=$FI_PROVIDER
unset MPIR_CVAR_CH4_POSIX_COLL_SELECTION_TUNING_JSON_FILE
unset MPIR_CVAR_CH4_COLL_SELECTION_TUNING_JSON_FILE
unset MPIR_CVAR_COLL_SELECTION_TUNING_JSON_FILE
```

The following additional set of environment variable setups might be application-dependent. Users are encouraged to try to set them and see whether they help their applications.

```bash
ulimit -c unlimited
export FI_MR_ZE_CACHE_MONITOR_ENABLED=0
export FI_MR_CACHE_MONITOR=disabled
export FI_CXI_RX_MATCH_MODE=hybrid
export FI_CXI_OFLOW_BUF_SIZE=8388608
export FI_CXI_DEFAULT_CQ_SIZE=1048576
export FI_CXI_CQ_FILL_PERCENT=30
export INTELGT_AUTO_ATTACH_DISABLE=1
export PALS_PING_PERIOD=240
export PALS_RPC_TIMEOUT=240
export MPIR_CVAR_GATHERV_INTER_SSEND_MIN_PROCS=-1 # to solve the sync send issue in Horovod seg fault
export CCL_ATL_SYNC_COLL=1 # to avoid potential hang at large scale
export CCL_OP_SYNC=1 # to avoid potential hang at large scale
```
<!-- --8<-- [end:onecclenv] -->

**Algorithm selection**

```bash
export CCL_COLLECTIVENAME=topo
export CCL_COLLECTIVENAME_SCALEOUT=ALGORITHM_NAME
```
More info on Algorithm selection: [oneCCL Environment Variables](https://oneapi-src.github.io/oneCCL/env-variables.html)

```bash
export CCL_ALLREDUCE=topo
export CCL_ALLREDUCE_SCALEOUT=rabenseifner 
```

## Native C++ SYCL mode

You can compile examples from the oneCCL Git repository and use the library from the system default instead of local builds. More information at: [oneCCL Benchmark User Guide](https://www.intel.com/content/www/us/en/docs/oneccl/benchmark-user-guide/2021-12/overview.html)

To build the C++ benchmark examples:

```bash
cd oneccl
mkdir build
cd build
module load cmake
cmake .. -DCMAKE_C_COMPILER=icx-cc -DCMAKE_CXX_COMPILER=icpx -DCOMPUTE_BACKEND=dpcpp -DCMAKE_INSTALL_PREFIX=/lus/flare/projects/Aurora_deployment/kaushik/all_reduce_frameworks/gitrepos/oneCCL/build/
make -j install

rm -rf _install/bin/* _install/lib/*mpi* _install/lib/*fabric* _install/opt/
```

To run from a job script:

```bash
#!/bin/bash -x
# qsub -l nodes=2:ncpus=208 -q workq  -l walltime=02:00:00 -l filesystems=lustre_scaling -A  prod ./pbs_job_
#PBS -A <ProjectName>
#PBS -k doe

module load frameworks 
cd $PBS_O_WORKDIR
echo Jobid: $PBS_JOBID
echo Running on nodes `cat $PBS_NODEFILE`
NNODES=`wc -l < $PBS_NODEFILE`
RANKS_PER_NODE=12          # Number of MPI ranks per node
NRANKS=$(( NNODES * RANKS_PER_NODE ))
echo "NUM_OF_NODES=${NNODES}  TOTAL_NUM_RANKS=${NRANKS}  RANKS_PER_NODE=${RANKS_PER_NODE}"

CPU_BINDING1=list:4:9:14:19:20:25:56:61:66:71:74:79
EXT_ENV="--env FI_CXI_DEFAULT_CQ_SIZE=1048576"
APP1=/lus/flare/projects/Aurora_deployment/kaushik/all_reduce_frameworks/gitrepos/oneCCL/build/_install/examples/benchmark/benchmark

echo $CCL_ROOT
export LD_LIBRARY_PATH=$CCL_ROOT/lib:$LD_LIBRARY_PATH
export CPATH=$CCL_ROOT/include:$CPATH
export LIBRARY_PATH=$CCL_ROOT/lib:$LIBRARY_PATH

export CCL_PROCESS_LAUNCHER=pmix  
export CCL_ATL_TRANSPORT=mpi
export CCL_ALLREDUCE=topo
export CCL_ALLREDUCE_SCALEOUT=rabenseifner 

export CCL_KVS_MODE=mpi
export CCL_CONFIGURATION_PATH=""
export CCL_CONFIGURATION=cpu_gpu_dpcpp
export CCL_KVS_CONNECTION_TIMEOUT=600 

which python
 
mkdir -p ./out_${PBS_JOBID}/c_oneccl_gpu
for NNODES in 4 8 16 32 64 
do 
RANKS_PER_NODE=12          # Number of MPI ranks per node
NRANKS=$(( NNODES * RANKS_PER_NODE ))

    for BUF_SIZE in 1 2 4 8 16 32 64 128 256 512 1024 2048 4096 8192 16384 32768 65536 131072 262144 524288 1048576 2097152 4194304 8388608 16777216  33554432 67108864 134217728 268435456
    do
        date
        mpiexec ${EXT_ENV}  --env CCL_LOG_LEVEL=info  --env CCL_PROCESS_LAUNCHER=pmix  --env CCL_ATL_TRANSPORT=mpi \
                            --np ${NRANKS} -ppn ${RANKS_PER_NODE} --cpu-bind  $CPU_BINDING1    $APP1   \
                            --elem_counts ${BUF_SIZE},${BUF_SIZE},${BUF_SIZE}  \
                            --coll allreduce -j off -i 1 -w 0  --backend sycl  --sycl_dev_type gpu >  ./out_${PBS_JOBID}/c_oneccl_gpu/${PBS_JOBID}_${NNODES}_${NRANKS}_${RANKS_PER_NODE}_${BUF_SIZE}_sycl_ccl_gpu_out_w1.txt
        date
    echo ${BUF_SIZE}

    done
done

# For CPU only, change benchmark options to : --backend host --sycl_dev_type host
```
For more information on oneCCL benchmark, please refer to: [oneCCL Benchmark User Guide](https://www.intel.com/content/www/us/en/docs/oneccl/benchmark-user-guide/2021-12/overview.html)

## Horovod

TensorFlow Horovod example:

```python
import datetime
from time import perf_counter_ns
import sys

import tensorflow as tf
import horovod.tensorflow as hvd
import intel_extension_for_tensorflow as itex
print(itex.__version__)
hvd.init()

hvd_local_rank = hvd.local_rank()
hvd_size = hvd.size()
print("hvd_local_rank = %d  hvd_size = %d" % (hvd_local_rank, hvd_size))

xpus = tf.config.experimental.list_physical_devices('XPU')
logical_gpus = tf.config.experimental.set_visible_devices(xpus[hvd.local_rank()], 'XPU')
print(xpus)
tf.debugging.set_log_device_placement(True)

dim_size = int(int(sys.argv[1]) / 4)
elapsed1 = []

for _ in range(5):
    with tf.device(f"XPU:{hvd_local_rank % 12}"):
        x = tf.ones([1, dim_size], dtype=tf.float32)
        # print(x)
        t5 = perf_counter_ns() 
        y = hvd.allreduce(x, average=False)
        t6 = perf_counter_ns()
        elapsed1.append(t6 - t5)

if hvd.rank() == 0:
    for e in elapsed1:
        print(e)
```

PyTorch Horovod example:

```python
from time import perf_counter_ns
import sys
import intel_extension_for_pytorch  # Added Extra
import torch.nn.parallel
import horovod.torch as hvd
hvd.init()
hvd_local_rank = hvd.local_rank()
hvd_size = hvd.size()
# print("hvd_local_rank = %d  hvd_size = %d" % (hvd_local_rank, hvd_size))

def get_default_device():
    if torch.xpu.is_available():
        return torch.device(f"xpu:{hvd_local_rank % 12}")
    else:
        return torch.device('cpu')

device = get_default_device()

dim_size = int(int(sys.argv[1]) / 4)
elapsed1 = []

for _ in range(50):
    x = torch.ones([1, dim_size], dtype=torch.float32).to(device, non_blocking=True)
    # print(x)
    t5 = perf_counter_ns() 
    y = hvd.allreduce(x, average=False)
    t6 = perf_counter_ns()
    elapsed1.append(t6 - t5)

if hvd.rank() == 0:
    for e in elapsed1:
        print(e)
```

## PyTorch DDP

```python
import datetime
from time import perf_counter_ns
import sys
import os
import socket
from mpi4py import MPI
import intel_extension_for_pytorch  # Added Extra
import torch.nn.parallel
import torch.distributed as dist
import oneccl_bindings_for_pytorch

MPI.COMM_WORLD.Barrier()

os.environ['RANK'] = str(os.environ.get('PMI_RANK', 0))
os.environ['WORLD_SIZE'] = str(os.environ.get('PMI_SIZE', 1))
mpi_world_size = MPI.COMM_WORLD.Get_size()
mpi_my_rank = MPI.COMM_WORLD.Get_rank()

if mpi_my_rank == 0:
   master_addr = socket.gethostname()
   sock = socket.socket()
   sock.bind(('', 0))
   # master_port = sock.getsockname()[1] 
   master_port = 2345
else:
   master_addr = None
   master_port = None

master_addr = MPI.COMM_WORLD.bcast(master_addr, root=0)
master_port = MPI.COMM_WORLD.bcast(master_port, root=0)
os.environ["MASTER_ADDR"] = master_addr
os.environ["MASTER_PORT"] = str(master_port)

MPI.COMM_WORLD.Barrier()
dist.init_process_group(backend="ccl", init_method='env://', world_size=mpi_world_size, rank=mpi_my_rank, timeout=datetime.timedelta(seconds=3600))
MPI.COMM_WORLD.Barrier()

dist_my_rank = dist.get_rank()
dist_world_size = dist.get_world_size()

def get_default_device():
    if torch.xpu.is_available():
        return torch.device(f"xpu:{dist_my_rank % 12}")
    else:
        return torch.device('cpu')

device = get_default_device()

dim_size = int(int(sys.argv[1]) / 4)
MPI.COMM_WORLD.Barrier()

elapsed1 = []

for _ in range(50):
    x = torch.ones([1, dim_size], dtype=torch.float32).to(device, non_blocking=True)
    # print(x)
    t5 = perf_counter_ns() 
    dist.all_reduce(x, op=dist.ReduceOp.SUM)  # Added Extra op
    MPI.COMM_WORLD.Barrier()
    t6 = perf_counter_ns()
    elapsed1.append(t6 - t5)

if mpi_my_rank == 0:
    for e in elapsed1:
        print(e)
```

References

1. [oneCCL Environment Variables](https://oneapi-src.github.io/oneCCL/env-variables.html)
2. [oneCCL GitHub Repository](https://github.com/oneapi-src/oneCCL)
3. [Intel Torch CCL](https://github.com/intel/torch-ccl)
4. [Argonne LCF DL Scaling](https://github.com/argonne-lcf/dl_scaling)
5. [oneCCL Benchmark User Guide](https://www.intel.com/content/www/us/en/docs/oneccl/benchmark-user-guide/2021-12/overview.html)