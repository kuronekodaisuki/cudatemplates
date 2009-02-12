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

#ifndef CUDA_CONVERT_H
#define CUDA_CONVERT_H


#include <cudatemplates/copy.hpp>
#include <cudatemplates/dimension.hpp>
#include <cudatemplates/hostmemory.hpp>


#ifdef __CUDACC__

template <class Type1, class Type2>
__global__ void convert_type_nocheck_kernel(Type1 dst, Type2 src, Cuda::Dimension<1> dummy)
{
  int x = threadIdx.x + blockIdx.x * blockDim.x;
  dst.data[x] = src.data[x];
}

template <class Type1, class Type2>
__global__ void convert_type_check_kernel(Type1 dst, Type2 src, Cuda::Dimension<1> dummy)
{
  int x = threadIdx.x + blockIdx.x * blockDim.x;

  if(x < dst.size[0])
    dst.data[x] = src.data[x];
}

template <class Type1, class Type2>
__global__ void convert_type_nocheck_kernel(Type1 dst, Type2 src, Cuda::Dimension<2> dummy)
{
  int x = threadIdx.x + blockIdx.x * blockDim.x;
  int y = threadIdx.y + blockIdx.y * blockDim.y;
  dst.data[x + y * dst.stride[0]] = src.data[x + y * src.stride[0]];
}

template <class Type1, class Type2>
__global__ void convert_type_check_kernel(Type1 dst, Type2 src, Cuda::Dimension<2> dummy)
{
  int x = threadIdx.x + blockIdx.x * blockDim.x;
  int y = threadIdx.y + blockIdx.y * blockDim.y;

  if((x < dst.size[0]) && (y < dst.size[1]))
    dst.data[x + y * dst.stride[0]] = src.data[x + y * src.stride[0]];
}

template <class Type1, class Type2>
__global__ void convert_type_nocheck_kernel(Type1 dst, Type2 src, Cuda::Dimension<3> dummy)
{
  int x = threadIdx.x + blockIdx.x * blockDim.x;
  int y = threadIdx.y + blockIdx.y * blockDim.y;
  int z = threadIdx.z + blockIdx.z * blockDim.z;
  dst.data[x + y * dst.stride[0] + z * dst.stride[1]] = src.data[x + y * src.stride[0] + z * src.stride[1]];
}

template <class Type1, class Type2>
__global__ void convert_type_check_kernel(Type1 dst, Type2 src, Cuda::Dimension<3> dummy)
{
  int x = threadIdx.x + blockIdx.x * blockDim.x;
  int y = threadIdx.y + blockIdx.y * blockDim.y;
  int z = threadIdx.z + blockIdx.z * blockDim.z;

  if((x < dst.size[0]) && (y < dst.size[1]) && (z < dst.size[2]))
    dst.data[x + y * dst.stride[0] + z * dst.stride[1]] = src.data[x + y * src.stride[0] + z * src.stride[1]];
}

#endif  // __CUDACC__

namespace Cuda {

/**
   Convert data in host memory.
   @param dst destination pointer
   @param src source pointer
*/
template<class Type1, class Type2, unsigned Dim>
void
copy(HostMemory<Type1, Dim> &dst, const HostMemory<Type2, Dim> &src)
{
  CUDA_CHECK_SIZE;
  Cuda::Iterator<Dim> src_end = src.end();

  for(Cuda::Iterator<Dim> i = src.begin(); i != src_end; ++i)
    dst[i] = src[i];
}

#ifdef __CUDACC__

/**
   Convert data in device memory.
   Since this function calls a CUDA kernel, it is only available if the file
   from which this function is called is compiled by nvcc.
   @param dst destination pointer
   @param src source pointer
*/
template<class Type1, class Type2, unsigned Dim>
void
copy(DeviceMemory<Type1, Dim> &dst, const DeviceMemory<Type2, Dim> &src)
{
  CUDA_CHECK_SIZE;
  dim3 gridDim, blockDim;
  bool aligned;
  dst.getExecutionConfiguration(gridDim, blockDim, aligned);
  typename DeviceMemory<Type1, Dim>::KernelData kdst(dst);
  typename DeviceMemory<Type2, Dim>::KernelData ksrc(src);

  if(aligned)
    convert_type_nocheck_kernel<<<gridDim, blockDim>>>(kdst, ksrc, Dimension<Dim>());
  else
    convert_type_check_kernel<<<gridDim, blockDim>>>(kdst, ksrc, Dimension<Dim>());

  CUDA_CHECK(cudaGetLastError());
}

#endif  // __CUDACC__

/**
   Convert data in host memory.
   @param dst generic destination pointer
   @param src generic source pointer
   @param dst_ofs destination offset
   @param src_ofs source offset
   @param size size of region to be converted
*/
template<class Type1, class Type2, unsigned Dim>
void
copy(HostMemory<Type1, Dim> &dst, const HostMemory<Type2, Dim> &src,
     const Size<Dim> &dst_ofs, const Size<Dim> &src_ofs, const Size<Dim> &size)
{
  check_bounds(dst, src, dst_ofs, src_ofs, size);
  Cuda::Iterator<Dim> src_begin(src_ofs, Cuda::Size<Dim>(src_ofs + size));
  Cuda::Iterator<Dim> src_end = src_begin;
  src_end.setEnd();
  Cuda::Iterator<Dim> dst_begin(dst_ofs, Cuda::Size<Dim>(dst_ofs + size));

  for(Cuda::Iterator<Dim> i = src_begin, j = dst_begin; i != src_end; ++i, ++j)
    dst[j] = src[i];
}

}  // namespace Cuda


#endif
