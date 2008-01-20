///////////////////////////////////////////////////////////////////////////////
//
// File: TestConstantBinarySpecialization4.cpp
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
// Description: Tests Constant-Binary specializations which require a temporary
// for each side.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef NEKTAR_USE_EXPRESSION_TEMPLATES
#define NEKTAR_USE_EXPRESSION_TEMPLATES
#endif //NEKTAR_USE_EXPRESSION_TEMPLATES

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>
#include <LibUtilities/BasicUtils/OperatorGenerators.hpp>

namespace Nektar
{
//    // a - Expression is Associative.
//        // b - Expression is Commutative.
//        // c - Left Equal Operator is defined.
//        // d - Constant Expression is result type.
//        // e - Binary Expression is result type.
//        // f - In A + (B-C), (A+B) is the result type.
//        // g - In A + (B-C)
//        // Case 4 - Evaluate lhs, evaluate rhs with parent op type.
//        // 1--1-- 000010       
//        template<typename LhsResultType, typename RhsLhsPolicyType, typename RhsRhsPolicyType,
//                 template <typename, typename> class RhsOpType,
//                 template <typename, typename> class OpType,
//                 typename ResultType>
//        struct BinaryExpressionEvaluator<ConstantExpressionPolicy<LhsResultType>,
//                                        BinaryExpressionPolicy<RhsLhsPolicyType, RhsOpType, RhsRhsPolicyType>,
//                                        ResultType, OpType, BinaryNullOp,
//                                        typename boost::enable_if
//                                          boost::mpl::and_
//                                            <
//                                                typename AssociativeTraits<ConstantExpressionPolicy<LhsResultType>, OpType, BinaryExpressionPolicy<RhsLhsPolicyType, RhsOpType, RhsRhsPolicyType> >::IsAssociative,
//                                     <
//                                                     boost::is_same<ResultType, typename ConstantExpressionPolicy<LhsResultType>::ResultType>
//                                            >
//                                         >::type
//                                         >
//        {
//            static void Eval(const Expression<ConstantExpressionPolicy<LhsResultType> >& lhs,
//                             const Expression<BinaryExpressionPolicy<RhsLhsPolicyType, RhsOpType, RhsRhsPolicyType> >& rhs, 
//                             Accumulator<ResultType>& accum)
//            {
//                typedef typename BinaryExpressionPolicy<RhsLhsPolicyType, RhsOpType, RhsRhsPolicyType>::ResultType RhsResultType;
//                
//                lhs.Evaluate(accum);
//                rhs.template Evaluate<OpType>(accum);
//            }
//            
//            #ifdef NEKTAR_UNIT_TESTS
//            static const unsigned int ClassNum = 4;
//            #endif //NEKTAR_UNIT_TESTS
//        };

//        boost::mpl::and_
//        <
//            typename AssociativeTraits<ConstantExpressionPolicy<LhsResultType>, OpType, BinaryExpressionPolicy<RhsLhsPolicyType, RhsOpType, RhsRhsPolicyType> >::IsAssociative,
//            boost::is_same<ResultType, typename ConstantExpressionPolicy<LhsResultType>::ResultType>,
//            typename AssociativeTraits<ConstantExpressionPolicy<LhsResultType>, OpType, BinaryExpressionPolicy<RhsLhsPolicyType, RhsOpType, RhsRhsPolicyType> >::OpEqualsAreDefined
//        >
        // Need R = R + (B+C), where R += B and R += C exist. 
        // R = R - (B+C), just need to add r -= B and r -= c
        
        class B
        {
            public:
                B() : Value(0) {}
                B(int v) : Value(v) {}
                int Value;
        };
        
        class C
        {
            public:
                C() : Value(0) {}
                C(int v) : Value(v) {}
                int Value;
        };
        
        class R
        {
            public:
                R() : Value(0) {}
                R(int v) : Value(v) {}
                int Value;
        };
        
        R NekAdd(const B& lhs, const C& rhs)
        {
            return R(lhs.Value*rhs.Value);
        }
        
        void NekAdd(R& result, const B& lhs, const C& rhs)
        {
            result.Value = lhs.Value+rhs.Value;
        }
        
        R NekAdd(const R& lhs, const B& rhs)
        {
            return R(lhs.Value + rhs.Value);
        }
        
        R NekAdd(const R& lhs, const C& rhs)
        {
            return R(lhs.Value + rhs.Value);
        }
        
        void NekAdd(R& result, const R& lhs, const B& rhs)
        {
            result.Value = lhs.Value + rhs.Value;
        }
        
        void NekAdd(R& result, const R& lhs, const C& rhs)
        {
            result.Value = lhs.Value + rhs.Value;
        }
        
        void NekAddEqual(R& result, const B& rhs)
        {
            result.Value += rhs.Value;
        }
        
        void NekAddEqual(R& result, const C& rhs)
        {
            result.Value += rhs.Value;
        }
            
        GENERATE_ADDITION_OPERATOR(B, 0, C, 0);     
        
        R NekAdd(const R& lhs, const R& rhs)
        {
            return R(lhs.Value + rhs.Value);
        }
        
        void NekAdd(R& result, const R& lhs, const R& rhs)
        {
            result.Value = lhs.Value + rhs.Value;
        }
        
        GENERATE_ADDITION_OPERATOR(R, 0, R,0);
        GENERATE_ADDITION_OPERATOR(R, 0, B, 0);
        GENERATE_ADDITION_OPERATOR(R, 0, C, 0);
        
        R NekSubtract(const R& lhs, const R& rhs)
        {
            return R(lhs.Value - rhs.Value);
        }
        
        void NekSubtract(R& result, const R& lhs, const R& rhs)
        {
            result.Value = lhs.Value - rhs.Value;
        }
        
        GENERATE_SUBTRACTION_OPERATOR(R, 0, R, 0);
        
        
        R NekSubtract(const R& lhs, const B& rhs)
        {
            return R(lhs.Value - rhs.Value);
        }
        
        void NekSubtract(R& result, const R& lhs, const B& rhs)
        {
            result.Value = lhs.Value - rhs.Value;
        }
        
        void NekSubtractEqual(R& result, const B& rhs)
        {
            result.Value -= rhs.Value;
        }
        
        GENERATE_SUBTRACTION_OPERATOR(R, 0, B, 0);
        
        R NekSubtract(const R& lhs, const C& rhs)
        {
            return R(lhs.Value - rhs.Value);
        }
        
        void NekSubtract(R& result, const R& lhs, const C& rhs)
        {
            result.Value = lhs.Value - rhs.Value;
        }
        
        void NekSubtractEqual(R& result, const C& rhs)
        {
            result.Value -= rhs.Value;
        }
        
        GENERATE_SUBTRACTION_OPERATOR(R, 0, C, 0);
        
        BOOST_AUTO_TEST_CASE(TestConstantBinarySpecialization4_Case1_NoOpChange)
        {
            R obj1(10);
            B obj2(2);
            C obj3(8);
            
            typedef BinaryExpressionPolicy<ConstantExpressionPolicy<B>,
                                           AddOp,
                                           ConstantExpressionPolicy<C> > RhsExpressionType;
            typedef ConstantExpressionPolicy<R> LhsExpressionType;
            
            //BOOST_STATIC_ASSERT(( BinaryExpressionEvaluator<LhsExpressionType, RhsExpressionType, R, AddOp, BinaryNullOp>::ClassNum == 5 ));
             
            Expression<BinaryExpressionPolicy<LhsExpressionType, AddOp, RhsExpressionType> > exp =
                obj1 + (obj2+obj3);

            R r;
            Assign(r, exp);
            BOOST_CHECK_EQUAL(r.Value, 10+2+8);
        }

        BOOST_AUTO_TEST_CASE(TestConstantBinarySpecialization4_Case2_OpChange)
        {
            // R = R - (B+C)
            R obj1(10);
            B obj2(2);
            C obj3(8);
            
            typedef BinaryExpressionPolicy<ConstantExpressionPolicy<B>,
                                           AddOp,
                                           ConstantExpressionPolicy<C> > RhsExpressionType;
            typedef ConstantExpressionPolicy<R> LhsExpressionType;
            
            //BOOST_STATIC_ASSERT(( BinaryExpressionEvaluator<LhsExpressionType, RhsExpressionType, R, SubtractOp, BinaryNullOp>::ClassNum == 5 ));
             
            Expression<BinaryExpressionPolicy<LhsExpressionType, SubtractOp, RhsExpressionType> > exp =
                obj1 - (obj2+obj3);

            R r;
            Assign(r, exp);
            BOOST_CHECK_EQUAL(r.Value, 10-2-8);
            
            
        }

}

