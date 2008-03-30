///////////////////////////////////////////////////////////////////////////////
//
// File: TestNekMemoryManager.cpp
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
// Description: Tests the boost utility functions.
//
///////////////////////////////////////////////////////////////////////////////


#define NEKTAR_MAX_MEMORY_MANAGER_CONSTRUCTOR_ARGS 4

#include <UnitTests/Memory/TestNekMemoryManager.h>
#include <UnitTests/CountedObject.h>

#include <LibUtilities/Memory/NekMemoryManager.hpp>

namespace Nektar
{
    namespace MemManagerUnitTests
    {        
        BOOST_AUTO_TEST_CASE(testParameterizedConstructors)
        {
            CountedObject<int>::ClearCounters();

            CountedObject<int>* ob1 = MemoryManager<CountedObject<int> >::Allocate();
            BOOST_CHECK_EQUAL(CountedObject<int>::numberDefaultConstructed, 1);
            BOOST_CHECK_EQUAL(CountedObject<int>::numberOf1ParameterConstructions, 0);
            BOOST_CHECK_EQUAL(CountedObject<int>::numberOf2ParameterConstructions, 0);
            BOOST_CHECK_EQUAL(CountedObject<int>::numberOf3ParameterConstructions, 0);

            unsigned int one = 1;
            CountedObject<int>* ob2 = MemoryManager<CountedObject<int> >::Allocate(one);
            BOOST_CHECK_EQUAL(CountedObject<int>::numberDefaultConstructed, 1);
            BOOST_CHECK_EQUAL(CountedObject<int>::numberOf1ParameterConstructions, 1);
            BOOST_CHECK_EQUAL(CountedObject<int>::numberOf2ParameterConstructions, 0);
            BOOST_CHECK_EQUAL(CountedObject<int>::numberOf3ParameterConstructions, 0);

            unsigned int two = 2;
            CountedObject<int>* ob3 = MemoryManager<CountedObject<int> >::Allocate(one, two);
            BOOST_CHECK_EQUAL(CountedObject<int>::numberDefaultConstructed, 1);
            BOOST_CHECK_EQUAL(CountedObject<int>::numberOf1ParameterConstructions, 1);
            BOOST_CHECK_EQUAL(CountedObject<int>::numberOf2ParameterConstructions, 1);
            BOOST_CHECK_EQUAL(CountedObject<int>::numberOf3ParameterConstructions, 0);

            unsigned int three = 3;
            CountedObject<int>* ob4 = MemoryManager<CountedObject<int> >::Allocate(one, two, three);
            BOOST_CHECK_EQUAL(CountedObject<int>::numberDefaultConstructed, 1);
            BOOST_CHECK_EQUAL(CountedObject<int>::numberOf1ParameterConstructions, 1);
            BOOST_CHECK_EQUAL(CountedObject<int>::numberOf2ParameterConstructions, 1);
            BOOST_CHECK_EQUAL(CountedObject<int>::numberOf3ParameterConstructions, 1);
            BOOST_CHECK_EQUAL(ob4->value, 6);

            MemoryManager<CountedObject<int> >::Deallocate(ob1);
            MemoryManager<CountedObject<int> >::Deallocate(ob2);
            MemoryManager<CountedObject<int> >::Deallocate(ob3);
            MemoryManager<CountedObject<int> >::Deallocate(ob4);

            BOOST_CHECK(ob1 == NULL);
            BOOST_CHECK(ob2 == NULL);
            BOOST_CHECK(ob3 == NULL);
            BOOST_CHECK(ob4 == NULL);

            BOOST_CHECK_EQUAL(CountedObject<int>::numberDefaultConstructed, 1);
            BOOST_CHECK_EQUAL(CountedObject<int>::numberOf1ParameterConstructions, 1);
            BOOST_CHECK_EQUAL(CountedObject<int>::numberOf2ParameterConstructions, 1);
            BOOST_CHECK_EQUAL(CountedObject<int>::numberOf3ParameterConstructions, 1);
            BOOST_CHECK_EQUAL(CountedObject<int>::numberDestroyed, 4);
        }

        BOOST_AUTO_TEST_CASE(testSmartPointerAllocation)
        {
            CountedObject<int>::ClearCounters();

            {
                boost::shared_ptr<CountedObject<int> > ob1 = MemoryManager<CountedObject<int> >::AllocateSharedPtr();
                BOOST_CHECK_EQUAL(CountedObject<int>::numberDefaultConstructed, 1);
                BOOST_CHECK_EQUAL(CountedObject<int>::numberOf1ParameterConstructions, 0);
                BOOST_CHECK_EQUAL(CountedObject<int>::numberOf2ParameterConstructions, 0);
                BOOST_CHECK_EQUAL(CountedObject<int>::numberOf3ParameterConstructions, 0);

                int one = 1;
                boost::shared_ptr<CountedObject<int> > ob2 = MemoryManager<CountedObject<int> >::AllocateSharedPtr(one);
                BOOST_CHECK_EQUAL(CountedObject<int>::numberDefaultConstructed, 1);
                BOOST_CHECK_EQUAL(CountedObject<int>::numberOf1ParameterConstructions, 1);
                BOOST_CHECK_EQUAL(CountedObject<int>::numberOf2ParameterConstructions, 0);
                BOOST_CHECK_EQUAL(CountedObject<int>::numberOf3ParameterConstructions, 0);

                int two = 2;
                boost::shared_ptr<CountedObject<int> > ob3 = MemoryManager<CountedObject<int> >::AllocateSharedPtr(one, two);
                BOOST_CHECK_EQUAL(CountedObject<int>::numberDefaultConstructed, 1);
                BOOST_CHECK_EQUAL(CountedObject<int>::numberOf1ParameterConstructions, 1);
                BOOST_CHECK_EQUAL(CountedObject<int>::numberOf2ParameterConstructions, 1);
                BOOST_CHECK_EQUAL(CountedObject<int>::numberOf3ParameterConstructions, 0);

                int three = 3;
                boost::shared_ptr<CountedObject<int> > ob4 = MemoryManager<CountedObject<int> >::AllocateSharedPtr(one, two, three);
                BOOST_CHECK_EQUAL(CountedObject<int>::numberDefaultConstructed, 1);
                BOOST_CHECK_EQUAL(CountedObject<int>::numberOf1ParameterConstructions, 1);
                BOOST_CHECK_EQUAL(CountedObject<int>::numberOf2ParameterConstructions, 1);
                BOOST_CHECK_EQUAL(CountedObject<int>::numberOf3ParameterConstructions, 1);
                BOOST_CHECK_EQUAL(ob4->value, 6);
                BOOST_CHECK_EQUAL(CountedObject<int>::numberDestroyed, 0);
            }
            
            BOOST_CHECK_EQUAL(CountedObject<int>::numberDestroyed, 4);
            BOOST_CHECK_EQUAL(CountedObject<int>::numberDefaultConstructed, 1);
            BOOST_CHECK_EQUAL(CountedObject<int>::numberOf1ParameterConstructions, 1);
            BOOST_CHECK_EQUAL(CountedObject<int>::numberOf2ParameterConstructions, 1);
            BOOST_CHECK_EQUAL(CountedObject<int>::numberOf3ParameterConstructions, 1);
            BOOST_CHECK_EQUAL(CountedObject<int>::numberDestroyed, 4);
        }

    }
}
