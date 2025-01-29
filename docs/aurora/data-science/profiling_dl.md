# Profiling Deep Learning Applications

On Aurora, we can use the `unitrace` profiler from Intel to profile deep learning applications. Refer to the [`unitrace` documentation page](https://github.com/intel/pti-gpu/tree/master/tools/unitrace) for details.

## Example Usage

We can use `unitrace` to trace an application running on multiple ranks and multiple nodes. A simple example, where we use a wrapper script to trace rank 0 on each node of a 4-node job running a PyTorch application, is below.

There are several important shell variables in the wrapper, which may require modification:

```bash
#!/bin/bash
## This wrapper should be used with unitrace to trace in any number of nodes.
## The script for this example is set up to trace rank 0 of the first 4 nodes in the case of
## profiling a job running on more than 4 nodes.
FNAME_EXT=$(basename "$2")
FNAME="${FNAME_EXT%%.*}"

NNODES=`wc -l < $PBS_NODEFILE`

WORK_DIR=/path/to/the/Python/program
UNITRACE_DIR=/opt/aurora/24.180.1/support/tools/pti-gpu/063214e # (1)!
UNITRACE_LIB=${UNITRACE_DIR}/lib64
UNITRACE_BIN=${UNITRACE_DIR}/bin
UNITRACE_EXE=${UNITRACE_BIN}/unitrace
DTAG=$(date +%F_%H%M%S)
UNITRACE_OUTDIR=${WORK_DIR}/logs/unitrace_profiles/name_of_choice_json_n${NNODES}_${DTAG}/${FNAME}_n${NNODES}_${DTAG}
mkdir -p ${UNITRACE_OUTDIR}
UNITRACE_OPTS=" --ccl-summary-report --chrome-mpi-logging --chrome-sycl-logging \
--chrome-device-logging \
--chrome-ccl-logging --chrome-call-logging --chrome-dnn-logging --device-timing --host-timing \
--output-dir-path ${UNITRACE_OUTDIR} --output ${UNITRACE_OUTDIR}/UNITRACE_${FNAME}_n${NNODES}_${DTAG}.txt "  # (2)!

export LD_LIBRARY_PATH=${UNITRACE_LIB}:${UNITRACE_BIN}:$LD_LIBRARY_PATH

# Use $PMIX_RANK for MPICH and $SLURM_PROCID with srun.
PROFRANK=0 # (3)!
RANKCUTOFF=48 # (4)!

if [[ $PALS_LOCAL_RANKID -eq $PROFRANK ]] && [[ $PMIX_RANK -lt $RANKCUTOFF ]]; then
  echo "On rank $PMIX_RANK, collecting traces "
  $UNITRACE_EXE $UNITRACE_OPTS "$@"
else
  "$@"
fi
```

1. `UNITRACE_DIR`: This is the main `unitrace` directory, which may change after an update to the programming environment.
2. `UNITRACE_OPTS`: These are the options that `unitrace` uses to trace data at different levels. Based on the number of options, the sizes of the output profiles will vary. Usually, enabling more options leads to a larger profile (in terms of storage in MB).
3. `PROFRANK`: As implemented, this variable is set by the user to trace the rank of choice. For example, this wrapper will trace rank 0 on each node.
4. `RANKCUTOFF`: This variable is Aurora-specific. As we can run as many as 12 ranks per node (without using CCS), the first 4 nodes of a job will have 48 ranks running. This provides the upper cutoff of the label (in number) of ranks, beyond which `unitrace` will not trace any rank. A user can change the number according to the number of maximum ranks running per node to set up how many ranks to be traced. `unitrace` will produce a profile (`json` file, by default) per traced rank. This profile can be viewed using the [Perfetto trace viewer](https://ui.perfetto.dev/).

### Deployment

The wrapper above can be deployed using the following PBS job script:

```bash
#!/bin/bash -x
#PBS -l select=4
#PBS -l place=scatter
#PBS -l walltime=00:10:00
#PBS -q debug-scaling
#PBS -l filesystems=<fs1:fs2>
#PBS -A <ProjectName>

WORK_DIR=/path/to/the/Python/program
UNITRACE_WRAPPER=${WORK_DIR}/unitrace_wrapper.sh

# MPI and OpenMP settings
NNODES=`wc -l < $PBS_NODEFILE`
NRANKS_PER_NODE=12

let NRANKS=${NNODES}*${NRANKS_PER_NODE}

module load frameworks/2024.2.1_u1

mpiexec --pmi=pmix -n ${NRANKS} -ppn ${NRANKS_PER_NODE} -l --line-buffer \
${UNITRACE_WRAPPER} python ${WORK_DIR}/application.py 
```