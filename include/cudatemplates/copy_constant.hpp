/*
  Cuda Templates.

  Copyright (C) 2008 Institute for Computer Graphics and Vision,
                     Graz University of Technology

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CUDA_COPY_CONSTANT_H
#define CUDA_COPY_CONSTANT_H

#include <cudatemplates/devicememory.hpp>
#include <cudatemplates/dimension.hpp>
#include <cudatemplates/hostmemory.hpp>


template <class Type1, class Type2>
__global__ void copy_constant_nocheck_kernel(Type1 dst, Type2 val, Cuda::Dimension<1> dummy)
{
  int x = threadIdx.x + blockIdx.x * blockDim.x;
  dst.data[x] = val;
}

template <class Type1, class Type2>
__global__ void copy_constant_check_kernel(Type1 dst, Type2 val, CUDA_KERNEL_SIZE(1) rmin, CUDA_KERNEL_SIZE(1) rmax)
{
  int x = threadIdx.x + blockIdx.x * blockDim.x;

  if((x >= rmin[0]) && (x < rmax[0]))
    dst.data[x] = val;
}

template <class Type1, class Type2>
__global__ void copy_constant_nocheck_kernel(Type1 dst, Type2 val, Cuda::Dimension<2> dummy)
{
  int x = threadIdx.x + blockIdx.x * blockDim.x;
  int y = threadIdx.y + blockIdx.y * blockDim.y;
  dst.data[x + y * dst.stride[0]] = val;
}

template <class Type1, class Type2>
__global__ void copy_constant_check_kernel(Type1 dst, Type2 val, CUDA_KERNEL_SIZE(2) rmin, CUDA_KERNEL_SIZE(2) rmax)
{
  int x = threadIdx.x + blockIdx.x * blockDim.x;
  int y = threadIdx.y + blockIdx.y * blockDim.y;

  if((x >= rmin[0]) && (x < rmax[0]) && (y >= rmin[1]) && (y < rmax[1]))
    dst.data[x + y * dst.stride[0]] = val;
}

template <class Type1, class Type2>
__global__ void copy_constant_nocheck_kernel(Type1 dst, Type2 val, Cuda::Dimension<3> dummy)
{
  int x = threadIdx.x + blockIdx.x * blockDim.x;
  int y = threadIdx.y + blockIdx.y * blockDim.y;
  int z = threadIdx.z + blockIdx.z * blockDim.z;
  dst.data[x + y * dst.stride[0] + z * dst.stride[1]] = val;
}

template <class Type1, class Type2>
__global__ void copy_constant_check_kernel(Type1 dst, Type2 val, CUDA_KERNEL_SIZE(3) rmin, CUDA_KERNEL_SIZE(3) rmax)
{
  int x = threadIdx.x + blockIdx.x * blockDim.x;
  int y = threadIdx.y + blockIdx.y * blockDim.y;
  int z = threadIdx.z + blockIdx.z * blockDim.z;

  if((x >= rmin[0]) && (x < rmax[0]) && (y >= rmin[1]) && (y < rmax[1]) && (z >= rmin[2]) && (z < rmax[2]))
    dst.data[x + y * dst.stride[0] + z * dst.stride[1]] = val;
}

template <class Type1, class Type2>
__global__ void copy_constant_check_kernel_3D( Type1 dst, Type2 val, int width, int height, int depth,
                                               int pitch, int pitchPlane, size_t offset_z)
{
  int x = threadIdx.x + blockIdx.x * blockDim.x;
  int y = threadIdx.y + blockIdx.y * blockDim.y;
  int z = threadIdx.z + offset_z;

  if((x < width) && (y < height) && (z < depth))
    dst[x + y*pitch + z*pitchPlane] = val;
}

namespace Cuda {

static inline int div_up(int a, int b) { return (a + b - 1) / b; }

/**
   Dummy class for kernel instantiation.
   nvcc gets confused if it doesn't see at least one instatiation of template
   kernels. The constructor of this dummy class provides one.
*/
struct DummyInstantiateCopyConstantKernels
{
  DummyInstantiateCopyConstantKernels()
  {
    dim3 gridDim(1, 1, 1), blockDim(1, 1, 1);
    Size<1> r1;
    int val = 0;
    typename DeviceMemory<int, 1>::KernelData kdst;
    copy_constant_nocheck_kernel<<<gridDim, blockDim>>>(kdst, val, Dimension<1>());
    copy_constant_check_kernel<<<gridDim, blockDim>>>(kdst, val, r1, r1);
  }
};

/**
   Copy constant value to region in device memory.
   Since this function calls a CUDA kernel, it is only available if the file
   from which this function is called is compiled by nvcc.
   @param dst destination pointer (device memory)
   @param val value to copy
   @param dst_ofs destination offset
   @param size size of region
*/
template<class Type, unsigned Dim>
void
copy(DeviceMemory<Type, Dim> &dst, Type val,
     const Size<Dim> &dst_ofs, const Size<Dim> &size)
{
  if (Dim<3)
  {
    dst.checkBounds(dst_ofs, size);
    dim3 gridDim, blockDim;
    bool aligned;
    size_t dofs;
    Size<Dim> rmin, rmax;
    dst.getExecutionConfiguration(gridDim, blockDim, aligned, dofs, rmin, rmax, dst_ofs, size);
    typename DeviceMemory<Type, Dim>::KernelData kdst(dst);
    kdst.data += dofs;

    if(aligned)
      copy_constant_nocheck_kernel<<<gridDim, blockDim>>>(kdst, val, Dimension<Dim>());
    else
      copy_constant_check_kernel<<<gridDim, blockDim>>>(kdst, val, rmin, rmax);
  }
  else if (Dim == 3)
  {
    dst.checkBounds(dst_ofs, size);
    dim3 gridDim_dummy, blockDim_dummy;
    bool aligned;
    size_t dofs;
    Size<Dim> rmin, rmax;
    dst.getExecutionConfiguration( gridDim_dummy, blockDim_dummy, aligned,
                                   dofs, rmin, rmax, dst_ofs, size);

    // Compute optimal blocksize
    unsigned int block_size = 16;
    unsigned int block_size_z = 2;

    // prepare fragmentation for processing
    dim3 dimBlock(block_size, block_size, block_size_z);
    dim3 dimGrid(div_up(rmax[0], block_size), div_up(rmax[1], block_size), 1);

    for (unsigned int offset_z=0; offset_z<rmax[1]; offset_z+=block_size_z)
    {
      copy_constant_check_kernel_3D<<<dimGrid, dimBlock>>>( &dst.getBuffer()[dofs], val,
                                                            rmax[0], rmax[1], rmax[2],
                                                            dst.stride[0], dst.stride[1],
                                                            offset_z);
    }
  }
  CUDA_CHECK(cudaGetLastError());
}

/**
   Copy constant value to device memory.
   Since this function calls a CUDA kernel, it is only available if the file
   from which this function is called is compiled by nvcc.
   @param dst destination pointer (device memory)
   @param val value to copy
*/
template<class Type, unsigned Dim>
void
copy(DeviceMemory<Type, Dim> &dst, Type val)
{
  copy(dst, val, Size<Dim>(), dst.size);
}

}  // namespace Cuda


#endif
