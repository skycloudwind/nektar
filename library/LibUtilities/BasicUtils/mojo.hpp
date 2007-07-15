///////////////////////////////////////////////////////////////////////////////
//
// File mojo.hpp
//
// For more information, please see: http://www.nektar.info
//
// The MIT License
//
// Copyright (c) 2006 Scientific Computing and Imaging Institute,
// University of Utah (USA) and Department of Aeronautics, Imperial
// College London (UK).
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
// Description: Move Constructors.  See Generic Programming: Move Constructors
// by Andrei Alexandrescu in the C++ User's Journal.
//
// These classes were originally used with SharedArray.  We had problems with
// some code not compiling because of too many implicit conversion operators,
// so we backed that out.  These objects are currently not used but they are
// being left in because the idiom may prove useful in other contexts.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef NEKTAR_LIB_UTILITIES_BASIC_UTILS_MOJO_HPP
#define NEKTAR_LIB_UTILITIES_BASIC_UTILS_MOJO_HPP

namespace Nektar
{
    template<typename ClassType>
    class Constant
    {
        public:
            explicit Constant(const ClassType& rhs) :
                data(&rhs)
            {
            }
            
            const ClassType& GetValue() const { return *data; }
            
        private:
            const ClassType* data;
        
    };
    
    template<typename ClassType>
    class Temporary : private Constant<ClassType>
    {
        public:
            explicit Temporary(ClassType& rhs) :
                Constant<ClassType>(rhs)
            {
            }
            
            ClassType& GetValue()
            {
                return const_cast<ClassType&>(Constant<ClassType>::GetValue());
            }
    };
    
    template<typename ClassType>
    class MojoEnabled
    {
        public:
            operator Temporary<ClassType>()
            {
                return Temporary<ClassType>(static_cast<ClassType&>(*this));
            }
            
            operator Constant<ClassType>() const
            {
                return Constant<ClassType>(static_cast<const ClassType&>(*this));
            }
            
        protected:
            MojoEnabled() {}
            ~MojoEnabled() {}
    };
    
}

#endif //NEKTAR_LIB_UTILITIES_BASIC_UTILS_MOJO_HPP


