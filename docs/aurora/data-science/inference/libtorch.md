# LibTorch C++ Library

LibTorch is a C++ library for Torch, with many of the APIs that are available in PyTorch. Users can find more information in the [PyTorch documentation](https://pytorch.org/cppdocs/installing.html). This is useful for integrating the Torch ML framework into traditional HPC simulation codes and therefore enables training and inference of ML models. On Aurora, the Intel Extension for PyTorch (IPEX) library is needed to access the Max 1550 GPU, which has the device name `kXPU` in LibTorch. During compilation, Intel optimizations will be activated automatically once the IPEX dynamic library is linked.

## Environment Setup

To use LibTorch on Aurora, load the ML frameworks module:

```bash
module load frameworks
```

This will also load the consistent oneAPI SDK (version 2024.2) and `cmake`.

## Torch and IPEX libraries

With the ML frameworks module loaded as shown above, run:

```bash
python -c 'import torch; print(torch.__path__[0])'
python -c 'import torch; print(torch.utils.cmake_prefix_path)'
```

to find the path to the Torch libraries, include files, and CMake files.

For the path to the IPEX dynamic library, run:

```bash
python -c 'import torch; print(torch.__path__[0].replace("torch","intel_extension_for_pytorch"))'
```

## Linking LibTorch and IPEX Libraries

When using the CMake build system, LibTorch and IPEX libraries can be linked to an example C++ application using the following `CMakeLists.txt` file:

```cmake
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
cmake_policy(SET CMP0074 NEW)
project(project-name)

find_package(Torch REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS} -Wl,--no-as-needed")
set(TORCH_LIBS ${TORCH_LIBRARIES})

find_library(IPEX_LIB intel-ext-pt-gpu PATHS ${INTEL_EXTENSION_FOR_PYTORCH_PATH}/lib NO_DEFAULT_PATH REQUIRED)
set(TORCH_LIBS ${TORCH_LIBS} ${IPEX_LIB})
include_directories(SYSTEM ${INTEL_EXTENSION_FOR_PYTORCH_PATH}/include)

add_executable(exe main.cpp)
target_link_libraries(exe ${TORCH_LIBS})

set_property(TARGET exe PROPERTY CXX_STANDARD 17)
```

and configuring the build with:

```bash
cmake \
    -DCMAKE_PREFIX_PATH=`python -c 'import torch; print(torch.utils.cmake_prefix_path)'` \
    -DINTEL_EXTENSION_FOR_PYTORCH_PATH=`python -c 'import torch; print(torch.__path__[0].replace("torch","intel_extension_for_pytorch"))'` \
    ./
make
```

## Device Introspection

Similarly to PyTorch, LibTorch provides APIs to perform introspection on the devices available on the system. The simple code below shows how to check if XPU devices are available, how many are present, and how to loop through them to discover some properties.

```cpp
#include <torch/torch.h>
#include <c10/xpu/XPUFunctions.h>

int main(int argc, const char* argv[])
{
  torch::DeviceType device;
  int num_devices = 0;
  if (torch::xpu::is_available()) {
    std::cout << "XPU devices detected" << std::endl;
    device = torch::kXPU;

    num_devices = torch::xpu::device_count();
    std::cout << "Number of XPU devices: " << num_devices << std::endl;

    for (int i = 0; i < num_devices; ++i) {
      c10::xpu::set_device(i);
      std::cout << "Device " << i << ":" << std::endl;

      c10::xpu::DeviceProp device_prop{};
      c10::xpu::get_device_properties(&device_prop, i);
      std::cout << "  Name: " << device_prop.name << std::endl;
      std::cout << "  Total memory: " << device_prop.global_mem_size / (1024 * 1024) << " MB" << std::endl;
    }
  } else {
    device = torch::kCPU;
    std::cout << "No XPU devices detected, setting device to CPU" << std::endl;
  }

  return 0;
}
```

## Model Inferencing Using the Torch API

This example shows how to perform inference with the ResNet50 model using LibTorch. First, get a JIT-traced version of the model by executing `python resnet50_trace.py` (shown below) on a compute node.

```python
import torch
import torchvision
import intel_extension_for_pytorch as ipex
from time import perf_counter

device = 'xpu'

model = torchvision.models.resnet50()
model.to(device)
model.eval()

dummy_input = torch.rand(1, 3, 224, 224).to(device)

model_jit = torch.jit.trace(model, dummy_input)
tic = perf_counter()
predictions = model_jit(dummy_input)
toc = perf_counter()
print(f"Inference time: {toc-tic}")

torch.jit.save(model_jit, f"resnet50_jit.pt")
```

Then, build `inference-example.cpp` (shown below):

```cpp
#include <torch/torch.h>
#include <torch/script.h>

int main(int argc, const char* argv[]) {
  torch::jit::script::Module model;
  try {
    model = torch::jit::load(argv[1]);
    std::cout << "Loaded the model\n";
  }
  catch (const c10::Error& e) {
    std::cerr << "error loading the model\n";
    return -1;
  }

  model.to(torch::Device(torch::kXPU));
  std::cout << "Model offloaded to GPU\n\n";

  auto options = torch::TensorOptions()
                      .dtype(torch::kFloat32)
                      .device(torch::kXPU);
  torch::Tensor input_tensor = torch::rand({1,3,224,224}, options);
  assert(input_tensor.dtype() == torch::kFloat32);
  assert(input_tensor.device().type() == torch::kXPU);
  std::cout << "Created the input tensor on GPU\n";

  torch::Tensor output = model.forward({input_tensor}).toTensor();
  std::cout << "Performed inference\n\n";

  std::cout << "Slice of predicted tensor is : \n";
  std::cout << output.slice(/*dim=*/1, /*start=*/0, /*end=*/10) << '\n';

  return 0;
}
```

and execute it with `./inference-example ./resnet50_jit.pt`.

## LibTorch Interoperability with SYCL Pipelines

The LibTorch API can be integrated with data pipelines using SYCL to operate on input and output data already offloaded to the Intel Max 1550 GPU. The code below extends the above example of performing inference with the ResNet50 model by first generating the input data on the CPU, then offloading it to the GPU with SYCL, and finally passing the device pointer to LibTorch for inference using `torch::from_blob()`, which creates a Torch tensor from a data pointer with zero-copy.

The source code for `inference-example.cpp` is modified as follows:

```cpp
#include <torch/torch.h>
#include <torch/script.h>
#include <iostream>
#include "sycl/sycl.hpp"
#include <vector>

const int N_BATCH = 1;
const int N_CHANNELS = 3;
const int N_PIXELS = 224;
const int INPUTS_SIZE = N_BATCH*N_CHANNELS*N_PIXELS*N_PIXELS;
const int OUTPUTS_SIZE = N_BATCH*N_CHANNELS;

int main(int argc, const char* argv[]) {
  torch::jit::script::Module model;
  try {
    model = torch::jit::load(argv[1]);
    std::cout << "Loaded the model\n";
  }
  catch (const c10::Error& e) {
    std::cerr << "error loading the model\n";
    return -1;
  }

  model.to(torch::Device(torch::kXPU));
  std::cout << "Model offloaded to GPU\n\n";

  // Create the input data on the host
  std::vector<float> inputs(INPUTS_SIZE);
  srand(12345);
  for (int i=0; i<INPUTS_SIZE; i++) {
    inputs[i] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
  }
  std::cout << "Generated input data on the host \n\n";

  // Move input data to the device with SYCL
  sycl::queue Q(sycl::gpu_selector_v);
  std::cout << "SYCL running on "
            << Q.get_device().get_info<sycl::info::device::name>()
            << "\n\n";
  float *d_inputs = sycl::malloc_device<float>(INPUTS_SIZE, Q);
  Q.memcpy((void *) d_inputs, (void *) inputs.data(), INPUTS_SIZE*sizeof(float));
  Q.wait();

  // Pre-allocate the output array on device and fill with a number
  double *d_outputs = sycl::malloc_device<double>(OUTPUTS_SIZE, Q);
  Q.submit([&](sycl::handler &cgh) {
    cgh.parallel_for(OUTPUTS_SIZE, [=](sycl::id<1> idx) {
      d_outputs[idx] = 1.2345;
    });
  });
  Q.wait();
  std::cout << "Offloaded input data to the GPU \n\n";

  // Convert input array to Torch tensor
  auto options = torch::TensorOptions()
                      .dtype(torch::kFloat32)
                      .device(torch::kXPU);
  torch::Tensor input_tensor = torch::from_blob(
                                 d_inputs,
                                 {N_BATCH,N_CHANNELS,N_PIXELS,N_PIXELS},
                                 options);
  assert(input_tensor.dtype() == torch::kFloat32);
  assert(input_tensor.device().type() == torch::kXPU);
  std::cout << "Created the input Torch tensor on GPU\n\n";

  // Perform inference
  torch::NoGradGuard no_grad; // equivalent to "with torch.no_grad():" in PyTorch
  torch::Tensor output = model.forward({input_tensor}).toTensor();
  std::cout << "Performed inference\n\n";

  // Copy the output Torch tensor to the SYCL pointer
  auto output_tensor_ptr = output.contiguous().data_ptr();
  Q.memcpy((void *) d_outputs, (void *) output_tensor_ptr, OUTPUTS_SIZE*sizeof(double));
  Q.wait();
  std::cout << "Copied output Torch tensor to SYCL pointer\n";

  return 0;
}
```

Note that an additional C++ flag is needed in this case, as shown below in the `cmake` command:

```bash
cmake \
    -DCMAKE_CXX_FLAGS="-std=c++17 -fsycl" \
    -DCMAKE_PREFIX_PATH=`python -c 'import torch; print(torch.utils.cmake_prefix_path)'` \
    -DINTEL_EXTENSION_FOR_PYTORCH_PATH=`python -c 'import torch; print(torch.__path__[0].replace("torch","intel_extension_for_pytorch"))'` \
    ./
```