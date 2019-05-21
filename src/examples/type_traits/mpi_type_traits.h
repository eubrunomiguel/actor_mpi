/******************************************************************************
 *
 *                          MPP: An MPI CPP Interface
 *
 *                  Copyright (C) 2011-2012  Simone Pellegrini
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 ******************************************************************************/

#pragma once

#include <algorithm>
#include <complex>
#include <mpi.h>
#include <vector>

namespace mpi {

//*****************************************************************************
// 									MPI Type
// Traits
//*****************************************************************************
template <class T> struct mpi_type_traits {

  typedef T element_type;
  typedef T *element_addr_type;

  static inline MPI_Datatype get_type(T &&raw);

  static inline size_t get_size(T &raw) { return 1; }

  static inline element_addr_type get_addr(T &raw) { return &raw; }
};

/**
 * Specialization of the mpi_type_traits for primitive types
 */
#define PRIMITIVE(Type, MpiType)                                               \
  template <> inline MPI_Datatype mpi_type_traits<Type>::get_type(Type &&) {   \
    return MpiType;                                                            \
  }

PRIMITIVE(char, MPI_CHAR);
PRIMITIVE(wchar_t, MPI_WCHAR);
PRIMITIVE(short, MPI_SHORT);
PRIMITIVE(int, MPI_INT);
PRIMITIVE(long, MPI_LONG);
PRIMITIVE(signed char, MPI_SIGNED_CHAR);
PRIMITIVE(unsigned char, MPI_UNSIGNED_CHAR);
PRIMITIVE(unsigned short, MPI_UNSIGNED_SHORT);
PRIMITIVE(unsigned int, MPI_UNSIGNED);
PRIMITIVE(unsigned long, MPI_UNSIGNED_LONG);
PRIMITIVE(unsigned long long, MPI_UNSIGNED_LONG_LONG);

PRIMITIVE(float, MPI_FLOAT);
PRIMITIVE(double, MPI_DOUBLE);
PRIMITIVE(long double, MPI_LONG_DOUBLE);

PRIMITIVE(bool, MPI_CXX_BOOL);
PRIMITIVE(std::complex<float>, MPI_COMPLEX);
PRIMITIVE(std::complex<double>, MPI_DOUBLE_COMPLEX);

#undef PRIMITIVE

template <class T> struct mpi_type_traits<const T> {

  typedef const typename mpi_type_traits<T>::element_type element_type;
  typedef const typename mpi_type_traits<T>::element_addr_type
      element_addr_type;

  static inline size_t get_size(const T &elem) {
    return mpi_type_traits<T>::get_size(const_cast<T &>(elem));
  }

  static inline MPI_Datatype get_type(const T &elem) {
    return mpi_type_traits<T>::get_type(T());
  }

  static inline element_addr_type get_addr(const T &elem) {
    return mpi_type_traits<T>::get_addr(const_cast<T &>(elem));
  }
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	std::vector<T> traits
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <class T> struct mpi_type_traits<std::vector<T>> {

  typedef T element_type;
  typedef T *element_addr_type;

  static inline size_t get_size(std::vector<T> &vec) { return vec.size(); }

  static inline MPI_Datatype get_type(std::vector<T> &&vec) {
    return mpi_type_traits<T>::get_type(T());
  }

  static inline element_addr_type get_addr(std::vector<T> &vec) {
    return mpi_type_traits<T>::get_addr(vec.front());
  }
};

} // namespace mpi
