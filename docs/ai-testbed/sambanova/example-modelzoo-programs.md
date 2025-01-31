# SambaNova Model Zoo samples

The [SambaNova Model Zoo](https://github.com/sambanova/modelzoo/tree/main) is SambaNova's new github repository for delivering RDU-compatible source code, including example applications for compiling and running models on SambaNova hardware.

In the ALCF SN30 cluster, the Model Zoo samples run inside of Singularity containers. The Singularity image includes support for compiling and running models.

The procedures in this section are drawn from [Walkthrough—​Inference and Fine-tuning with Llama2 7B for Chat](https://github.com/sambanova/modelzoo/blob/main/examples/nlp/README.adoc). 
The Model Zoo inference sample used as an example in this section is described in more detail here [About the Generation Example Apps](https://github.com/sambanova/modelzoo/tree/main/examples/nlp/text_generation). This readme (on GitHub) also describes the changes made to a CPU mode sample to run on an RDU.
The original python scripts and scripts converted to run on an RDU are also supplied in the modelzoo.  
[cpu_generate_text.py](https://github.com/sambanova/modelzoo/blob/main/examples/nlp/text_generation/cpu_generate_text.py)  
[rdu_generate_text.py](https://github.com/sambanova/modelzoo/blob/main/examples/nlp/text_generation/rdu_generate_text.py)  
and  
[cpu_train_llm.py](https://github.com/sambanova/modelzoo/blob/main/examples/nlp/training/cpu_train_llm.py)  
[rdu_train_llm.py](https://github.com/sambanova/modelzoo/blob/main/examples/nlp/training/rdu_train_llm.py)  
[rdu_train_llm_dp.py](https://github.com/sambanova/modelzoo/blob/main/examples/nlp/training/rdu_train_llm_dp.py)  

## Setup
### Cloning the Model Zoo Repository

Clone the repo in your usual location. 
```
mkdir ~/sambanova
cd ~/sambanova
git clone https://github.com/sambanova/modelzoo.git
```
Note: your home directory is mounted by default in the singularity containers.

### Starting a container:

Change directory to your Model Zoo clone, and set an environment variable to be host SambaNova runts version, then start the container. This example binds a directory containing an OpenWebText dataset. 
```
cd ~/sambanova/modelzoo
export TARGET_SAMBAFLOW_VERSION=$((rpm -q sambanova-runtime 2>/dev/null || dpkg -s sambanova-runtime 2>/dev/null) | egrep -m 1 -o "[0-9]+\.[0-9]+\.[0-9]+")
echo $TARGET_SAMBAFLOW_VERSION
# should be of the form 1.19.1
./start_container.sh -b /data/ANL/openwebtext/hdf5/hdf5:/opt/datasets/openweb_hdf54096/ -b  /software:/software / /software/sambanova/singularity/images/llm-modelzoo/Modelzoo/ModelzooDevbox_0.2.0.sif 
```
Container startup output should look like:
```
APP_ROOT: /home/arnoldw/sambanova/modelzoo
Using singularity with image /software/sambanova/singularity/images/llm-modelzoo/Modelzoo/ModelzooDevbox_1.sif

Running singularity instance with name: devbox_arnoldw_1724873417
Singularity start command: singularity instance start --writable-tmpfs --bind /home/arnoldw/github.com/sambanova/modelzoo:/opt/modelzoo --bind /tmp:/tmp --bind /data/ANL/openwebtext/hdf5/hdf5:/opt/datasets/openweb_hdf54096/ --bind /software/models/:/opt/ckpts/ --bind /dev/hugepages:/dev/hugepages --bind /opt/sambaflow/pef/:/opt/sambaflow/pef/ --bind /opt/sambaflow/runtime/:/opt/sambaflow/runtime/ --bind /var/lib/sambaflow/ccl/ccl_config.db:/var/lib/sambaflow/ccl/ccl_config.db --bind /var/snml.sock:/var/snml.sock --bind /opt/sambanova/lib/python3.8/site-packages/pysnml:/opt/sambanova/lib/python3.8/site-packages/pysnml --bind /opt/sambanova/lib/python3.8/site-packages/pysnrdureset:/opt/sambanova/lib/python3.8/site-packages/pysnrdureset --bind /opt/sambanova/lib/python3.8/site-packages/pysnrdutools:/opt/sambanova/lib/python3.8/site-packages/pysnrdutools --bind /opt/sambanova/lib/python3.8/site-packages/sambaruntime:/opt/sambanova/lib/python3.8/site-packages/sambaruntime /software/sambanova/singularity/images/llm-modelzoo/Modelzoo/ModelzooDevbox_1.sif devbox_arnoldw_1724873417
INFO:    instance started successfully
Singularity instance devbox_arnoldw_1724873417 started
Run command: singularity exec instance://devbox_arnoldw_1724873417 /bin/bash
Singularity> 
```

To list all running containers (while outside a container, e.g. a different SSH session):
```
$ singularity instance list
INSTANCE NAME                PID        IP    IMAGE
devbox_arnoldw_1724873417    1649294          /software/sambanova/singularity/images/llm-modelzoo/Modelzoo/ModelzooDevbox_1.sif
```
To re-enter an exited but still-running container (while outside a container):
```
$ singularity exec instance://devbox_arnoldw_1724873417 /bin/bash
Singularity> 
```

To stop all your running containers (while outside a container):
```
$ singularity instance stop devbox_<youruserid>_*
```

### Set up the Python environment in the container

```
cd ~/sambanova/modelzoo/
pip install -r requirements/requirements.txt 
pip install --upgrade pip
pip install -e . 
```

### Optionally, download the Hugging Face model for Llama-2-7b

This model is also avaiable in `/software/models/Llama-2-7b-hf/`
First, create a Hugging Face account at https://huggingface.co/join if you do not already have one.  
Go to [meta-llama/Llama-2-7b-hf](https://huggingface.co/meta-llama/Llama-2-7b-hf[meta-llama/Llama-2-7b-hf) and accept the terms of use for Llama2 7B.  
You will need to wait (minutes at least) until the request is proccessed.  
In your [Hugging Face account settings](https://huggingface.co/settings/tokens), generate a [user access token](link:https://huggingface.co/docs/hub/en/security-tokens). A read-only token works. Record the token such that it can easily be copy-pasted in the future.
```
# if working in an environment (e.g. laptop) where git-lfs is not installed, 
# sudo apt install git-lfs 
git lfs install # Only needs to be done once 
cd ~/sambanova
git clone https://huggingface.co/meta-llama/Llama-2-7b-hf
# Enter your HF user name and user access token (copy;paste) when prompted.
```

## Text generation sample
### Compile a text generation sample that uses the HF model

Compile a LLaMA-7b text generation sample (using the Hugging Face model). This will take 20 minutes

```
cd ~/sambanova
# or ./Llama-2-7b-hf if downloaded
python ./modelzoo/examples/nlp/text_generation/rdu_generate_text.py \
command=compile \
checkpoint.model_name_or_path=/software/models/Llama-2-7b-hf/ \
samba_compile.output_folder=/home/$(whoami)/sambanova/out_generation \
+samba_compile.target_sambaflow_version=LATEST
```

Note: each compile will add a new subdirectory to the ouput folder (`/home/$(whoami)/sambanova/out_generation`), containing compile artifacts. The folder can be deleted when testing is complete;

### Run the text generation sample 

Run the sample, using the `.pef` binary created by the compile.
Note: The expression in the command line finds the most recent pef file.
<!--
$(find ./out_generation/ -type f -name "*.pef" -printf "%T@ %p\n" | sort -n | tail -n1 | awk '{print $2}')
$(find ./out_generation/$(ls -lart ~/output_llama/ | grep rdu | tail -n 1 | awk '{print $9}') -name "*.pef")
-->

```
cd ~/sambanova
export PEF=$(find /home/$(whoami)/sambanova/out_generation -type f -name "*.pef" -printf "%T@ %p\n" | sort -n | tail -n1 | awk '{print $2}')
# or ./Llama-2-7b-hf if downloaded
python ./modelzoo/examples/nlp/text_generation/rdu_generate_text.py \
  command=run \
  checkpoint.model_name_or_path=/software/models/Llama-2-7b-hf/ \
  samba_run.pef=${PEF}
```
<!---
Note, 2024/09/04, relative path to pef file is not working. Can't find file, thought the full pathname
in the error message exists according to ls
--->

The end of the console output should resemble the following:
```
Generating 32 tokens ...
Decoding ...
Completion:
[', there was a little boy who lived in a small town.\nHe was a good boy, but sometimes he had a hard time following the rules.\n']

latencies
    time to first token 1.1981s
    tokens,  excluding first token 0.3330s
    tokens,  overall 0.3600s
    Total Latency 1.5310s
throughputs
    tokens/second excluding first token 3.0032
    tokens/second overall 2.7777
Singularity> 
```

## Model Finetuning Sample

Fine-tune the Llama2 7B model using a chat dataset.


### Data preparation
NOTE: These data preparation steps should be performed on a SambaNova node, **and not in a singularity container**.
#### Install the Generative Data Prep package in a virtualenv
```
cd ~/sambanova
git clone https://github.com/sambanova/generative_data_prep.git
cd generative_data_prep
python -m venv gdp_venv
source gdp_venv/bin/activate
pip install .
cd ~/sambanova

```

#### Download UltraChat from its Hugging Face page
Make sure that you have git lfs installed, with `git lfs install`
```
cd ~/sambanova
git clone https://huggingface.co/datasets/stingning/ultrachat
```

#### Convert the dataset to the `.jsonl` format
```
cd ~/sambanova
source generative_data_prep/gdp_venv/bin/activate
# This step makes a single jsonl file
python ./modelzoo/examples/nlp/training/utils/convert_ultrachat.py -src ultrachat/ -dest ultrachat_processed.jsonl
# get a small subset to keep the 1 epoch runtime down.
mv ~/sambanova/ultrachat_processed.jsonl ~/sambanova/ultrachat_processed_full.jsonl
head -1000 ~/sambanova/ultrachat_processed_full.jsonl > ~/sambanova/ultrachat_processed.jsonl
# This step makes a directory of hdf5 files from the single jsonl file
export TOKENIZER="meta-llama/Llama-2-7b-hf"
export MAX_SEQ_LENGTH=4096
python -m generative_data_prep pipeline --input_file_path=./ultrachat_processed.jsonl --output_path=./ultrachat_dialogue --pretrained_tokenizer=${TOKENIZER} --max_seq_length=${MAX_SEQ_LENGTH}
deactivate
```

### Compile a sample that finetunes the HF model

#### Start container
If you are not already in a Singularity container (with the pre-reqs installed),  
start a new Model Zoo Singularity container with
```
cd ~/sambanova/modelzoo
export TARGET_SAMBAFLOW_VERSION=$((rpm -q sambanova-runtime 2>/dev/null || dpkg -s sambanova-runtime 2>/dev/null) | egrep -m 1 -o "[0-9]+\.[0-9]+\.[0-9]+")
echo $TARGET_SAMBAFLOW_VERSION
# should be of the form 1.19.1
./start_container.sh -b /data/ANL/openwebtext/hdf5/hdf5:/opt/datasets/openweb_hdf54096/ -b  /software:/software /software/sambanova/singularity/images/llm-modelzoo/Modelzoo/ModelzooDevbox_1.sif
```
or use an existing container with instructions at [starting a container](#starting-a-container)

#### Install pre-reqs
Then install the pre-reqs into the container with
```
cd ~/sambanova/modelzoo/
pip install -r requirements/requirements.txt 
pip install --upgrade pip
pip install -e . 
```

#### Compile the sample for fine tuning
```
cd ~/sambanova
export CHECKPOINT=/software/models/Llama-2-7b-hf/ # or ./Llama-2-7b-hf
export MAX_SEQ_LENGTH=4096
export BATCH_SIZE=8
export ARCH=sn30
python modelzoo/examples/nlp/training/rdu_train_llm.py \
    command=compile \
    checkpoint.config_name=${CHECKPOINT} \
    model.max_seq_length=${MAX_SEQ_LENGTH} \
    training.batch_size=${BATCH_SIZE} \
    samba_compile.arch=${ARCH} \
    samba_compile.output_folder=/home/$(whoami)/sambanova/out_train \
    +samba_compile.target_sambaflow_version=LATEST
```

Note: each compile will add a new subdirectory to the ouput folder (`/home/$(whoami)/sambanova/out_train`), containing compile artifacts. The folder can be deleted when testing is complete;

#### Run finetuning using generated pef file

This will run for 1 full epoch and takes 1 hour to execute, using a single RDU.
It uses the config file `modelzoo/examples/nlp/training/config/base_config_rdu.yaml`

```
cd ~/sambanova
export CHECKPOINT=/software/models/Llama-2-7b-hf/ # or ./Llama-2-7b-hf
export MAX_SEQ_LENGTH=4096
export DATASET=./ultrachat_dialogue;  # or container path to dataset
# Finds most recent pef file in tree
export PEF=$(find /home/$(whoami)/sambanova/out_train -type f -name "*.pef" -printf "%T@ %p\n" | sort -n | tail -n1 | awk '{print $2}')
python -u modelzoo/examples/nlp/training/rdu_train_llm.py \
    command=run \
    checkpoint.model_name_or_path=${CHECKPOINT} \
    model.max_seq_length=${MAX_SEQ_LENGTH} \
    samba_run.pef=${PEF} \
    training.dataset=${DATASET}
```

The end of the console output should resemble the following if run for a full epoch:
```
Targeting samba-runtime v4.2.5. Samba is running with --target-runtime-version=1.3.10 on a system with installed runtime None.

Log ID initialized to: [arnoldw][python][1003] at /var/log/sambaflow/runtime/sn.log
Loading dataset for epoch 1...

Number of epochs: 1
Batch size: 8
Number of batches (steps): 1,143

Starting training for epoch 1...
Epoch [1/1], Step [1/1143], Loss: 0.8184
Epoch [1/1], Step [2/1143], Loss: 0.2452
Epoch [1/1], Step [3/1143], Loss: 0.3727
Epoch [1/1], Step [4/1143], Loss: 0.2945
...
Epoch [1/1], Step [1134/1143], Loss: 0.2529
Epoch [1/1], Step [1135/1143], Loss: 0.2713
Epoch [1/1], Step [1136/1143], Loss: 0.2669
Epoch [1/1], Step [1137/1143], Loss: 0.2144
Epoch [1/1], Step [1138/1143], Loss: 0.2129
Epoch [1/1], Step [1139/1143], Loss: 0.2229
Epoch [1/1], Step [1140/1143], Loss: 0.2263
Epoch [1/1], Step [1141/1143], Loss: 0.2434
Epoch [1/1], Step [1142/1143], Loss: 0.2131
Epoch [1/1], Step [1143/1143], Loss: 0.1626
Finished training.
Saving checkpoint...
Checkpoint saved at finetuned_model/
Saving summary...
Summary saved at finetuned_model/summary.txt
Singularity> 
```
