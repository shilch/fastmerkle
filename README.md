# fastmerkle

Proof of concept of bitcoin merkle tree calcuation on GPUs using OpenCL.  

**Why?**  
Some people in the bitcoin community claim that bitcoin's merkle tree does not scale;
this code gives some ideas of how fast we can calculate Gigabyte block merkle trees today using GPUs instead of CPUs.  
If your computer doesn't have a dedicated graphics card, you might still be able to use the (slower) GPU on your processor.

**How to compile?**  

First off, you need to install OpenCL.  
You can skip this step if you are on macOS, as it ships with OpenCL.

Any of these links below should work. It's recommend to install your GPU vendor's library.  
Download links for OpenCL library:

**NVIDIA**: https://developer.nvidia.com/cuda-downloads  
**AMD**: https://github.com/GPUOpen-LibrariesAndSDKs/OCL-SDK/releases  
**Intel**: https://software.intel.com/en-us/articles/opencl-drivers  

If you have installed make and cmake, you can just run `cmake . && make` in the root directory of this repository. You'll get a `fastmerkle` file.  
Alternatively, with gcc:  

**macOS**: `gcc main.c -framework OpenCL -o fastmerkle`  
**Linux** and **Windows**: `gcc main.c -lOpenCL -o fastmerkle`
