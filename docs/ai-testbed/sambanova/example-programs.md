# Example Programs

You can use the link to the tutorials on the SambaNova GitHub site or the examples on the compute node (as explained below).

* Find the tutorials on the [SambaNova GitHub site](https://github.com/sambanova/tutorials/tree/main). If you use those instructions, ensure that you still use the steps for accessing the SN compute node, setting the required environment and compiling and running the applications as described in this documentation. 
* Use the examples of well-known simple AI applications under the path: `/opt/sambaflow/apps/starters`, on all SambaNova compute nodes, as discussed on this page.  

Make a copy of this to your home directory:

```bash
cd ~/
mkdir apps
cp -r /opt/sambaflow/apps/starters apps/starters
```

Deactivate any active conda environment. If you have conda installed and a conda environment is active, you will see something like `(base)` at the beginning of the command prompt. If so, you will need to deactivate it with `conda deactivate`. Conda is not used on the SambaNova SN30 cluster. 

## LeNet

Change directory

```bash
cd ~/apps/starters/lenet
```

### Common Arguments

Below are some of the common arguments used across most of the models in the example code.

| Argument               | Default   | Help                           |
|------------------------|-----------|--------------------------------|
| -b                     | 1         | Batch size for training        |
|                        |           |                                |
| -n,                    | 100       | Number of iterations to run    |
| --num-iterations       |           | the pef for                    |
|                        |           |                                |
| -e,                    | 1         | Number epochs for training     |
| --num-epochs           |           |                                |
|                        |           |                                |
| --log-path             | 'check    | Log path                       |
|                        | points'   |                                |
|                        |           |                                |
| --num-workers          | 0         | Number of workers              |
|                        |           |                                |
| --measure-train-       | None      | Measure training performance   |
| performance            |           |                                |
|                        |           |                                |

### LeNet Arguments

| Argument               | Default   | Help                           |
|------------------------|-----------|--------------------------------|
| --lr                   | 0.01      | Learning rate for training     |
|                        |           |                                |
| --momentum             | 0.0       | Momentum value for training    |
|                        |           |                                |
| --weight-decay         | 0.01      | Weight decay for training      |
|                        |           |                                |
| --data-path            | './data'  | Data path                      |
|                        |           |                                |
| --data-folder          | 'mnist_   | Folder containing mnist data   |
|                        | data'     |                                |
|                        |           |                                |

<!---
Establish the Environment

```bash
source /opt/sambaflow/apps/starters/lenet/venv/bin/activate
```
--->

> **Note**:  If you receive an \"HTTP error\" message on any of the
following commands, run the command again. Such errors (e.g 503) are
commonly an intermittent failure to download a dataset.

Run these commands to compile and train the LeNet model:

```bash
srun python lenet.py compile -b=1 --pef-name="lenet" --output-folder="pef"
srun python lenet.py run --pef="pef/lenet/lenet.pef"
```

Alternatively to use Slurm sbatch, create submit-lenet-job.sh with the following
contents:

```bash
#!/bin/sh

python lenet.py compile -b=1 --pef-name="lenet" --output-folder="pef"
python lenet.py run --pef="pef/lenet/lenet.pef"
```

Then

```bash
mkdir -p pef/lenet
sbatch --output=pef/lenet/output.log submit-lenet-job.sh
```

Squeue will give you the queue status.

```bash
squeue
# One may also...
watch squeue
```

One may see the run log using:

```bash
cat pef/lenet/output.log
```

## MNIST - Feed Forward Network
<!---
Establish the Environment

```bash
source /opt/sambaflow/apps/starters/ffn_mnist/venv/bin/activate
```
--->

Change directory

```bash
cd ~/apps/starters/ffn_mnist/
```

Commands to run MNIST example:

```bash
srun python ffn_mnist.py  compile -b 1 --pef-name="ffn_mnist" --mac-v2
srun python ffn_mnist.py  run -b 1 -p out/ffn_mnist/ffn_mnist.pef
```

To run the same using Slurm sbatch, create and run the submit-ffn_mnist-job.sh with the following contents.

```bash
#!/bin/sh
python ffn_mnist.py  compile -b 1 --pef-name="ffn_mnist" --mac-v2
python ffn_mnist.py  run -b 1 -p out/ffn_mnist/ffn_mnist.pef
```

```bash
mkdir -p pef/ffn_mnist
sbatch --output=pef/ffn_mnist/output.log submit-ffn_mnist-job.sh
```

## Logistic Regression

<!---
Establish the Environment

```bash
source /opt/sambaflow/apps/starters/logreg/venv/bin/activate
```
--->

Change directory

```bash
cd ~/apps/starters/logreg
```

### Logistic Regression Arguments

This is not an exhaustive list of arguments.

Arguments

| Argument            | Default     | Help                         | Step     |
|---------------------|-------------|------------------------------|----------|
| --lr                | 0.001       | Learning rate for training   | Compile  |
|                     |             |                              |          |
| --momentum          | 0.0         | Momentum value for training  | Compile  |
|                     |             |                              |          |
| --weight-decay      | 1e-4        | Weight decay for training    | Compile  |
|                     |             |                              |          |
| --num-features      | 784         | Number features for training | Compile  |
|                     |             |                              |          |
| --num-classes       | 10          | Number classes for training  | Compile  |
|                     |             |                              |          |
| --weight-norm       | na          | Enable weight normalization  | Compile  |
|                     |             |                              |          |

Run these commands:

```bash
srun python logreg.py compile --pef-name="logreg" --output-folder="pef"
srun python logreg.py run --pef="pef/logreg/logreg.pef"
```

To use Slurm, create submit-logreg-job.sh with the following contents:

```bash
#!/bin/sh
python logreg.py compile --pef-name="logreg" --output-folder="pef"
python logreg.py run --pef="pef/logreg/logreg.pef"
```

Then

```bash
mkdir -p pef/logreg
sbatch --output=pef/logreg/output.log submit-logreg-job.sh
```

The output, pef/logreg/output.log, will look something like this:

```text
2023-03-08 21:18:25.168190: I tensorflow/core/platform/cpu_feature_guard.cc:193] This TensorFlow binary is optimized with oneAPI Deep Neural Network Library (oneDNN) to use the following CPU instructions in performance-critical operations:  AVX2 FMA
To enable them in other operations, rebuild TensorFlow with the appropriate compiler flags.
2023-03-08 21:18:25.334389: W tensorflow/compiler/xla/stream_executor/platform/default/dso_loader.cc:64] Could not load dynamic library 'libcudart.so.11.0'; dlerror: libcudart.so.11.0: cannot open shared object file: No such file or directory
2023-03-08 21:18:25.334430: I tensorflow/compiler/xla/stream_executor/cuda/cudart_stub.cc:29] Ignore above cudart dlerror if you do not have a GPU set up on your machine.
2023-03-08 21:18:26.422458: W tensorflow/compiler/xla/stream_executor/platform/default/dso_loader.cc:64] Could not load dynamic library 'libnvinfer.so.7'; dlerror: libnvinfer.so.7: cannot open shared object file: No such file or directory
2023-03-08 21:18:26.422701: W tensorflow/compiler/xla/stream_executor/platform/default/dso_loader.cc:64] Could not load dynamic library 'libnvinfer_plugin.so.7'; dlerror: libnvinfer_plugin.so.7: cannot open shared object file: No such file or directory
2023-03-08 21:18:26.422709: W tensorflow/compiler/tf2tensorrt/utils/py_utils.cc:38] TF-TRT Warning: Cannot dlopen some TensorRT libraries. If you would like to use Nvidia GPU with TensorRT, please make sure the missing libraries mentioned above are installed properly.
[Info][SAMBA]# Placing log files in /home/wilsonb/apps/starters/logreg/pef/logreg/logreg.samba.log
[Info][MAC]# Placing log files in /home/wilsonb/apps/starters/logreg/pef/logreg/logreg.mac.log
...

Epoch [1/1], Step [10000/60000], Loss: 0.4642
Epoch [1/1], Step [20000/60000], Loss: 0.4090
Epoch [1/1], Step [30000/60000], Loss: 0.3863
Epoch [1/1], Step [40000/60000], Loss: 0.3703
Epoch [1/1], Step [50000/60000], Loss: 0.3633
Epoch [1/1], Step [60000/60000], Loss: 0.3553
Test Accuracy: 91.40  Loss: 0.3014
2023-03-08T21:19:08 : [INFO][LIB][2688517]: sn_create_session: PEF File: pef/logreg/logreg.pef
```

## UNet2D

The UNet application example is provided in the the path : `/opt/sambaflow/apps/image/segmentation/`. As any other application, we first compile and then train the model using *compile* and *run* arguments respectively.
The scripts containing the compile and run commands for UNet2D model can be accessed at [Unet2d.sh](./files/Unet2d.sh "Unet2d.sh") or at `/data/ANL/scripts/Unet2d.sh` on any SN30 compute node.

Change directory and copy files.

```bash
mkdir -p ~/apps/image/unet
cd ~/apps/image/unet
```

Copy and paste the contents of
[Unet2d.sh](./files/Unet2d.sh "Unet2d.sh")
to a file with the same name into the current directory using your favorite editor.

```bash
chmod +x Unet2d.sh
```

Run these commands for training (compile + train):

```bash
./Unet2d.sh compile <image size> <batch_size> <num of instances> <RunID>
./Unet2d.sh run <image size> <batch_size> <num of instances> <RunID>
```

The `compile` and `run` arguments of the script can only be run with number of instances equal to 1, indicating that this is a simple 4 tile run without data parallel framework.
For a image size of 256x256 and batch size 256 when running just 1 instance, the commands are provided as follows.

!!! note

    The compilation runs for over 30 minutes.

```bash
./Unet2d.sh compile 256 256 1 unet2d_single_compile
./Unet2d.sh run 256 256 1 unet2d_single_run
```

The above commands displays the file that contains the output for the execution of the above scripts, usually `/data/ANL/results/<hostname>/<userid>/<RunID>/Unet2d.out`

If we inspect the compile and run commands for the UNet application provided in the script, we see that the application is compiled with `--num-tiles 4`, which means that the entire application fits on 4 tiles or half of a RDU.
The pef generated from the compilation process of the above command is placed under `out/Unet2d/unet_train_256_256_single_4` inside the current working directory.

```bash
python ${UNET}/compile.py compile --mac-v2 --in-channels=3 --in-width=${2} --in-height=${2} --batch-size=${BS} --enable-conv-tiling --num-tiles=4 --pef-name=unet_train_${BS}_${2}_single_${NUM_TILES} --output-folder=${OUTDIR}
```

```bash
srun --nodelist $(hostname) python /opt/sambaflow/apps/image/segmentation//hook.py run --data-cache=${CACHE_DIR}  --data-in-memory --num-workers=${NUM_WORKERS} --enable-tiling  --min-throughput 395 --in-channels=3 --in-width=${2} --in-height=${2} --init-features 32 --batch-size=${BS} --epochs 10 --data-dir ${DS} --log-dir log_dir_unet_${2}_${BS}_single_${NUM_TILES} --pef=${OUTDIR}/unet_train_${BS}_${2}_single_${NUM_TILES}/unet_train_${BS}_${2}_single_${NUM_TILES}.pef
```

The performance data is located at the bottom of log file.

```console
inner train loop time : 374.6789753437042 for 10 epochs, number of global steps: 130, e2e samples_per_sec: 88.82270474202953
```

## GPT 1.5B

The GPT 1.5B application example is provided in the the path : `/opt/sambaflow/apps/nlp/transformers_on_rdu/`.
The scripts containing the `compile` and `run` commands for the GPT 1.5B model can be accessed at the path `/data/ANL/scripts/1.23.5-46/legacy_models/Gpt1.5B_base_single_compile.sh` and `/data/ANL/scripts/1.23.5-46/legacy_models/Gpt1.5B_base_single_run.sh` on any SN30 compute node. This script is compiled and run for only 1 instance and the model fits on 4 tiles or half of a RDU. The scripts are provided for reference. 

Change directory and copy files.

```bash
mkdir -p ~/apps/nlp/Gpt1.5B_single
cd ~/apps/nlp/Gpt1.5B_single
```

Copy and paste the contents of
[Gpt1.5B_base_single_compile.sh](./files/Gpt1.5B_base_single_compile.sh "Gpt1.5B_base_single_compile.sh") and [Gpt1.5B_base_single_run.sh](./files/Gpt1.5B_base_single_run.sh "Gpt1.5B_base_single_run.sh") 
to a file with the same names into the current directory using your favorite editor.

or copy the contents from `/data/ANL/scripts/Gpt1.5B_base_single_compile.sh` and `/data/ANL/scripts/Gpt1.5B_base_single_run.sh`.

```bash
cp /data/ANL/scripts/1.23.5-46/legacy_models/Gpt1.5B_base_single_compile.sh ~/apps/nlp/Gpt1.5B_single/
cp /data/ANL/scripts/1.23.5-46/legacy_models/Gpt1.5B_base_single_run.sh ~/apps/nlp/Gpt1.5B_single/
```

Run the script with batch size as an argument(shown below with an example of 32).

```bash
chmod +x Gpt1.5B_base_single_compile.sh 
./Gpt1.5B_base_single_compile.sh 32
```

The Gpt1.5B_base_single_compile.sh  script will internally call the Gpt1.5B_base_single_run.sh to perform the training. You can inspect the `compile` and `run` commands in the scripts to learn that this model trains with a batch size of 32 for 1 instance over 4 tiles. The human decision file and the compiler config file helps to optimize the compute and memory resources specific to this Gpt 1.5B model run.

```bash
python /opt/sambaflow/apps/nlp/transformers_on_rdu/transformers_hook.py compile --pef-name=GPT1.5B_base_single_32 --output-folder=/data/scratch/user/GPT1.5B_base_single_32 --module_name gpt2_pretrain --task_name clm --max_seq_length 1024 -b 32  --output_dir=/data/scratch/user/GPT1.5B_base_single_32/hf_gpt1dot5b_ss1k_gas_1_bs32  --overwrite_output_dir --do_train  --per_device_train_batch_size 32   --tokenizer_name gpt2 --model_name gpt2 --mac-v2 --non_split_head --mac-human-decision /opt/sambaflow/apps/nlp/transformers_on_rdu/human_decisions_gm/mac_v2_overrides/gpt2_48_enc_full_recompute_training_spatialmapping_tiling16_clmerge_gm_pardp2_lnsd.json --compiler-configs-file /opt/sambaflow/apps/nlp/transformers_on_rdu/human_decisions_gm/compiler_configs/compiler_configs_gpt1dot5b_perf.json --skip_broadcast_patch --config_name /opt/sambaflow/apps/nlp/transformers_on_rdu/customer_specific/mv/configs/gpt2_config_xl_50260.json --no_index_select_patch --weight_decay 0.1  --max_grad_norm_clip 1.0 --num-tiles 4 --enable-stochastic-rounding
```

```bash
COMMAND= /usr/local/bin/srun --mpi=pmi2 python /opt/sambaflow/apps/nlp/transformers_on_rdu/transformers_hook.py run  -b 32  --data_dir /data/ANL/ss1024 --pef=/data/scratch/user/GPT1.5B_base_single_32/GPT1.5B_base_single_32/GPT1.5B_base_single_32.pef --output_dir=/data/scratch/user/GPT1.5B_base_single_32/hf_gpt1dot5b_ss1k_gas_1_bs16 --module_name gpt2_pretrain --task_name clm --max_seq_length 1024  --overwrite_output_dir --do_train  --per_device_train_batch_size 32 --tokenizer_name gpt2 --model_name gpt2 --non_split_head --skip_broadcast_patch --no_index_select_patch --config_name /opt/sambaflow/apps/nlp/transformers_on_rdu/customer_specific/mv/configs/gpt2_config_xl_50260.json --max_grad_norm_clip 1.0 --skip_checkpoint --logging_steps 1 --max_steps 75000 --learning_rate 0.00025 --steps_this_run 100
```

The `sntilestat` command shows that the application runs on 4 tiles as shown below.

```bash
/XRDU_0/RDU_0/TILE_0   2.1  96.9    0.8    0.1    0.0      0.0 796481  user python /opt/sambaflow/apps/nlp/transformers_on_rdu/
/XRDU_0/RDU_0/TILE_1   2.1  96.9    0.8    0.1    0.0      0.0 796481  user python /opt/sambaflow/apps/nlp/transformers_on_rdu/
/XRDU_0/RDU_0/TILE_2   2.5  96.9    0.4    0.1    0.0      0.0 796481  user python /opt/sambaflow/apps/nlp/transformers_on_rdu/
/XRDU_0/RDU_0/TILE_3   2.5  96.9    0.4    0.1    0.0      0.0 796481  user python /opt/sambaflow/apps/nlp/transformers_on_rdu/
/XRDU_0/RDU_0/TILE_4 100.0   0.0    0.0    0.0    0.0      0.0
/XRDU_0/RDU_0/TILE_5 100.0   0.0    0.0    0.0    0.0      0.0
...

```
