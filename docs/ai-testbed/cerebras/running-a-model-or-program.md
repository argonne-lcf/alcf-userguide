# Running a Model/Program

## Getting Started

#### Job submission and queuing

Cerebras jobs are initiated and tracked automatically within the Python framework in **modelzoo.common.pytorch.run_utils**. This framework interacts with the Cerebras cluster management node.

#### Login nodes

Jobs are launched from **login** nodes.
If you expect a loss of an internet connection for any reason, for long-running jobs we suggest logging into a specific login node and using either **screen** or **tmux** to create persistent command line sessions.  For details use:2

```bash
man screen
# or
man tmux
```

## Running jobs on the wafer

Follow these instructions to compile and train a small (111m parameters) GPT3 model.

### Cerebras virtual environments

First, make a virtual environment for Cerebras for PyTorch.
See [Customizing Environments](./customizing-environment.md) for the procedures for making PyTorch virtual environments for Cerebras.
If an environment is made in ```~/R_2.4.0/```, it would be activated as follows:
```console
source ~/R_2.4.0/venv_cerebras_pt/bin/activate
```

### Clone the Cerebras modelzoo

```console
mkdir ~/R_2.4.0
cd ~/R_2.4.0
git clone https://github.com/Cerebras/modelzoo.git
cd modelzoo
git tag
git checkout Release_2.4.0
```
## Running a Pytorch sample

### Activate your PyTorch virtual environment, install modelzoo requirements, and change to the working directory

```console
source ~/R_2.4.0/venv_cerebras_pt/bin/activate
pip install -r ~/R_2.4.0/modelzoo/requirements.txt
cd ~/R_2.4.0/modelzoo/src/cerebras/modelzoo/models/nlp/gpt3
```

Next, copy a sample config file. This is for a small GPT3 model, modified to use a preprocessed dataset and to run for fewer steps. 

```text
cp /software/cerebras/dataset/OWT/Pytorch/111m_modified.yaml configs/Cerebras_GPT/111m_modified.yaml
```

### Running a sample PyTorch training job

To run the sample:

```console
export MODEL_DIR=model_dir_gpt3_111m
# deletion of the model_dir is only needed if sample has been previously run
if [ -d "$MODEL_DIR" ]; then rm -Rf $MODEL_DIR; fi
python run.py CSX --job_labels name=gpt3_111m --params configs/Cerebras_GPT/111m_modified.yaml --num_csx=1 --mode train --model_dir $MODEL_DIR --mount_dirs /home/ /software --python_paths /home/$(whoami)/R_2.4.0/modelzoo/src --compile_dir $(whoami) |& tee mytest.log
```

A successful GPT3 (111m parameters) PyTorch training run should finish with output resembling the following:

```text
2023-11-29 18:13:13,048 INFO:   | Train Device=CSX, Step=1950, Loss=2.28834, Rate=397.31 samples/sec, GlobalRate=433.98 samples/sec
2023-11-29 18:13:13,555 INFO:   | Train Device=CSX, Step=2000, Loss=2.34778, Rate=395.69 samples/sec, GlobalRate=431.83 samples/sec
2023-11-29 18:13:13,555 INFO:   Saving checkpoint at step 2000
2023-11-29 18:13:17,242 INFO:   Saved checkpoint model_dir/checkpoint_2000.mdl
2023-11-29 18:13:55,517 INFO:   Heartbeat thread stopped for wsjob-fpwqt7maq8a5mxvblwwzbu.
2023-11-29 18:13:55,523 INFO:   Training completed successfully!
2023-11-29 18:13:55,523 INFO:   Processed 4000 sample(s) in 51.230697212 seconds.
```




