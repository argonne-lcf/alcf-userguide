# scikit-learn on Aurora

[scikit-learn](https://scikit-learn.org/stable/) is a popular open-source Python library for machine learning. It has wide coverage of machine learning algorithms (other than neural networks), such as k-means clustering and random forests.

scikit-learn (abbreviated "sklearn") is built for CPUs. However, Intel(R) Extension for Scikit-learn (abbreviated "sklearnex") is a free Python package that speeds up scikit-learn on Intel CPUs & GPUs and adds support for additional functionality, such as incremental and distributed algorithms. For more information, see the [scikit-learn-intelex GitHub page](https://github.com/oneapi-src/oneDAL), [the documentation](https://intel.github.io/scikit-learn-intelex/latest/index.html), or [Intel's website](https://www.intel.com/content/www/us/en/developer/tools/oneapi/scikit-learn.html#gs.b2f4sw).

## Environment Setup

Intel Extension for Scikit-learn is already pre-installed on Aurora, available in the `frameworks` module. You can load the frameworks module as described [here](../python.md), which will activate a conda environment.

## Usage

### Patching

To accelerate existing scikit-learn code with minimal code changes, Intel Extension for Scikit-learn uses patching: replacing stock scikit-learn algorithms with versions that utilize Intel(R) oneAPI Data Analytics Library (oneDAL).

Note that patching only affects supported algorithms and parameters. To see the current support, check Intel's page [here](https://intel.github.io/scikit-learn-intelex/latest/algorithms.html). Otherwise, the Intel Extension will fall back on stock scikit-learn, which has to run on the CPU. To know which version is being used, enable [Verbose Mode](https://intel.github.io/scikit-learn-intelex/latest/verbose.html), for example with the environment variable `SKLEARNEX_VERBOSE=INFO`. However, verbose mode is only available for supported algorithms.

There are multiple ways to patch scikit-learn with the Intel Extension, as Intel documents [here](https://intel.github.io/scikit-learn-intelex/latest/what-is-patching.html). For example, you can patch within the script, like this:

```python
from sklearnex import patch_sklearn
patch_sklearn()
```

It is important to note that this needs to happen before importing scikit-learn. To explicitly only patch certain estimators, you can import particular functions from `sklearnex` instead of `sklearn`, like this:

```python
from sklearnex.neighbors import NearestNeighbors
```

### GPU Acceleration

Intel Extension for Scikit-learn can execute algorithms on the GPU via the [dpctl](https://intelpython.github.io/dpctl/latest/index.html) package, which should be included in the frameworks module. (If not, see the [Python page](../python.md).) dpctl implements oneAPI concepts like queues and devices.

As described in more detail in Intel's documentation [here](https://intel.github.io/scikit-learn-intelex/latest/oneapi-gpu.html), there are two ways to run on the GPU.

1. Pass the input data to the algorithm as `dpctl.tensor.usm_ndarray`. Then the algorithm will run on the same device as the data and return the result as a usm_array on the same device.
2. Configure Intel Extension for Scikit-learn, for example, by setting a context: `sklearnex.config_context`.

Patching (described above) can be helpful in the case of functionality that already exists in scikit-learn because you can import the functions from `sklearn` instead of `sklearnex`.

### Distributed Mode

To distribute an `sklearnex` algorithm across multiple GPUs, we need several ingredients demonstrated in an example below. We recommend using the MPI backend rather than the CCL backend since it is tested more thoroughly on Aurora.

!!! warning "Warning"
    The current version of Intel Extension to scikit-learn does not scale well to multiple GPUs. The cause has been identified, and we're waiting on a fix. However, if you use the oneDAL C++ API, the scaling is much better.

1. Use dpctl to create a SYCL queue (connection to the GPU devices you choose).
2. Using dpctl and your queue, move your data to the GPU devices.
3. Run the algorithm on that data. The compute will happen where the data is. The algorithm should be from `sklearnex.spmd`.

Since you are importing the algorithm from `sklearnex` instead of `sklearn`, patching is not necessary here.

### An Example Python Script

This example is adapted from [an example](https://github.com/oneapi-src/oneDAL/blob/main/examples/sklearnex/knn_bf_classification_spmd.py) in Intel's scikit-learn-intelex GitHub repo.

```python
import dpctl
import dpctl.tensor as dpt
from mpi4py import MPI
from sklearn.datasets import make_classification
from sklearnex.spmd.neighbors import KNeighborsClassifier

# Create a GPU SYCL queue to store data on device.
q = dpctl.SyclQueue("gpu")

# mpi4py is one way to handle arranging data across ranks.
# For the sake of a concise demo, each rank is generating different random training data.
comm = MPI.COMM_WORLD
rank = comm.Get_rank()
X, y = make_classification(n_samples=100000, n_features=8, random_state=rank)

# Move the data to the GPU devices.
dpt_X = dpt.asarray(X, usm_type="device", sycl_queue=q)
dpt_y = dpt.asarray(y, usm_type="device", sycl_queue=q)

# Run the algorithm.
model_spmd = KNeighborsClassifier(
    algorithm="brute", n_neighbors=20, weights="uniform", p=2, metric="minkowski"
)
model_spmd.fit(dpt_X, dpt_y)
```

### An Example Job Script

Below we give an example job script. Note that we are using Aurora MPICH (the default MPI library on Aurora) and not using oneCCL, so we don't need special oneCCL settings. For more about pinning ranks to CPU cores and GPUs, see the [Running Jobs page](../../running-jobs-aurora.md).

```bash
module use /soft/modulefiles
module load frameworks

# This is to resolve an issue due to a package called "numexpr".
# It sets the variable
# 'numexpr.nthreads' to available number of threads by default, in this case
# to 208. However, the 'NUMEXPR_MAX_THREADS' is also set to 64 as a package
# default. The solution is to either set the 'NUMEXPR_NUM_THREADS' to less than
# or equal to '64' or to increase the 'NUMEXPR_MAX_THREADS' to the available
# number of threads. Both of these variables can be set manually.
export NUMEXPR_NUM_THREADS=64

export CPU_BIND="verbose,list:2-4:10-12:18-20:26-28:34-36:42-44:54-56:62-64:70-72:78-80:86-88:94-96"

# Launch the script
mpiexec -np 12 -ppn 12 --cpu-bind ${CPU_BIND} gpu_tile_compact.sh python knn_mpi4py_spmd.py
```