///////////////////////////////////////////////////////////////////////////////
//
// File: TestFullMatrixStoragePolicy.cpp
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
// Description: 
//
///////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>

#include <LibUtilities/LinearAlgebra/NekMatrix.hpp>
#include <UnitTests/CountedObject.h>
#include <UnitTests/util.h>
#include <boost/test/auto_unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>

#include <boost/test/auto_unit_test.hpp>

namespace Nektar
{
    namespace FullMatrixStoragePolicyUnitTests
    {
        typedef FullMatrixFuncs Policy;

        BOOST_AUTO_TEST_CASE(TestCalculateIndex)
        {
            UnitTests::RedirectCerrIfNeeded();
            BOOST_CHECK_EQUAL(0, Policy::CalculateIndex(5, 5, 0, 0, 'N'));
            BOOST_CHECK_EQUAL(1, Policy::CalculateIndex(5, 5, 1, 0, 'N'));
            BOOST_CHECK_EQUAL(2, Policy::CalculateIndex(5, 5, 2, 0, 'N'));
            BOOST_CHECK_EQUAL(3, Policy::CalculateIndex(5, 5, 3, 0, 'N'));
            BOOST_CHECK_EQUAL(4, Policy::CalculateIndex(5, 5, 4, 0, 'N'));
            BOOST_CHECK_EQUAL(5, Policy::CalculateIndex(5, 5, 5, 0, 'N'));
            BOOST_CHECK_EQUAL(6, Policy::CalculateIndex(5, 5, 6, 0, 'N'));

            BOOST_CHECK_EQUAL(0, Policy::CalculateIndex(5, 5, 0, 0, 'T'));
            BOOST_CHECK_EQUAL(5, Policy::CalculateIndex(5, 5, 1, 0, 'T'));
            BOOST_CHECK_EQUAL(10, Policy::CalculateIndex(5, 5, 2, 0, 'T'));
            BOOST_CHECK_EQUAL(15, Policy::CalculateIndex(5, 5, 3, 0, 'T'));
            BOOST_CHECK_EQUAL(20, Policy::CalculateIndex(5, 5, 4, 0, 'T'));
            BOOST_CHECK_EQUAL(25, Policy::CalculateIndex(5, 5, 5, 0, 'T'));
            BOOST_CHECK_EQUAL(30, Policy::CalculateIndex(5, 5, 6, 0, 'T'));
        }


                    
        BOOST_AUTO_TEST_CASE(TestAdvance)
        {
            UnitTests::RedirectCerrIfNeeded();
            typedef FullMatrixFuncs Policy;

            {
                unsigned int curRow = 0; 
                unsigned int curColumn = 0;
                boost::tie(curRow, curColumn) = Policy::Advance(4, 3, curRow, curColumn);
                BOOST_CHECK_EQUAL(1, curRow);
                BOOST_CHECK_EQUAL(0, curColumn);

                boost::tie(curRow, curColumn) = Policy::Advance(4, 3, curRow, curColumn);
                BOOST_CHECK_EQUAL(2, curRow);
                BOOST_CHECK_EQUAL(0, curColumn);

                boost::tie(curRow, curColumn) = Policy::Advance(4, 3, curRow, curColumn);
                BOOST_CHECK_EQUAL(3, curRow);
                BOOST_CHECK_EQUAL(0, curColumn);

                boost::tie(curRow, curColumn) = Policy::Advance(4, 3, curRow, curColumn);
                BOOST_CHECK_EQUAL(0, curRow);
                BOOST_CHECK_EQUAL(1, curColumn);

                boost::tie(curRow, curColumn) = Policy::Advance(4, 3, curRow, curColumn);
                BOOST_CHECK_EQUAL(1, curRow);
                BOOST_CHECK_EQUAL(1, curColumn);

                boost::tie(curRow, curColumn) = Policy::Advance(4, 3, curRow, curColumn);
                BOOST_CHECK_EQUAL(2, curRow);
                BOOST_CHECK_EQUAL(1, curColumn);

                boost::tie(curRow, curColumn) = Policy::Advance(4, 3, curRow, curColumn);
                BOOST_CHECK_EQUAL(3, curRow);
                BOOST_CHECK_EQUAL(1, curColumn);

                boost::tie(curRow, curColumn) = Policy::Advance(4, 3, curRow, curColumn);
                BOOST_CHECK_EQUAL(0, curRow);
                BOOST_CHECK_EQUAL(2, curColumn);

                boost::tie(curRow, curColumn) = Policy::Advance(4, 3, curRow, curColumn);
                BOOST_CHECK_EQUAL(1, curRow);
                BOOST_CHECK_EQUAL(2, curColumn);

                boost::tie(curRow, curColumn) = Policy::Advance(4, 3, curRow, curColumn);
                BOOST_CHECK_EQUAL(2, curRow);
                BOOST_CHECK_EQUAL(2, curColumn);

                boost::tie(curRow, curColumn) = Policy::Advance(4, 3, curRow, curColumn);
                BOOST_CHECK_EQUAL(3, curRow);
                BOOST_CHECK_EQUAL(2, curColumn);

                boost::tie(curRow, curColumn) = Policy::Advance(4, 3, curRow, curColumn);
                BOOST_CHECK_EQUAL(std::numeric_limits<unsigned int>::max(), curRow);
                BOOST_CHECK_EQUAL(std::numeric_limits<unsigned int>::max(), curColumn);
            }

            {

                unsigned int curRow = 0; 
                unsigned int curColumn = 0;
                boost::tie(curRow, curColumn) = Policy::Advance(1, 1, curRow, curColumn);
                BOOST_CHECK_EQUAL(std::numeric_limits<unsigned int>::max(), curRow);
                BOOST_CHECK_EQUAL(std::numeric_limits<unsigned int>::max(), curColumn);
            }
        }


    }
}


