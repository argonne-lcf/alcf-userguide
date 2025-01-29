# Dask

[Dask](https://www.dask.org/) is a Python library for parallel and distributed computing. 
A Dask cluster is composed of one scheduler that coordinates the job of many workers, which can have access to CPU or GPU resources. 
Here we show how to install Dask in a conda environment on Aurora and how to start a cluster with GPU workers and run a simple example script. 

## Install Dask on Aurora

From one of Aurora's login nodes, use the following commands to create a conda environment and install Dask. 
This will also install other libraries needed to run an example script and create a Jupyter kernel that allows you to work interactively from a notebook. 

```bash linenums="1"
module load frameworks
conda create -y -n dask -c conda-forge python=3.11 pip dask ipykernel jupyterlab
conda activate dask
# install additional libraries
conda install -y -c https://software.repos.intel.com/python/conda/ -c conda-forge dpnp
# create the jupyter kernel
python -m ipykernel install --prefix=${CONDA_PREFIX} --name dask
```

## Start a Dask cluster

Copy the following script into a file called `start_dask_aurora.sh` and make it executable with:

```bash linenums="1"
chmod a+x ./start_dask_aurora.sh
```

```bash linenums="1" title="start_dask_aurora.sh"
#!/bin/bash

# start_dask_aurora.sh
# Usage: 
# mpiexec -n NNODES * NUM_WORKERS_PER_NODE --ppn NUM_WORKERS_PER_NODE ./start_dask_aurora.sh WORKER_TYPE NUM_WORKERS_PER_NODE
# Examples on two nodes:
# mpiexec -n 12 --ppn 6 ./start_dask_aurora.sh gpu 6
# mpiexec -n 208 --ppn 104 ./start_dask_aurora.sh cpu 104

WORKER_TYPE=$1
NUM_WORKERS_PER_NODE=$2
# if using 12 GPU workers, assign one worker per tile, otherwise use one worker per GPU (2 tiles)
if [ $NUM_WORKERS_PER_NODE = 12 ] && [ $WORKER_TYPE = 'gpu' ]; then
    export ZE_FLAT_DEVICE_HIERARCHY=FLAT
    export ZE_ENABLE_PCI_ID_DEVICE_ORDER=1
else
    export ZE_FLAT_DEVICE_HIERARCHY=COMPOSITE
fi

# Number of threads per worker (208 CPU threads per node divided by num workers)
NTHREADS=$(( 208 / NUM_WORKERS_PER_NODE ))  # 208 / 12 ≈ 17
# Memory limit per worker (1100GB RAM per node divided by num workers)
MEMORY_PER_WORKER=$(( 1100 / NUM_WORKERS_PER_NODE ))GB  # 1100GB / 12 ≈ 91GB
LOCAL_DIRECTORY=~/dask-local-directory
DASK_DASHBOARD_PORT=${DASK_DASHBOARD_PORT:-8787}
DASK_SCHEDULER_PORT=${DASK_SCHEDULER_PORT:-8786}

# Start Dask scheduler on rank 0
if [ $PALS_RANKID = 0 ]; then
    # Purge Dask worker, log directories and config directories
    rm -rf ${LOCAL_DIRECTORY}/* /tmp/dask-workers/*  ~/.config/dask
    mkdir -p ${LOCAL_DIRECTORY}/logs /tmp/dask-workers
    # Setup scheduler
    nohup dask scheduler --port ${DASK_SCHEDULER_PORT} --dashboard-address $DASK_DASHBOARD_PORT \
        --scheduler-file ${LOCAL_DIRECTORY}/scheduler.json > ${LOCAL_DIRECTORY}/logs/$HOSTNAME-scheduler.log 2>&1 &
fi
sleep 10
# Setup workers
if [ $WORKER_TYPE = 'gpu' ]; then
    ZE_AFFINITY_MASK=$PALS_LOCAL_RANKID dask worker \
        --resources "GPU=1" --memory-limit ${MEMORY_PER_WORKER} \
        --nthreads ${NTHREADS}  --local-directory /tmp/dask-workers \
        --scheduler-file ${LOCAL_DIRECTORY}/scheduler.json >> ${LOCAL_DIRECTORY}/logs/$HOSTNAME-worker.log 2>&1
else
    dask worker \
        --nthreads ${NTHREADS} --local-directory /tmp/dask-workers \
        --scheduler-file ${LOCAL_DIRECTORY}/scheduler.json >> ${LOCAL_DIRECTORY}/logs/$HOSTNAME-worker.log 2>&1
fi
```

### Start a cluster with CPU workers

Run the following commands from a compute node on Aurora to start a Dask cluster with 104 CPU workers per node:
```bash linenums="1"
module load frameworks
conda activate dask
NNODES=`wc -l < $PBS_NODEFILE`
mpiexec -n $(( $NNODES * 104 )) --ppn 104 ./start_dask_aurora.sh cpu 104
```

### Start a cluster with GPU workers

Run the following commands from a compute node on Aurora to start a Dask cluster with 6 GPU workers per node:
```bash linenums="1"
module load frameworks
conda activate dask
NNODES=`wc -l < $PBS_NODEFILE`
mpiexec -n $(( $NNODES * 6 )) --ppn 6 ./start_dask_aurora.sh gpu 6
```

## Example

In this example, we will [estimate Pi using a Monte Carlo method](https://en.wikipedia.org/wiki/Pi#Monte_Carlo_methods). 

Paste the following Python script into a file called `pi_dask_gpu.py`. Here is a breakdown of what the script does:

1. It connects to the Dask cluster (that you should start beforehand) and prints some information including the number of workers and available memory.
2. It divides the total number of points to sample between the workers, and each worker uses its GPU to
   - generate random points uniformly inside the unit square
   - return the number of points that are inside the unit circle
3. When the results from the workers are ready, they are aggregated to compute Pi.
4. A total of 5 Pi calculations are performed and timed (the very first iterations will incur initialization and warmup costs).
5. At the end, the Dask cluster is shut down.

```python linenums="1" title="pi_dask_gpu.py"
import json
import pathlib
from dask.distributed import Client


fname = f'{pathlib.Path.home().as_posix()}/dask-local-directory/scheduler.json'
with open(fname, 'r') as f:
    scheduler = json.load(f)
client = Client(scheduler['address'])
print(client)


import time
import dpnp as np


def count_points_inside_circle(N):
    x = np.random.uniform(low=-1.0, high=1.0, size=(N, 2))
    inside_circle = ((x * x).sum(axis=1) < 1.).sum()
    return int(inside_circle)


def compute_pi(inside_circle, N):
    return 4 * inside_circle / N


def run():
    start = time.time()
    num_workers = len(client.scheduler_info()['workers'])
    N = 10_400_000_004

    # number of points per worker
    Neach_section, extras = divmod(N, num_workers)
    points_per_worker = [Neach_section for _ in range(num_workers)]
    points_per_worker[-1] += extras

    futures = client.map(count_points_inside_circle, points_per_worker)
    inside_circle = client.submit(sum, futures).result()
    pi = compute_pi(inside_circle, N)
    end = time.time()
    return f"Num samples: {N:.2E}\t\tEstimate: {pi:.9f}\t\tTime taken: {end - start:.3f} s"


def main(runs=5):
    for i in range(runs):
        print(f"Run {i}\t\t{run()}")


main()
client.shutdown()
```

### Run the `pi_dask_gpu.py` example

- First, request an interactive job on 1 node.
- Then, [start a Dask cluster with 6 GPU workers](#start-a-cluster-with-gpu-workers) and wait about 10 seconds for the cluster to start.
- Press ++ctrl+z++ (SIGTSTP) and then execute `bg` to continue running the process in the background, or open a new shell and SSH onto the compute node. 
- Run the example script:
  ```bash linenums="1"
  module load frameworks
  conda activate dask
  python pi_dask_gpu.py
  ```

???+ example "Output"

    ``` { .bash .no-copy }
	<Client: 'tcp://10.168.0.10:8786' processes=6 threads=204, memory=1.00 TiB>
	Run 0           Num samples: 1.04E+10           Estimate: 3.141653798           Time taken: 1.596 s
	Run 1           Num samples: 1.04E+10           Estimate: 3.141570887           Time taken: 1.354 s
	Run 2           Num samples: 1.04E+10           Estimate: 3.141651954           Time taken: 1.451 s
	Run 3           Num samples: 1.04E+10           Estimate: 3.141636617           Time taken: 0.518 s
	Run 4           Num samples: 1.04E+10           Estimate: 3.141650108           Time taken: 0.511 s
	```

## Connect to a Dask cluster from JupyterLab

Here are the steps to start a Dask cluster and connect to it interactively from a Jupyter notebook:

- First, request an interactive job on 1 node. Print the compute node's hostname (that you get with the command `hostname`), which will be used later.
- Then, [start a Dask cluster](#start-a-cluster-with-gpu-workers) and wait about 10 seconds for the cluster to start.
- **On your local machine**, open an SSH tunnel to the compute node (`COMPUTE_NODE` is the compute node's hostname and `YOUR_ALCF_USERNAME` is your ALCF username):
  ```bash
  ssh -t -L 23456:localhost:23456 -L 8787:localhost:8787 YOUR_ALCF_USERNAME@bastion.alcf.anl.gov ssh -t -L 23456:localhost:23456 -L 8787:localhost:8787 login.aurora.alcf.anl.gov ssh -t -L 23456:localhost:23456 -L 8787:localhost:8787 COMPUTE_NODE
  ```
!!! failure 
    
	If you have issues with the above sequence of `ssh` commands, check [this page](https://github.com/argonne-lcf/ALCF_Hands_on_HPC_Workshop/issues/56) for troubleshooting.
	
- **On the compute node** where you land with the above ssh command, start JupyterLab:
  ```bash linenums="1"
  module load frameworks
  conda activate dask
  jupyter lab --no-browser --port=23456
  ```
- Copy the line starting with `http://localhost:23456/lab?token=<TOKEN>` at the end of the Jupyter command's output.
- **On your local machine**, open a browser window and go to that URL.
- **On the JupyterLab page**, select the `dask` kernel and use this script to connect to the Dask cluster:
  ```python linenums="1"
  import json
  import pathlib
  from dask.distributed import Client
  
  fname = f'{pathlib.Path.home().as_posix()}/dask-local-directory/scheduler.json'
  with open(fname, 'r') as f:
      scheduler = json.load(f)
  client = Client(scheduler['address'])
  client
  ```
- The [Dask dashboard](https://docs.dask.org/en/latest/dashboard.html) will be available at <http://localhost:8787>