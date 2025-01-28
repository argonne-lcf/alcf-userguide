# Running Jobs on Sophia

### Nodes vs Queue
The Sophia nodes are NVIDIA DGX A100 nodes, and each node contains eight (8) A100 GPUs. 
The majority of the nodes have the 40 GB A100 models, but two nodes contain the 80 GB A100 models (see below). 
You may request resources by node (with 8 GPUs) or by individual GPUs based on your job needs. 
What you will get is determined by the queue you submit to (see Queues section below).

## <a name="Sophia-Queues"></a>Queues

There are three production queues you can target in your `qsub` command (`-q <queue name>`):

| Queue Name  | Node/GPU Min | Node/GPU Max                             | Time Min | Time Max |
|-------------|--------------|------------------------------------------|----------|----------|
| `by-gpu `     | 1 GPU        | 8 GPUs (valid values are 1, 2, 4, and 8) | 5 min    | 12 hr    |
| `by-node`     | 1 Node       | 8 Nodes                                  | 5 min    | 12 hr    |
| `bigmem`      | 1 Node       | 1 Node                                   | 5 min    | 12 hrs   |

!!! note

    For all Sophia queues, `MaxQueued` will be 20 queued or running jobs (per project) and `MaxRunning` will be 5 concurrent jobs (per project)

The initial queue policy will be simple First-In-First-Out (FIFO) based on priority with EASY backfill. 
The `by-queue` and `by-gpu` queues target non-bigmem nodes. 
The old `single-node` queue is now a routing queue (redirect) to the `by-node`, and the old `single-gpu` queue is now a routing queue (redirect) to the `by-gpu` queue.

## <a name="Sophia-Queues"></a>Queue Descriptions

### `by-gpu`

This is the **default** production queue[^1] and is targeted at jobs that can utilize 1-8 GPUs. 
The number of "chunks" you specify in your `qsub` (i.e., `-l select=4`) will be the number of GPUs you are allocated, and they will all be on the same node. 
Valid values are 1, 2, 4, or 8 in your select statement. 
These restrictions ensure you get a sane set of resources (RAM is in the same NUMA node as the cores, the GPU has minimal hops to the GPU, etc.). 
If you specify a different value, your `qsub` will issue an error and fail.

### `by-node`

This queue is targeted at jobs that can utilize more than 8 GPUs. 
The number of "chunks" you specify in your `qsub` (i.e., `-l select=4`) will be the number of Sophia DGX nodes (with 8 GPUs each) you are allocated.

### `bigmem`

Two of the nodes have 80GB of RAM per GPU, while the other 22 have 40GB of RAM per GPU (640 GB of aggregate GPU memory per node vs. 320 aggregate GPU memory per node). 
Use this queue to access the 2 nodes with more memory by specifying `-q bigmem` in your `qsub`. 
A max of 1 node (`-l select=1`) can be requested in this queue.

[^1]: The default queue is where your job will be submitted if you don't have `-q <queue name>` in your `qsub`.
