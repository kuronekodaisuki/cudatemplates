/*
  NOTE: THIS FILE HAS BEEN CREATED AUTOMATICALLY,
  ANY CHANGES WILL BE OVERWRITTEN WITHOUT NOTICE!
*/

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

#ifndef CUDA_GILREFERENCE3D_H
#define CUDA_GILREFERENCE3D_H


#include <cudatemplates/gilreference.hpp>


namespace Cuda {

/**
   GilReference template specialized for 3 dimension(s).
 */
template <class Type>
class GilReference3D: public GilReference<Type, 3>
{
public:
  inline GilReference3D()
  {
  }

  inline GilReference3D(const Size<3> &_size, Type *_buffer):
    Layout<Type, 3>(_size),
    Pointer<Type, 3>(_size),
    GilReference<Type, 3>(_size, _buffer)
  {
  }

  inline GilReference3D(const Layout<Type, 3> &layout, Type *_buffer):
    Layout<Type, 3>(layout),
    Pointer<Type, 3>(layout),
    GilReference<Type, 3>(layout, _buffer)
  {
  }

  inline GilReference3D(size_t size0, size_t size1, size_t size2, Type *_buffer):
    Layout<Type, 3>(Size<3>(size0, size1, size2)),
    Pointer<Type, 3>(Size<3>(size0, size1, size2)),
    GilReference<Type, 3>(Size<3>(size0, size1, size2), _buffer)
  {
  }

  inline void alloc()
  {
    GilReference<Type, 3>::alloc();
  }

  inline void alloc(size_t size0, size_t size1, size_t size2)
  {
    Storage<Type, 3>::alloc(Size<3>(size0, size1, size2));
  }
};

}  // namespace Cuda


#endif