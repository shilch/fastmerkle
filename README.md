# fastmerkle

Proof of concept of bitcoin merkle tree calcuation on GPUs using OpenCL.  

**Why?**  
Some people in the bitcoin community claim that bitcoin's merkle tree does not scale;
this code gives some ideas of how fast we can calculate Gigabyte block merkle trees today using GPUs instead of CPUs.  
If your computer doesn't have a dedicated graphics card, you might still be able to use the (slower) GPU on your processor.  
You can not run the calculations on multiple GPUs right now, but it would be trivial to implement.

**Usage**

First, run `./fastmerkle info` (`.\fastmerkle.exe info` on Windows) to get a list of supported OpenCL hardware that you can use. Example output:  
```
Platform #0
  Name:       Apple
  Version:    OpenCL 1.2 (Oct 29 2018 21:43:16)

  Device #0
    Name:     Intel(R) Core(TM) i7-8550U CPU @ 1.80GHz
    Type:     CPU

  Device #1
    Name:     Intel(R) UHD Graphics 617
    Type:     GPU
```
For any device, you have to note down the platform id and the device id. I will be using the Intel UHD Graphics, that's platform #0 and device #1. Don't pick a device of type CPU. It would be possible to run the calculations on CPUs as well, but the code is optimized for GPUs.  

Now, open the config.txt file:  
```ini
# OpenCL platform index
# Run fastmerkle with command 'info' to obtain
# information about available hardware
platform = 0

# OpenCL device to use on specified platform
# Run fastmerkle with command 'info' to obtain
# information about available hardware
device = 0

# Whether or not you want to use
# host pointers when dealing with buffers.
# Setting to 1 can increase or decrease
# performance or even crash the program.
# For integrated GPUs it's worthwhile testing
# it, for dedicated GPUs it might result into
# errors and wrong results.
use_host_ptr = 0

# Number of iterations for benchmarking
iterations = 10
```
Here you'll have to set your platform id and your device id. You can see that I already entered these two identifiers.  
You can keep the other two properties as is.

Now you can perform a short integrity check to see whether the program computes the right hashes. Run:  

`./fastmerkle file 563638.bin` (`.\fastmerkle.exe file 563638.bin` on Windows) to hash the largest bitcoin block ever found: [563638](https://blockchair.com/bitcoin-sv/block/563638).  

The merkle root hash should be this one: `a46310f75adc7a63ff5be1fbba23fc345efce5175c59e1b7a443a297f8d3219d`.  
'Mutated' should be `false`.  

If everything is working so far, you can now run the benchmark. If not, try setting `use_host_ptr` to 1.

`./fastmerkle benchmark` (`.\fastmerkle.exe benchmark` on Windows) to run a benchmark. This will take some time.

Sometimes, the operating system has issues freeing memory fast enough (the benchmark consumes a lot) in which case it crashes the program.  
You can just run `./fastmerkle custom` (`.\fastmerkle.exe custom` on Windows) and enter the number 33554432 to do one single iteration of hashing a 13.5GB block.
That's the most interesting time to know.

Assuming an average transaction size of 400 byte, you can use the following formular to calculate the block size from the number of leaves: `BLOCKSIZE_IN_MB = LEAVES * 400 / 1000000`. The benchmark tests up to 33554432 leaves which corresponds to a blocksize of about 13.5GB.

**How to compile?**  

First off, you need to install OpenCL.  
You can skip this step if you are on macOS, as it ships with OpenCL.

Any of these links below should work. It's recommend to install your GPU vendor's library.  
Download links for OpenCL library:

**NVIDIA**: https://developer.nvidia.com/cuda-downloads  
**AMD**: https://github.com/GPUOpen-LibrariesAndSDKs/OCL-SDK/releases  
**Intel**: https://software.intel.com/en-us/articles/opencl-drivers  

On my Windows VM, I found it pretty easy to use the AMD link.  
If you have installed make and cmake, you can just run `cmake . && make` in the root directory of this repository. You'll get a `fastmerkle` file.  
Alternatively, with gcc:  

**macOS**: `gcc main.c -framework OpenCL -o fastmerkle`  
**Linux** and **Windows (MinGW-w64)**: `gcc main.c -lOpenCL -o fastmerkle`

You might have to set include and library paths.
