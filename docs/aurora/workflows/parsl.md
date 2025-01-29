# Parsl on Aurora

[Parsl](https://parsl.readthedocs.io/en/stable/) is a parallel programming library for Python.  It can be used to deploy large numbers of tasks in parallel and with complex dependencies on ALCF machines, and is particularly well suited to run high-throughput workflows.  While Parsl is a Python library, it can execute tasks that run any compiled application.  Parsl can also execute tasks that run mpi applications.

Parsl uses Python's concurrent futures module to create functions that return a Python futures object.  A Parsl workflow operates by creating futures for tasks that the Parsl executor will then fulfill by running them on available compute resources.

A Parsl workflow contains two parts:

- The workflow logic of applications, tasks and task dependencies
- The configuration of compute resources that execute tasks

Here we sketch out some possible configurations for executing workflows on Aurora.  

!!! info "These docs were written for Parsl 2025.1.13."

## Installation and Setup
Parsl is a Python library and can be installed with `pip`.  For example, in a Python virtual environment:

```bash linenums="1"
python -m venv $HOME/_env
source $HOME/_env/bin/activate
pip install parsl
```

!!! info "Python on Aurora"
	To get Python on Aurora, users can either load the AI frameworks module with `module load frameworks` or the basic Python 3.10 module with `module load python/3.10.13`

When using Parsl to distribute work over many PBS Jobs (first two examples below), your workflow script will be executed on a login node and will not return until all tasks are completed.  In this situation, it is advisable to run your script in a [screen](https://linuxize.com/post/how-to-use-linux-screen/) session on the login node.

## Parsl Config for a Large Ensemble of Single Tile tasks run over many PBS Jobs

A common use case is to run a large ensemble of tasks that each require one GPU tile on Aurora and to spread this workload over multiple PBS Jobs.  The reason for spreading this workload over many PBS Jobs may be the size of the ensemble and/or the runtime of the tasks.

The `Config` object for this case is defined like this:

```python linenums="1" title="config.py"
# config.py
import os
from parsl.config import Config

# PBSPro is the right provider for ALCF:
from parsl.providers import PBSProProvider
# The high throughput executor is for scaling large single core/tile/gpu tasks on HPC system:
from parsl.executors import HighThroughputExecutor
# Use the MPI launcher to launch worker processes:
from parsl.launchers import MpiExecLauncher

# These options will run work in 1 node batch jobs run one at a time
nodes_per_job = 1
max_num_jobs = 1
tile_names = [f'{gid}.{tid}' for gid in range(6) for tid in range(2)]

# The config will launch workers from this directory
execute_dir = os.getcwd()

aurora_single_tile_config = Config(
    executors=[
        HighThroughputExecutor(
            # Ensures one worker per GPU tile on each node
            available_accelerators=tile_names,
            max_workers_per_node=12,
            # Distributes threads to workers/tiles in a way optimized for Aurora
            cpu_affinity="list:0-7,104-111:8-15,112-119:16-23,120-127:24-31,128-135:32-39,136-143:40-47,144-151:52-59,156-163:60-67,164-171:68-75,172-179:76-83,180-187:84-91,188-195:92-99,196-203",
            # Increase if you have many more tasks than workers
            prefetch_capacity=0,
            # Options that specify properties of PBS Jobs
            provider=PBSProProvider(
                # Project name
                account="Aurora_deployment",
                # Submission queue
                queue="debug",
                # Commands run before workers launched
                # Make sure to activate your environment where Parsl is installed
                worker_init=f'''source $HOME/_env/bin/activate; cd {execute_dir}''',
                # Wall time for batch jobs
                walltime="0:30:00",
                # Change if data/modules located on other filesystem
                scheduler_options="#PBS -l filesystems=home:flare",
                # Ensures 1 manger per node; the manager will distribute work to its 12 workers, one per tile
                launcher=MpiExecLauncher(bind_cmd="--cpu-bind", overrides="--ppn 1"),
                # options added to #PBS -l select aside from ncpus
                select_options="",
                # Number of nodes per PBS job
                nodes_per_block=nodes_per_job,
                # Minimum number of concurrent PBS jobs running workflow
                min_blocks=0,
                # Maximum number of concurrent PBS jobs running workflow
                max_blocks=max_num_jobs,
                # Hardware threads per node
                cpus_per_node=208,
            ),
        ),
    ],
    # How many times to retry failed tasks
    # this is necessary if you have tasks that are interrupted by a PBS job ending
    # so that they will restart in the next job
    retries=1,
)
```

Import this `Config` object and use in a workflow script, e.g.:

```python linenums="1" title="my_parsl_workflow.py"
# my_parsl_workflow.py
import os
import parsl
from parsl import bash_app, python_app

from config import aurora_single_tile_config

# Bash apps are for executing compiled applications or other shell commands
@bash_app
def hello_affinity(stdout='hello.stdout', stderr='hello.stderr'):
    return f'$HOME/GettingStarted/Examples/Aurora/affinity_gpu/sycl/hello_affinity'

# Python apps are for executing native python functions
@python_app
def hello_world(message, sleep_time=1):
    import time
    time.sleep(sleep_time)
    return f"Hello {message}"

working_directory = os.getcwd()

print("Starting my_parsl_workflow")

with parsl.load(aurora_single_tile_config):

    # Create 12 hello_world tasks
    hello_world_futures = [hello_world(f"Aurora {i}") for i in range(12)]
    print(f"Created {len(hello_world_futures)} hello_world tasks")

    # Create 12 hello_affinity tasks
    hello_affinity_futures = [hello_affinity(stdout=f"{working_directory}/output/hello_{i}.stdout",
                                             stderr=f"{working_directory}/output/hello_{i}.stderr")
                              for i in range(12)]
    print(f"Created {len(hello_world_futures)} hello_affinity tasks")

    # This line will block until all hello_world results are returned
    hello_world_results = [tf.result() for tf in hello_world_futures]
    print("hello_world tasks complete")
    print(f"python apps like hello_world return the function result, e.g. {hello_world_results[0]=}")

    # This line will block until all hello_affinity results are returned
    hello_affinity_results = [tf.result() for tf in hello_affinity_futures]
    print("hello_affinity tasks complete")
    print(f"bash apps like hello_affinity return the return code of the executable, e.g. {hello_affinity_results[0]=}")

    print(f"Read results of hello_affinity from stdout file:")
    for i,tf in enumerate(hello_affinity_futures):
        with open(f"{working_directory}/output/hello_{i}.stdout", "r") as f:
            outputs = f.readlines()
            print(outputs)

    print("Tasks done!")
```
Note that a Parsl workflow script must block at some point on the result of all tasks that are created in order to ensure that the tasks complete.

To run this workflow script:
```bash linenums="1"
source $HOME/_env/bin/activate
python my_parsl_workflow.py
```

When executing this script, the script will block until all tasks are completed. You may wish to check the scheduler to verify that Parsl queues a job to execute the tasks.

## Parsl Config for Ensemble of Multinode MPI tasks tasks run over many PBS Jobs

In the previous example, `mpiexec` was used as a launcher, rather than an executor.  In order to run applications that have MPI communication, `mpiexec` has to be used a different way by Parsl.  To run MPI applications, use the `SimpleLauncher` and the `MPIExecutor`.  Note that the configuration has to set `max_workers_per_block` to align with the resource needs of the application.  The `MPIExecutor` can only run tasks that use more than one node.

!!! warning

    Ensembles of tasks launched with `mpiexec` on multiple nodes are currently limited to 1000 total tasks run per batch job.  This means when `mpiexec` calls return, the nodes they used can refill only a limited number of times, rather than an arbitrary number of times like on Polaris.  This is due to a known issue with Slingshot and will be fixed in the future.  Users running MPI application ensembles on Aurora with Parsl should take this into account when configuring their workflows.

This example `Config` object can be used to execute MPI tasks that use two nodes each:

``` python linenums="1" title="config.py"
import parsl
import os
from parsl.config import Config
# PBSPro is the right provider for ALCF:
from parsl.providers import PBSProProvider
# The MPIExecutor is for running MPI applications:
from parsl.executors import MPIExecutor
# Use the Simple launcher
from parsl.launchers import SimpleLauncher

# These options will run work in 10 node batch jobs run one at a time
nodes_per_task = 2
nodes_per_job = 10
max_num_jobs = 1

# We will save outputs in the current working directory
working_directory = os.getcwd()

mpi_ensemble_config = Config(
    executors=[
        MPIExecutor(
            # This creates 1 worker for each multinode task slot
            max_workers_per_block=nodes_per_job//nodes_per_task,
            provider=PBSProProvider(
                account="Aurora_deployment",
                worker_init=f"""source $HOME/_env/bin/activate; \
                                cd {working_directory}""",
                walltime="0:30:00",
                queue="lustre_scaling",
                scheduler_options="#PBS -l filesystems=home:flare",
                launcher=SimpleLauncher(),
                select_options="",
                nodes_per_block=nodes_per_job,
                max_blocks=1,
                cpus_per_node=208,
            ),
        ),
    ],
    retries=1,
)
```

This example workflow uses this `Config` to run an ensemble of 2-node MPI tasks:

```python linenums="1" title="my_parsl_workflow.py"
# my_parsl_workflow.py
import os
import parsl
from parsl import bash_app

from config import mpi_ensemble_config

# This app will run the hello_affinity application with mpiexec
# Using the set_affinity_gpu_aurora.sh script will bind each mpi rank to a gpu tile
@bash_app
def mpi_hello_affinity(parsl_resource_specification, depth=8, stdout='mpi_hello.stdout', stderr='mpi_hello.stderr'):
    APP_DIR = "$HOME/GettingStarted"
    # PARSL_MPI_PREFIX will resolve to `mpiexec -n num_ranks -ppn ranks_per_node -hosts NODE001,NODE002`
    return f"$PARSL_MPI_PREFIX --cpu-bind depth --depth={depth} {APP_DIR}/HelperScripts/Aurora/set_affinity_gpu_aurora.sh {APP_DIR}/Examples/Aurora/affinity_gpu/sycl/hello_affinity"

print("Starting my_parsl_workflow")

working_directory = os.getcwd()

with parsl.load(mpi_ensemble_config):

    task_futures = []

    # Create 2-node tasks
    # We set 12 ranks per node to match the number of gpu tiles on an aurora node
    resource_specification = {'num_nodes': 2, # Number of nodes required for the application instance
                              'ranks_per_node': 12, # Number of ranks / application elements to be launched per node
                              'num_ranks': 24, # Number of ranks in total
                             }

    print(f"Creating mpi tasks with {resource_specification['num_nodes']} nodes per task, {resource_specification['num_ranks']} ranks per task, and {resource_specification['ranks_per_node']} ranks per node")
    task_futures += [mpi_hello_affinity(
                            parsl_resource_specification=resource_specification,
                            stdout=f"{working_directory}/mpi_output/{i}/hello.stdout",
                            stderr=f"{working_directory}/mpi_output/{i}/hello.stderr")
                        for i in range(10)]

    # This loop will block until all task results are returned
    print(f"{len(task_futures)} tasks created, wating for completion")
    for tf in task_futures:
        tf.result()

    print("Tasks done!")
```

## Run Parsl Workflow within a single PBS Job

If your tasks can be run within a single PBS job, Parsl can be configured to run inside the PBS job, instead of submitting multiple jobs to the scheduler as shown in the examples above.

To run the single tile task ensemble from above in this alternate mode, use this `Config` object in the workflow script:
```python linenums="1" title="config.py"
# config.py
import os
from parsl.config import Config

# Use LocalProvider to launch workers within a submitted batch job
from parsl.providers import LocalProvider
# The high throughput executor is for scaling large single core/tile/gpu tasks on HPC system:
from parsl.executors import HighThroughputExecutor
# Use the MPI launcher to launch worker processes:
from parsl.launchers import MpiExecLauncher

tile_names = [f'{gid}.{tid}' for gid in range(6) for tid in range(2)]

# The config will launch workers from this directory
execute_dir = os.getcwd()

# Get the number of nodes:
node_file = os.getenv("PBS_NODEFILE")
with open(node_file,"r") as f:
    node_list = f.readlines()
    num_nodes = len(node_list)

aurora_single_tile_config = Config(
    executors=[
        HighThroughputExecutor(
            # Ensures one worker per GPU tile on each node
            available_accelerators=tile_names,
            max_workers_per_node=12,
            # Distributes threads to workers/tiles in a way optimized for Aurora
            cpu_affinity="list:0-7,104-111:8-15,112-119:16-23,120-127:24-31,128-135:32-39,136-143:40-47,144-151:52-59,156-163:60-67,164-171:68-75,172-179:76-83,180-187:84-91,188-195:92-99,196-203",
            # Increase if you have many more tasks than workers
            prefetch_capacity=0,
            # Options that specify properties of PBS Jobs
            provider=LocalProvider(
                # Number of nodes job
                nodes_per_block=num_nodes,
                launcher=MpiExecLauncher(bind_cmd="--cpu-bind", overrides="--ppn 1"),
                init_blocks=1,
                max_blocks=1,
            ),
        ),
    ],
)
```

Then submit the the workflow with a PBS batch script:
```bash linenums="1"
#!/bin/bash -l
#PBS -l select=1
#PBS -l place=scatter
#PBS -l walltime=0:30:00
#PBS -q debug
#PBS -A <ProjectName>

cd ${PBS_O_WORKDIR}

source $HOME/_env/bin/activate
python my_workflow_script.py
```

Note that if the workflow does not complete before the end of the PBS job, outstanding tasks will not complete.
