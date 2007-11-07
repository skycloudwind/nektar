///////////////////////////////////////////////////////////////////////////////
//
// File: FullMatrixStoragePolicy.hpp
//
// For more information, please see: http://www.nektar.info
//
// The MIT License
//
// Copyright (c) 2006 Division of Applied Mathematics, Brown University (USA),
// Department of Aeronautics, Imperial College London (UK), and Scientific
// Computing and Imaging Institute, University of Utah (USA).
//
// License for the specific language governing rights and limitations under
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
// Description: Interface classes for matrices
//
///////////////////////////////////////////////////////////////////////////////

#ifndef NEKTAR_LIB_UTILITIES_LINEAR_ALGEBRA_FULL_MATRIX_STORAGE_POLICY_HPP
#define NEKTAR_LIB_UTILITIES_LINEAR_ALGEBRA_FULL_MATRIX_STORAGE_POLICY_HPP

#include <LibUtilities/LinearAlgebra/MatrixStoragePolicy.hpp>
#include <LibUtilities/BasicUtils/SharedArray.hpp>
#include <LibUtilities/LinearAlgebra/Lapack.hpp>
#include "boost/tuple/tuple.hpp"

namespace Nektar
{
    template<typename DataType>
    class MatrixStoragePolicy<DataType, FullMatrixTag>
    {
        public:
            typedef typename boost::call_traits<DataType>::reference GetValueReturnType;
            typedef DefaultPolicySpecificDataHolder PolicySpecificDataHolderType;

            static Array<OneD, DataType> Initialize()
            {
                return Array<OneD, DataType>();
            }
            
            static Array<OneD, DataType> Initialize(unsigned int rows, unsigned int columns, const PolicySpecificDataHolderType&)
            {
                return Array<OneD, DataType>(rows*columns);
            }
            
            static Array<OneD, DataType> Initialize(unsigned int rows, unsigned int columns, 
                                                    typename boost::call_traits<DataType>::const_reference d,
                                                    const PolicySpecificDataHolderType&)
            {
                return Array<OneD, DataType>(rows*columns, d);
            }
            
            static Array<OneD, DataType> Initialize(unsigned int rows, unsigned int columns, 
                                                    const DataType* d,
                                                    const PolicySpecificDataHolderType&)
            {
                return Array<OneD, DataType>(rows*columns, d);
            }
            
            static Array<OneD, DataType> Initialize(unsigned int rows, unsigned int columns, 
                                                    const ConstArray<OneD, DataType>& d,
                                                    const PolicySpecificDataHolderType&)
            {
                ASSERTL0(rows*columns <= d.num_elements(), 
                    std::string("An attempt has been made to create a full matrix of size ") +
                    boost::lexical_cast<std::string>(rows*columns) + 
                    std::string(" but the array being used to populate it only has ") + 
                    boost::lexical_cast<std::string>(d.num_elements()) + 
                    std::string(" elements."));
                Array<OneD, DataType> result;
                CopyArrayN(d, result, rows*columns);
                return result;
            }
            
            static unsigned int CalculateIndex(unsigned int totalRows, unsigned int totalColumns, unsigned int curRow, unsigned int curColumn, const char transpose)
            {
                if( transpose == 'N' )
                {
                    return curColumn*totalRows + curRow;
                }
                else
                {
                    return curRow*totalColumns + curColumn;
                }
            }

            static GetValueReturnType GetValue(unsigned int totalRows, unsigned int totalColumns,
                                               unsigned int curRow, unsigned int curColumn,
                                               Array<OneD, DataType>& data,
                                               const char transpose,
                                               const PolicySpecificDataHolderType&)
            {
                return data[CalculateIndex(totalRows, totalColumns, curRow, curColumn, transpose)];
            }
            
            static typename boost::call_traits<DataType>::const_reference GetValue(unsigned int totalRows, unsigned int totalColumns,
                                                                             unsigned int curRow, unsigned int curColumn,
                                                                             const ConstArray<OneD, DataType>& data,
                                                                             const char transpose,
                                                                             const PolicySpecificDataHolderType&)
            {
                return data[CalculateIndex(totalRows, totalColumns, curRow, curColumn, transpose)];
            }
            
            static void SetValue(unsigned int totalRows, unsigned int totalColumns,
                                 unsigned int curRow, unsigned int curColumn,
                                 Array<OneD, DataType>& data, typename boost::call_traits<DataType>::const_reference d,
                                 const char transpose,
                                 const PolicySpecificDataHolderType&)
            {
                data[CalculateIndex(totalRows, totalColumns, curRow, curColumn, transpose)] = d;
            }
            
            static boost::tuples::tuple<unsigned int, unsigned int> 
            Advance(const unsigned int totalRows, const unsigned int totalColumns,
                    const unsigned int curRow, const unsigned int curColumn,
                    const char transpose,
                    const PolicySpecificDataHolderType&)
            {
                unsigned int nextRow = curRow;
                unsigned int nextColumn = curColumn;

                if( transpose == 'N' )
                {
                    if( nextRow < totalRows )
                    {
                        ++nextRow;
                    }

                    if( nextRow >= totalRows )
                    {
                        nextRow = 0;
                        ++nextColumn;
                    }

                    if( nextColumn >= totalColumns )
                    {
                        nextRow = std::numeric_limits<unsigned int>::max();
                        nextColumn = std::numeric_limits<unsigned int>::max();
                    }
                }
                else
                {
                    if( nextColumn < totalColumns )
                    {
                        ++nextColumn;
                    }

                    if( nextColumn >= totalColumns )
                    {
                        nextColumn = 0;
                        ++nextRow;
                    }

                    if( nextRow >= totalRows )
                    {
                        nextRow = std::numeric_limits<unsigned int>::max();
                        nextColumn = std::numeric_limits<unsigned int>::max();
                    }
                }

                return boost::tuples::tuple<unsigned int, unsigned int>(nextRow, nextColumn);
            }
            
            static void Invert(unsigned int rows, unsigned int columns,
                               Array<OneD, DataType>& data,
                               const char transpose,
                               const PolicySpecificDataHolderType&)
            {
                #ifdef NEKTAR_USING_BLAS
                    ASSERTL0(rows==columns, "Only square matrices can be inverted.");
                    ASSERTL0(transpose=='N', "Only untransposed matrices may be inverted.");

                    int m = rows;
                    int n = columns;
                    int pivotSize = n;
                    int info = 0;
                    Array<OneD, int> ipivot(n);
                    Array<OneD, DataType> work(n);
                    
                    Lapack::Dgetrf(m, n, data.get(), m, ipivot.get(), info);
        
                    if( info < 0 )
                    {
                        std::string message = "ERROR: The " + boost::lexical_cast<std::string>(-info) + "th parameter had an illegal parameter for dgetrf";
                        ASSERTL0(false, message.c_str());
                    }
                    else if( info > 0 )
                    {
                        std::string message = "ERROR: Element u_" + boost::lexical_cast<std::string>(info) +   boost::lexical_cast<std::string>(info) + " is 0 from dgetrf";
                        ASSERTL0(false, message.c_str());
                    }   
                    
                    Lapack::Dgetri(n, data.get(), n, ipivot.get(),
                                   work.get(), n, info);
                    
                    if( info < 0 )
                    {
                        std::string message = "ERROR: The " + boost::lexical_cast<std::string>(-info) + "th parameter had an illegal parameter for dgetri";
                        ASSERTL0(false, message.c_str());
                    }
                    else if( info > 0 )
                    {
                        std::string message = "ERROR: Element u_" + boost::lexical_cast<std::string>(info) +   boost::lexical_cast<std::string>(info) + " is 0 from dgetri";
                        ASSERTL0(false, message.c_str());
                    }   
                    
                #else
                    // error Full matrix inversion not supported without blas.
                    BOOST_STATIC_ASSERT(sizeof(DataType) == 0);
                #endif
            }
            
    };
}

#endif //NEKTAR_LIB_UTILITIES_LINEAR_ALGEBRA_FULL_MATRIX_STORAGE_POLICY_HPP
