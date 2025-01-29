# Megatron-DeepSpeed

[Megatron-DeepSpeed](https://github.com/argonne-lcf/Megatron-DeepSpeed) is a scalable, highly performant library for training large language models on _any_ GPU[^any].

In particular, it retains the core 4D parallelism[^4d] functionality of the [NVIDIA / `Megatron-LM`](https://github.com/NVIDIA/Megatron-LM) library, while leveraging the [microsoft / `DeepSpeed`](https://github.com/microsoft/DeepSpeed) library for efficient scaling and [🍋 saforem2 / `ezpz`](https://github.com/saforem2/ezpz) for automated device + backend selection.

[^4d]: 4D parallelism refers to data (DP), tensor (TP), pipeline (PP), and sequence (SP) parallelism degrees of freedom.

[^any]: Megatron-DeepSpeed is designed to work on any GPU, including NVIDIA GPUs (NCCL), AMD GPUs (RCCL), and Intel XPUs (CCL).

## Getting Started

1. Clone the [argonne-lcf / `Megatron-DeepSpeed`](https://github.com/argonne-lcf/Megatron-DeepSpeed) repository:

    ```bash
    git clone https://github.com/argonne-lcf/Megatron-DeepSpeed
    cd Megatron-DeepSpeed
    ```

1. Set up your environment:

    ```bash
    export PBS_O_WORKDIR=$(pwd)
    source <(curl -s https://raw.githubusercontent.com/saforem2/ezpz/refs/heads/main/src/ezpz/bin/utils.sh)
    ezpz_setup_env
    ```

1. Install dependencies:

    1. 🍋 [saforem2 / `ezpz`](https://github.com/saforem2/ezpz):

        ```bash
        python3 -m pip install -e "git+https://github.com/saforem2/ezpz#egg=ezpz" --require-virtualenv
        ```

    1. [microsoft / `DeepSpeed`](https://github.com/microsoft/DeepSpeed):

        ```bash
        python3 -m pip install deepspeed
        ```

1. Launch training:

    ```bash
    # Before launching, `PBS_O_WORKDIR` should be set to Megatron-DeepSpeed's PATH
    # and venv inside Megatron-DeepSpeed/venv should be activated.
    TP=2 NLAYERS=10 DATA_FILE_LIST=ALCF/data-lists/aurora/books.txt bash train_aGPT_7B.sh
    ```

    This will launch a distributed pre-training run with:

    - `NLAYERS=10`: Llama style model consisting of 10 layers

      - `TP=2`: Split across 2 Tensor Parallel groups

    - `DATA_FILE_LIST`: Using the [Books corpus](https://github.com/argonne-lcf/Megatron-DeepSpeed/blob/main/ALCF/data-lists/aurora/books.txt) of the Dolma dataset

    ??? note "Overridable Options"

        This is a simple subset of the overridable options.

        The full list (as well as their default values) can be found in [ALCF / `helpers.sh`](https://github.com/argonne-lcf/Megatron-DeepSpeed/blob/main/ALCF/helpers.sh)

        - `DTYPE`: Data type
        - `DATA_FILE_LIST`: Data file list
        - `FFN_HIDDEN_SIZE`: Feedforward Neural Network projection size
        - `GRAD_ACC_STEPS`: Gradient accumulation steps
        - `HEADS`: Number of attention heads
        - `HIDDEN`: Hidden size
        - `MICRO_BATCH`: Micro batch size
        - `NO_FLASH_ATTN`: No Flash Attention
        - `NLAYERS`: Number of layers
        - `NUM_KV_HEAD`: Number of key-value heads
        - `OPT`: Optimizer
            - `adam`
            - `adam8bit`
            - `adamw`
            - `adamwschedulefree`
            - `apex.adam`
            - `apex.sgd`
            - `ds.fusedlamb`
            - `ds.onebitlamb`
            - `galoreadamw`
            - `galoreadamw8bit`
            - `galoreadamw8bitperlayer`
            - `ipex.fusedlamb`
            - `ipex.lamb`
            - `shampoo`
            - `sgd`
            - `sgdschedulefree`
            - `sophiag`
        - `PP`: Pipeline parallelism degree
        - `SEQ`: Sequence length
        - `SP`: Sequence parallelism (Ulysses) degree
        - `TP`: Tensor parallelism degree
        - `TRAIN_TOKENS`: Number of training tokens
        - `TRAIN_ITERS`: Number of training iterations
        - `USE_ACTIVATION_CHECKPOINTING`: Use activation checkpointing
        - `WEIGHT_DECAY`: Weight decay
        - `ZERO_STAGE`: Zero stage