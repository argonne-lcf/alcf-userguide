#! /bin/bash 
set -e 
export SOFTWARE_HOME=/opt
LOGDIR=`date +%m%d%y.%H`
if [ "$2" ] ; then
LOGDIR=$2
fi
MODEL_NAME="GPT1.5B_base_single_$1"
OUTPUT_PATH=/data/ANL/results/$(hostname)/${USER}/${LOGDIR}/${MODEL_NAME}.out
echo "Using ${OUTPUT_PATH} for output"
mkdir -p /data/ANL/results/$(hostname)/${USER}/${LOGDIR}

# Base python env now supports this, including in sbatch mode
#ACTIVATE=/opt/sambaflow/apps/nlp/transformers_on_rdu/venv/bin/activate
#######################
# Edit these variables.
#######################
export OMP_NUM_THREADS=18
#######################
# Start script timer
SECONDS=0
# Temp file location
DIRECTORY=$$
OUTDIR=/data/scratch/${USER}/${MODEL_NAME}
mkdir -p ${OUTDIR}
#source ${ACTIVATE}
echo "Model: " ${MODEL_NAME} >> ${OUTPUT_PATH} 2>&1
echo "Date: " $(date +%m/%d/%y) >> ${OUTPUT_PATH} 2>&1
echo "Time: " $(date +%H:%M) >> ${OUTPUT_PATH} 2>&1
apt list --installed sambaflow >> ${OUTPUT_PATH} 2>&1
cd ${OUTDIR}
#######################
echo "Machine State Before: " >> ${OUTPUT_PATH} 2>&1
/opt/sambaflow/bin/snfadm -l inventory >> ${OUTPUT_PATH} 2>&1
#######################
if [ ! -e ${OUTDIR}/${MODEL_NAME}/${MODEL_NAME}.pef ] ; then
  echo "PEF ${OUTDIR}/${MODEL_NAME}/${MODEL_NAME}.pef does not exist, exiting" >> ${OUTPUT_PATH} 2>&1
  exit 1
fi
 
#######################
echo "RUN" >> ${OUTPUT_PATH} 2>&1
#export CCL_TIMEOUT=3600
export REQUESTS_CA_BUNDLE=/usr/local/lib/python3.8/site-packages/certifi/cacert.pem
export CURL_CA_BUNDLE="/etc/ssl/certs/ca-certificates.crt"
export SAMBA_CCL_HIERARCHICAL_ALLREDUCE=1
      
COMMAND="/usr/local/bin/srun --mpi=pmi2 python /opt/sambaflow/apps/nlp/transformers_on_rdu/transformers_hook.py run  -b $1 --data_dir /data/ANL/ss1024 --pef=${OUTDIR}/${MODEL_NAME}/${MODEL_NAME}.pef --output_dir=${OUTDIR}/hf_gpt1dot5b_ss1k_gas_1_bs16 --module_name gpt2_pretrain --task_name clm --max_seq_length 1024  --overwrite_output_dir --do_train  --per_device_train_batch_size $1 --tokenizer_name gpt2 --model_name gpt2 --non_split_head --skip_broadcast_patch --no_index_select_patch --config_name /opt/sambaflow/apps/nlp/transformers_on_rdu/customer_specific/mv/configs/gpt2_config_xl_50260.json --max_grad_norm_clip 1.0 --skip_checkpoint --logging_steps 1 --max_steps 100 --learning_rate 0.00025 --steps_this_run 100"  >> ${OUTPUT_PATH} 2>&1

echo "COMMAND= $COMMAND" >> ${OUTPUT_PATH} 2>&1
eval $COMMAND >> ${OUTPUT_PATH} 2>&1

#######################
echo "Machine state After: " >> ${OUTPUT_PATH} 2>&1
/opt/sambaflow/bin/snfadm -l inventory >> ${OUTPUT_PATH} 2>&1
echo "Duration: " $SECONDS >> ${OUTPUT_PATH} 2>&1
