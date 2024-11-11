# OpenCL

## Overview

OpenCL™ (Open Computing Language) is an open, royalty-free standard for
cross-platform, parallel programming of diverse accelerators found in
supercomputers, cloud servers, personal computers, mobile devices and embedded
platforms. OpenCL greatly improves the speed and responsiveness of a wide
spectrum of applications in numerous market categories including professional
creative tools, scientific and medical software, vision processing, and neural
network training and inferencing.

## Setting the environment to use OpenCL on Aurora

The Intel Programming Environment is the main environment on Aurora. The Intel
Compute Runtime is part of this environment and grants access to OpenCL.
The Intel Compute Runtime is loaded by default in your environment.

```
> module list

Currently Loaded Modules:
  1) gcc/11.2.0                    3) intel_compute_runtime/release/agama-devel-551   5) libfabric/1.15.2.0   7) cray-libpals/1.3.3
  2) mpich/51.2/icc-all-pmix-gpu   4) oneapi/eng-compiler/2022.12.30.003              6) cray-pals/1.3.3

```

## Building on Aurora

OpenCL is a C API that can be used in your application by including the
`CL/opencl.h` file:

```C
#include <CL/opencl.h>
```

Application that use the OpenCL API need to be linked to the OpenCL
loader library by using the `-lOpenCL` linker flag.

C++ bindings exist and can be used in C++ applications by including the
`CL/opencl.hpp` file:

```C++
#include <CL/opencl.hpp>
```

## OpenCL Documentation

The [OpenCL Specification](https://registry.khronos.org/OpenCL/specs/3.0-unified/pdf/OpenCL_API.pdf) and the [OpenCL Reference Pages](https://registry.khronos.org/OpenCL/sdk/3.0/docs/man/html/) are provided by [Khronos](https://www.khronos.org/opencl/).

Documentation for the C++ bindings is available here: [OpenCL C++ Bindings](https://github.khronos.org/OpenCL-CLHPP/).
