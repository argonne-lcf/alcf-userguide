#! /bin/bash 
set -e 
export SOFTWARE_HOME=/opt
# Base python env now supports this, including in sbatch mode
#ACTIVATE=/opt/sambaflow/apps/nlp/transformers_on_rdu/venv/bin/activate
LOGDIR=`date +%m%d%y.%H`
if [ "$2" ] ; then
LOGDIR=$2
fi
MODEL_NAME="GPT1.5B_base_single_$1"
OUTPUT_PATH=/data/ANL/results/$(hostname)/${USER}/${LOGDIR}/${MODEL_NAME}.out
echo "Using ${OUTPUT_PATH} for output"
mkdir -p /data/ANL/results/$(hostname)/${USER}/${LOGDIR}

#######################
# Edit these variables.
#######################
export OMP_NUM_THREADS=18
export REQUESTS_CA_BUNDLE=/usr/local/lib/python3.8/site-packages/certifi/cacert.pem
export CURL_CA_BUNDLE=/etc/ssl/certs/ca-certificates.crt

#######################
# Start script timer
SECONDS=0
# Temp file location
DIRECTORY=$$
OUTDIR=/data/scratch/${USER}/${MODEL_NAME}
mkdir -p ${OUTDIR}
#source ${ACTIVATE}
echo "Model: " ${MODEL_NAME} > ${OUTPUT_PATH} 2>&1
echo "Date: " $(date +%m/%d/%y) >> ${OUTPUT_PATH} 2>&1
echo "Time: " $(date +%H:%M) >> ${OUTPUT_PATH} 2>&1
apt list --installed sambaflow >> ${OUTPUT_PATH} 2>&1
cd ${OUTDIR}
#######################
echo "Machine State Before: " >> ${OUTPUT_PATH} 2>&1
/opt/sambaflow/bin/snfadm -l inventory >> ${OUTPUT_PATH} 2>&1
#######################
export SN_NUM_THREADS=32

if [ $1 -eq 256 ] ; then
  BATCH_SIZE=256
elif [ $1 -eq 128 ] ; then
  BATCH_SIZE=128
elif [ $1 -eq 64 ] ; then
  BATCH_SIZE=64
elif [ $1 -eq 32 ] ; then
  BATCH_SIZE=32
elif [ $1 -eq 16 ] ; then
  BATCH_SIZE=16
else
  echo "Batchsize $1 is invalid use 16,32,64,or 128,256" $2 >> ${OUTPUT_PATH} 2>&1
  exit 1
fi

if [ ! -e  ${OUTDIR}/${MODEL_NAME}/${MODEL_NAME}.pef ] ; then
  echo "COMPILE START AT ${SECONDS}" >> ${OUTPUT_PATH} 2>&1
  export GAS=1
  
  export CC=compiler_configs_gpt1dot5b_perf.json
  #env | grep PYTHONPATH >> ${OUTPUT_PATH} 2>&1
  COMMAND="python /opt/sambaflow/apps/nlp/transformers_on_rdu/transformers_hook.py compile --pef-name=${MODEL_NAME} --output-folder=${OUTDIR} --module_name gpt2_pretrain --task_name clm --max_seq_length 1024 -b $BATCH_SIZE  --output_dir=${OUTDIR}/hf_gpt1dot5b_ss1k_gas_${GAS}_bs${BATCH_SIZE}  --overwrite_output_dir --do_train  --per_device_train_batch_size ${BATCH_SIZE}   --tokenizer_name gpt2 --model_name gpt2 --mac-v2 --non_split_head --mac-human-decision /opt/sambaflow/apps/nlp/transformers_on_rdu/human_decisions_gm/mac_v2_overrides/gpt2_48_enc_full_recompute_training_spatialmapping_tiling16_clmerge_gm_pardp2_lnsd.json --compiler-configs-file /opt/sambaflow/apps/nlp/transformers_on_rdu/human_decisions_gm/compiler_configs/$CC --skip_broadcast_patch --config_name /opt/sambaflow/apps/nlp/transformers_on_rdu/customer_specific/mv/configs/gpt2_config_xl_50260.json --no_index_select_patch --weight_decay 0.1  --max_grad_norm_clip 1.0 --num-tiles 4 --enable-stochastic-rounding"

                
  echo "COMPILE COMMAND: $COMMAND" >> ${OUTPUT_PATH} 2>&1
  eval $COMMAND >> ${OUTPUT_PATH} 2>&1
  echo "COMPILE END AT ${SECONDS}" >> ${OUTPUT_PATH} 2>&1
fi
#######################
echo "RUN" >> ${OUTPUT_PATH} 2>&1
/usr/local/bin/sbatch --output=${HOME}/slurm-%A.out --ntasks 1 --gres=rdu:8 --ntasks-per-node 16  --nodes 1 --nodelist $(hostname) --cpus-per-task=8  /home/$(whoami)/apps/nlp/Gpt1.5B_single/Gpt1.5B_base_single_run.sh $BATCH_SIZE $2 >> ${OUTPUT_PATH} 2>&1

echo "Machine state After: " >> ${OUTPUT_PATH} 2>&1
/opt/sambaflow/bin/snfadm -l inventory >> ${OUTPUT_PATH} 2>&1
echo "Duration: " $SECONDS >> ${OUTPUT_PATH} 2>&1
