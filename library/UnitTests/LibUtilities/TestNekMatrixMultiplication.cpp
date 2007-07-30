///////////////////////////////////////////////////////////////////////////////
//
// File: TestNekMatrixMultiplication.h
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
// Description: Tests NekMatrix functionality.
//
///////////////////////////////////////////////////////////////////////////////

#include <UnitTests/testNekMatrixOperations.h>
#include <LibUtilities/LinearAlgebra/NekMatrix.hpp>
#include <LibUtilities/BasicUtils/BoostUtil.hpp>
#include <boost/test/auto_unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/progress.hpp>
#include <iostream>
#include <boost/bind.hpp>
#include <functional>

namespace Nektar
{
    // Note - All tests should excercise both the blas and normal code.
    // The easiest way to do this is to perform one test with integers and 
    // one with doubles.
    namespace MatrixMultiplicationTests
    {
        void TestStandardFullTimesStandardFull()
        {
            {
                 unsigned int buf1[] = {1, 2, 3,
                                        4, 5, 6,
                                        7, 8, 9};
                 unsigned int buf2[] = { 10, 11, 12, 14,
                                         15, 16, 17, 18,
                                         19, 20, 21, 22 };
 
                                        
                 NekMatrix<unsigned int> lhs(3, 3, buf1);
                 NekMatrix<unsigned int> rhs(3, 4, buf2);
                 NekMatrix<unsigned int> result = lhs*rhs;
 
                 BOOST_CHECK_EQUAL(3, result.GetRows());
                 BOOST_CHECK_EQUAL(4, result.GetColumns());
 
                 BOOST_CHECK(result(0,0) == 97);
                 BOOST_CHECK(result(0,1) == 103);
                 BOOST_CHECK(result(0,2) == 109);
                 BOOST_CHECK(result(0,3) == 116);
 
                 BOOST_CHECK(result(1,0) == 229);
                 BOOST_CHECK(result(1,1) == 244);
                 BOOST_CHECK(result(1,2) == 259);
                 BOOST_CHECK(result(1,3) == 278);
 
                 BOOST_CHECK(result(2,0) == 361);
                 BOOST_CHECK(result(2,1) == 385);
                 BOOST_CHECK(result(2,2) == 409);
                 BOOST_CHECK(result(2,3) == 440);
             }

             {
                 double buf1[] = {1, 2, 3,
                                 4, 5, 6,
                                 7, 8, 9};
                 double buf2[] = { 10, 11, 12,
                     15, 16, 17,
                     19, 20, 21 };
 
                                  
                 NekMatrix<double> lhs(3, 3, buf1);
                 NekMatrix<double> rhs(3, 3, buf2);
 
                 NekMatrix<double> result = lhs*rhs;
 
                 BOOST_CHECK(result.GetRows() == 3);
                 BOOST_CHECK(result.GetColumns() == 3);
 
                 double epsilon = 1e-12;
                 BOOST_CHECK_CLOSE(result(0,0), 97.0, epsilon);
                 BOOST_CHECK_CLOSE(result(0,1), 103.0, epsilon);
                 BOOST_CHECK_CLOSE(result(0,2), 109.0, epsilon);
 
                 BOOST_CHECK_CLOSE(result(1,0), 229.0, epsilon);
                 BOOST_CHECK_CLOSE(result(1,1), 244.0, epsilon);
                 BOOST_CHECK_CLOSE(result(1,2), 259.0, epsilon);
 
                 BOOST_CHECK_CLOSE(result(2,0), 361.0, epsilon);
                 BOOST_CHECK_CLOSE(result(2,1), 385.0, epsilon);
                 BOOST_CHECK_CLOSE(result(2,2), 409.0, epsilon);
             }
        }

        void TestStandardFullTimesVector()
        {
            {
                 unsigned int buf1[] = {1, 2, 3,
                                        4, 5, 6,
                                        7, 8, 9};
                 unsigned int buf2[] = { 10, 11, 12 };
                                        
                 NekMatrix<unsigned int> lhs(3, 3, buf1);
                 NekVector<unsigned int> rhs(3, buf2);
                 NekVector<unsigned int> result = lhs*rhs;
 
                 BOOST_CHECK_EQUAL(3, result.GetRows());
 
                 BOOST_CHECK_EQUAL(68, result[0]);
                 BOOST_CHECK_EQUAL(167, result[1]);
                 BOOST_CHECK_EQUAL(266, result[2]);
             }

             {
                 double buf1[] = {1, 2, 3,
                                 4, 5, 6,
                                 7, 8, 9};
                 double buf2[] = { 10, 11, 12};
 
                                  
                 NekMatrix<double> lhs(3, 3, buf1);
                 NekVector<double> rhs(3, buf2); 
                 NekVector<double> result = lhs*rhs;
 
                 BOOST_CHECK(result.GetRows() == 3);

 
                 double epsilon = 1e-12;
                 BOOST_CHECK_CLOSE(result[0], 68.0, epsilon);
                 BOOST_CHECK_CLOSE(result[1], 167.0, epsilon);
                 BOOST_CHECK_CLOSE(result[2], 266.0, epsilon);
             }

             {
                 double buf1[] = {1, 2, 3,
                                 4, 5, 6,
                                 7, 8, 9,
                                 10, 11, 12};
                 double buf2[] = { 10, 11, 12};
 
                                  
                 NekMatrix<double> lhs(4, 3, buf1);
                 NekVector<double> rhs(3, buf2); 
                 NekVector<double> result = lhs*rhs;
 
                 BOOST_CHECK(result.GetRows() == 4);

 
                 double epsilon = 1e-12;
                 BOOST_CHECK_CLOSE(result[0], 68.0, epsilon);
                 BOOST_CHECK_CLOSE(result[1], 167.0, epsilon);
                 BOOST_CHECK_CLOSE(result[2], 266.0, epsilon);
                 BOOST_CHECK_CLOSE(result[3], 365.0, epsilon);
             }
        }

        void TestScaledFullTimesScaledFull()
        {
            {
                unsigned int buf1[] = {1, 2, 3,
                                    4, 5, 6,
                                    7, 8, 9};
                unsigned int buf2[] = { 10, 11, 12, 14,
                                     15, 16, 17, 18,
                                     19, 20, 21, 22 };

                boost::shared_ptr<NekMatrix<unsigned int> > lhsInnerMatrix(
                    new NekMatrix<unsigned int>(3, 3, buf1));
                boost::shared_ptr<NekMatrix<unsigned int> > rhsInnerMatrix(
                    new NekMatrix<unsigned int>(3, 4, buf2) );
                NekMatrix<NekMatrix<unsigned int>, FullMatrixTag, ScaledMatrixTag> lhs(2, lhsInnerMatrix);
                NekMatrix<NekMatrix<unsigned int>, FullMatrixTag, ScaledMatrixTag> rhs(3, rhsInnerMatrix);
                NekMatrix<unsigned int> result = lhs*rhs;

                BOOST_CHECK_EQUAL(3, result.GetRows());
                BOOST_CHECK_EQUAL(4, result.GetColumns());

                BOOST_CHECK(result(0,0) == 582);
                BOOST_CHECK(result(0,1) == 618);
                BOOST_CHECK(result(0,2) == 654);
                BOOST_CHECK(result(0,3) == 696);

                BOOST_CHECK(result(1,0) == 1374);
                BOOST_CHECK(result(1,1) == 1464);
                BOOST_CHECK(result(1,2) == 1554);
                BOOST_CHECK(result(1,3) == 1668);

                BOOST_CHECK(result(2,0) == 2166);
                BOOST_CHECK(result(2,1) == 2310);
                BOOST_CHECK(result(2,2) == 2454);
                BOOST_CHECK(result(2,3) == 2640);
            }

            {
                double buf1[] = {1, 2, 3,
                                 4, 5, 6,
                                 7, 8, 9};
                double buf2[] = { 10, 11, 12,
                                  15, 16, 17,
                                  19, 20, 21 };

                boost::shared_ptr<NekMatrix<double> > lhsInnerMatrix(                              
                    new NekMatrix<double>(3, 3, buf1));
                boost::shared_ptr<NekMatrix<double> > rhsInnerMatrix(                              
                    new NekMatrix<double>(3, 3, buf2));

                NekMatrix<NekMatrix<double>, FullMatrixTag, ScaledMatrixTag> lhs(2.0, lhsInnerMatrix);
                NekMatrix<NekMatrix<double>, FullMatrixTag, ScaledMatrixTag> rhs(3.0, rhsInnerMatrix);
                
                NekMatrix<double> result = lhs*rhs;

                BOOST_CHECK(result.GetRows() == 3);
                BOOST_CHECK(result.GetColumns() == 3);

                double epsilon = 1e-12;
                BOOST_CHECK_CLOSE(result(0,0), 582.0, epsilon);
                BOOST_CHECK_CLOSE(result(0,1), 618.0, epsilon);
                BOOST_CHECK_CLOSE(result(0,2), 654.0, epsilon);

                BOOST_CHECK_CLOSE(result(1,0), 1374.0, epsilon);
                BOOST_CHECK_CLOSE(result(1,1), 1464.0, epsilon);
                BOOST_CHECK_CLOSE(result(1,2), 1554.0, epsilon);

                BOOST_CHECK_CLOSE(result(2,0), 2166.0, epsilon);
                BOOST_CHECK_CLOSE(result(2,1), 2310.0, epsilon);
                BOOST_CHECK_CLOSE(result(2,2), 2454.0, epsilon);
            }
        }

        void TestScaledFullTimesVector()
        {
            {
                 unsigned int buf1[] = {1, 2, 3,
                                        4, 5, 6,
                                        7, 8, 9};
                 unsigned int buf2[] = { 10, 11, 12 };
                           
                 boost::shared_ptr<NekMatrix<unsigned int> > innerMatrix(
                     new NekMatrix<unsigned int>(3, 3, buf1));
                 NekMatrix<NekMatrix<unsigned int>, FullMatrixTag, ScaledMatrixTag>
                     lhs(2, innerMatrix);
                 NekVector<unsigned int> rhs(3, buf2);
                 NekVector<unsigned int> result = lhs*rhs;
 
                 BOOST_CHECK_EQUAL(3, result.GetRows());
 
                 BOOST_CHECK_EQUAL(136, result[0]);
                 BOOST_CHECK_EQUAL(334, result[1]);
                 BOOST_CHECK_EQUAL(532, result[2]);
             }

             {
                 double buf1[] = {1, 2, 3,
                                 4, 5, 6,
                                 7, 8, 9};
                 double buf2[] = { 10, 11, 12};
 
                                  
                 boost::shared_ptr<NekMatrix<double> > innerMatrix(
                     new NekMatrix<double>(3, 3, buf1));
                 NekMatrix<NekMatrix<double>, FullMatrixTag, ScaledMatrixTag> lhs(2.0, innerMatrix);
                 NekVector<double> rhs(3, buf2); 
                 NekVector<double> result = lhs*rhs;
 
                 BOOST_CHECK(result.GetRows() == 3);

                 double epsilon = 1e-12;
                 BOOST_CHECK_CLOSE(result[0], 136.0, epsilon);
                 BOOST_CHECK_CLOSE(result[1], 334.0, epsilon);
                 BOOST_CHECK_CLOSE(result[2], 532.0, epsilon);
             }

             {
                 double buf1[] = {1, 2, 3,
                                 4, 5, 6,
                                 7, 8, 9,
                                 10, 11, 12};
                 double buf2[] = { 10, 11, 12};
 
                 boost::shared_ptr<NekMatrix<double> > innerMatrix(
                     new NekMatrix<double>(4, 3, buf1));
                 NekMatrix<NekMatrix<double>, FullMatrixTag, ScaledMatrixTag> lhs(3.0, innerMatrix);
                 NekVector<double> rhs(3, buf2); 
                 NekVector<double> result = lhs*rhs;
 
                 BOOST_CHECK(result.GetRows() == 4);

                 double epsilon = 1e-12;
                 BOOST_CHECK_CLOSE(result[0], 204.0, epsilon);
                 BOOST_CHECK_CLOSE(result[1], 501.0, epsilon);
                 BOOST_CHECK_CLOSE(result[2], 798.0, epsilon);
                 BOOST_CHECK_CLOSE(result[3], 1095.0, epsilon);
             }
        }
    }
}



