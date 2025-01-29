# Model Inference with OpenVINO

OpenVINO is a library developed by Intel specifically designed for accelerating inference of ML models on their CPU and GPU hardware. 
This page contains build and run instructions for Python, but please refer to the [OpenVINO GitHub page](https://github.com/openvinotoolkit/openvino) for more information.

## Installing the OpenVINO Python Runtime and CLI Tools
OpenVINO does not come with the default frameworks module on Aurora, but it can be installed manually within a Python virtual environment as shown below:
```bash
module load frameworks
python -m venv --clear /path/to/_ov_env --system-site-packages
source /path/to/_ov_env/bin/activate
pip install openvino==2024.4.0
pip install openvino-dev==2024.4.0
```

It is recommended that the path to the virtual environment be in the user's project space on [Flare](../../data-management/lustre/flare.md).

## Model Converter

The first suggested step is to convert the model from one of the ML frameworks into OpenVINO's Intermediate Representation (IR). 
This consists of an `.xml` file which describes the network topology and a `.bin` file which contains the weights and biases in binary format. 
The conversion can be done from the command line with `ovc` or using the Python API `openvino.convert_model()`.
Note that PyTorch models cannot be converted directly with `ovc` and need to be converted to ONNX format first.
You can find more information on the conversion process on [OpenVINO's documentation page](https://docs.openvino.ai/2024/openvino-workflow/model-preparation/convert-model-to-ir.html).

The following code snippet demonstrates how to use the Python API to convert the ResNet50 model from TorchVision and save the OpenVINO IR.
```python
import openvino as ov
import torch
from torchvision.models import resnet50

model = resnet50(weights='DEFAULT')
input_data = torch.rand(1, 3, 224, 224)

ov_model = ov.convert_model(model, example_input=input_data)
ov.save_model(ov_model, 'resnet50.xml')
```

Information on using the CLI conversion tool can be found by running `ovc -h`, which will save the model in IR format by default.

Note that by default, both `ovc` and `openvino.save_model()` perform compression of the model weights to FP16. This reduces the memory needed to store the model and can provide an increase in performance. 
To disable this feature, use:
```python
ov.save_model(ov_model, 'resnet50.xml', compress_to_fp16=False)
```

or

```bash
ovc model.onnx --compress_to_fp16=False
```

## Benchmark App

Before writing a script or program to perform inference with the OpenVINO runtime, the performance of the model can be tested with the CLI tool `benchmark_app`. 

A minimal example to run on a single Intel Max 1550 tile is shown below:
```bash
benchmark_app -m resnet50.xml -hint latency -d GPU.0 -data_shape [1,3,224,224]
```

which returns a series of information on the parameters set for the benchmark tests and the performance of the tests. The last few lines of the output are shown below.

```bash
[ INFO ] Execution Devices:['GPU.0']
[ INFO ] Count:            42847 iterations
[ INFO ] Duration:         60001.96 ms
[ INFO ] Latency:
[ INFO ]    Median:        1.38 ms
[ INFO ]    Average:       1.38 ms
[ INFO ]    Min:           1.35 ms
[ INFO ]    Max:           21.31 ms
[ INFO ] Throughput:   714.09 FPS
```

Note that `benchmark_app` takes a number of additional configuration options which are listed by running `benchmark_app -h`. 

## Inference with Python OpenVINO API

Inference can be performed by invoking the compiled model directly or using the OpenVINO Runtime API explicitly to create inference requests.

An example of performing direct inference with the compiled model is shown below. 
This leads to compact code, but it performs a single synchronous inference request. 
Future calls to the model will reuse the same inference request created, thus experiencing less overhead.
```python
import openvino as ov
import torch

core = ov.Core()
compiled_model = core.compile_model("resnet50.xml")

input_data = torch.rand((1, 3, 224, 224))
results = compiled_model(input_data)[0]
```

By default, OpenVINO performs inference with FP16 precision on GPU, but the precision and device can be selected with hints, such as:
```python
import openvino.properties.hint as hints
core.set_property(
    "GPU.0",
    {hints.execution_mode: hints.ExecutionMode.ACCURACY},
)
```

More information on the available hints can be found on the [OpenVINO documentation page](https://docs.openvino.ai/2024/openvino-workflow/running-inference/optimize-inference/precision-control.html).

Other than the direct call to the model, the Runtime API can be used to create inference requests and control their execution.
For this approach, we refer the user to the [OpenVINO documentation page](https://docs.openvino.ai/2024/openvino-workflow/running-inference/integrate-openvino-with-your-application/inference-request.html).