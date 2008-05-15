///////////////////////////////////////////////////////////////////////////////
//
// File StdHexExp.h
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
// Description: Hex routines
//
///////////////////////////////////////////////////////////////////////////////

#ifndef NEKTAR_LIBS_STDREGSION_STDHEXEXP_H
#define NEKTAR_LIBS_STDREGSION_STDHEXEXP_H

#include <StdRegions/StdRegions.hpp>
#include <StdRegions/StdExpansion3D.h>


namespace Nektar
{
    namespace StdRegions
    {

        class StdHexExp: public StdExpansion3D
        {

        public:
        
            StdHexExp();

            /** \brief Constructor using BasisKey class for quadrature
            *  points and order definition 
            */
            StdHexExp(const  LibUtilities::BasisKey &Ba, const  LibUtilities::BasisKey &Bb, const  LibUtilities::BasisKey &Bc);

            /** \brief Constructor using BasisKey class for quadrature
            *  points and order definition where m_coeffs and m_phys 
            *  are all set. 
            */
            StdHexExp(const  LibUtilities::BasisKey &Ba, const  LibUtilities::BasisKey &Bb, const  LibUtilities::BasisKey &Bc,
                double *coeffs, double *phys);

            /** \brief Copy Constructor */
            StdHexExp(const StdHexExp &T);

            /** \brief Destructor */
            ~StdHexExp();
            

            /** \brief Return Shape of region, using  ShapeType enum list. 
            *  i.e. Hexahedron
            */
      
            ShapeType DetShapeType() const
            {
                return eHexahedron;
            }

            const int GetEdgeNcoeffs(const int i) const
            {
                ASSERTL2((i >= 0)&&(i <= 11),"edge id is out of range");

                if((i == 0)||(i == 2)||(i == 8)||(i == 10))
                {
                    return  GetBasisNumModes(0);
                }
                else if((i == 1)||(i == 3)||(i == 9)||(i == 11))
                {
                    return  GetBasisNumModes(1); 
                }
                else
                {
                    return GetBasisNumModes(2);
                }

            }

            const LibUtilities::BasisType GetEdgeBasisType(const int i) const
            {
                ASSERTL2((i >= 0)&&(i <= 11),"edge id is out of range");

                if((i == 0)||(i == 2)||(i==8)||(i==10))
                {
                    return  GetBasisType(0);
                }
                else if((i == 1)||(i == 3)||(i == 9)||(i == 11))
                {
                    return  GetBasisType(1);
                }
                else
                {
                    return GetBasisType(2);
                }

            }

            /** \brief Fill outarray with mode \a mode of expansion
            *
            *    Note for hexahedral expansions _base[0] (i.e. p)  modes run 
            *  fastest
            */

            void FillMode(const int mode, Array<OneD, NekDouble> &outarray);

            //////////////////////////////
            /// Integration Methods
            //////////////////////////////

            NekDouble Integral3D(const Array<OneD, const NekDouble>& inarray, 
                                const Array<OneD, const NekDouble>& wx,
                                const Array<OneD, const NekDouble>& wy, 
                                const Array<OneD, const NekDouble>& wz);
            NekDouble Integral(const Array<OneD, const NekDouble>& inarray);
            
            void IProductWRTBase(const Array<OneD, const NekDouble>& inarray, Array<OneD, NekDouble> & outarray);

            void IProductWRTBase(const Array<OneD, const NekDouble>& bx, 
                                 const Array<OneD, const NekDouble>& by, 
                                 const Array<OneD, const NekDouble>& bz, 
                                 const Array<OneD, const NekDouble>& inarray, 
                                 Array<OneD, NekDouble> & outarray );

            //----------------------------------
            // Local Matrix Routines
            //----------------------------------
            DNekMatSharedPtr GenMatrixHex(const StdMatrixKey &mkey);
             void GenLapMatrix(double * outarray);


            //----------------------------
            // Differentiation Methods
            //----------------------------
                
        /** \brief Calculate the deritive of the physical points 
        *
        *  For quadrilateral region can use the Tensor_Deriv function
        *  defined under StdExpansion.
        */
         void PhysDeriv( Array<OneD, NekDouble> &out_d0,
                                   Array<OneD, NekDouble> &out_d1,
                                   Array<OneD, NekDouble> &out_d2);

        /** \brief Calculate the deritive of the physical points 
        *
        *  For quadrilateral region can use the Tensor_Deriv function
        *  defined under StdExpansion.
        */
         void PhysDeriv(const Array<OneD, const NekDouble>& inarray,
                                   Array<OneD, NekDouble> &out_d0,
                                   Array<OneD, NekDouble> &out_d1,
                                   Array<OneD, NekDouble> &out_d2);
                                   
          void GetCoords(Array<OneD, NekDouble> &coords_0, 
               Array<OneD, NekDouble> &coords_1, Array<OneD, NekDouble> &coords_2);

            //----------------------------
            // Evaluations Methods
            //---------------------------

            void BwdTrans(const Array<OneD, const NekDouble>& inarray,
                          Array<OneD, NekDouble> &outarray);

            void FwdTrans(const Array<OneD, const NekDouble>& inarray,
                          Array<OneD, NekDouble> &outarray);
            NekDouble PhysEvaluate(Array<OneD, const NekDouble>& coords);


            //----------------------------------
            // Local Matrix Routines
            //----------------------------------

            DNekMatSharedPtr GenMassMatrix();

            DNekMatSharedPtr GenLaplacianMatrix();

            DNekMatSharedPtr GenLaplacianMatrix(const int i, const int j);

            DNekMatSharedPtr GenWeakDerivMatrix(const int i);

            DNekMatSharedPtr GenNBasisTransMatrix();

            DNekMatSharedPtr GenBwdTransMatrix();


            DNekMatSharedPtr GenMatrix(const StdMatrixKey &mkey)
            {
                return StdExpansion::CreateGeneralMatrix(mkey);
            }

        protected:
                
            void IProductWRTBase(const Array<OneD, const NekDouble>&base0, 
                                 const Array<OneD, const NekDouble>& base1, 
                                 const Array<OneD, const NekDouble>& base2,
                                 const Array<OneD, const NekDouble>& inarray,
                                 Array<OneD, NekDouble>& outarray, int coll_check);

        private:

            virtual int v_GetNverts() const
            {
                return 8;
            }

            virtual int v_GetNedges() const
            {
                return 12;
            }

            virtual int v_GetNfaces() const
            {
                return 6;
            }

            virtual ShapeType v_DetShapeType() const
            {
                return DetShapeType();
            };
            
            virtual DNekMatSharedPtr v_GenMatrix(const StdMatrixKey &mkey) 
            {
                return GenMatrix(mkey);
            }

            virtual DNekMatSharedPtr v_CreateStdMatrix(const StdMatrixKey &mkey)
            {
                return GenMatrix(mkey);
            }
                                  
            virtual void v_FillMode(const int mode, Array<OneD, NekDouble> &outarray)
            {
                return FillMode(mode, outarray);
            }

            virtual NekDouble v_Integral(const Array<OneD, const NekDouble>& inarray )
            {
                return Integral(inarray);
            }
            
            virtual void v_GetCoords(
                Array<OneD, NekDouble> &coords_x,
                Array<OneD, NekDouble> &coords_y,
                Array<OneD, NekDouble> &coords_z)
            {
                GetCoords(coords_x, coords_y, coords_z);
            }
            
            virtual void v_IProductWRTBase(const Array<OneD, const NekDouble>& inarray,
                Array<OneD, NekDouble> &outarray)
            {
                IProductWRTBase(inarray, outarray);
            }

            /** \brief Virtual call to GenMassMatrix */

            virtual void v_PhysDeriv( Array<OneD, NekDouble> &out_d0,
                                   Array<OneD, NekDouble> &out_d1,
                                   Array<OneD, NekDouble> &out_d2)
            {
                    PhysDeriv(out_d0, out_d1, out_d2);
            }
            virtual void v_StdPhysDeriv( Array<OneD, NekDouble> &out_d0,
                                   Array<OneD, NekDouble> &out_d1,
                                   Array<OneD, NekDouble> &out_d2)
            {
                                   
                    PhysDeriv(out_d0, out_d1, out_d2);                
            }

            virtual void v_PhysDeriv(const Array<OneD, const NekDouble>& inarray,
                                   Array<OneD, NekDouble> &out_d0,
                                   Array<OneD, NekDouble> &out_d1,
                                   Array<OneD, NekDouble> &out_d2)
            {
                    PhysDeriv(inarray, out_d0, out_d1, out_d2);
            }                                  
            virtual void v_StdPhysDeriv(const Array<OneD, const NekDouble>& inarray,
                                   Array<OneD, NekDouble> &out_d0,
                                   Array<OneD, NekDouble> &out_d1,
                                   Array<OneD, NekDouble> &out_d2)
            {
                    PhysDeriv(inarray, out_d0, out_d1, out_d2);
            }

            
            virtual void v_BwdTrans(const Array<OneD, const NekDouble>& inarray, 
                Array<OneD, NekDouble> &outarray)
            {
                BwdTrans(inarray, outarray);
            }

            virtual void v_FwdTrans(const Array<OneD, const NekDouble>& inarray, 
                Array<OneD, NekDouble> &outarray)
            {
                FwdTrans(inarray, outarray);
            }
            
            virtual NekDouble v_PhysEvaluate(Array<OneD, const NekDouble>& Lcoords)
            {
                return PhysEvaluate(Lcoords);
            }

            virtual int v_GetEdgeNcoeffs(const int i) const
            {
                return GetEdgeNcoeffs(i);
            }
            
            virtual LibUtilities::BasisType v_GetEdgeBasisType(const int i) const
            {
                return GetEdgeBasisType(i);
            }
            
             virtual void v_GenMassMatrix(Array<OneD, NekDouble> & outarray) 
            {
                 std::cout << "Implement me" << std::endl;
                 return;
            } 
            
            virtual void v_GenLapMatrix (Array<OneD, NekDouble> & outarray)
            {
                std::cout << "Implement me" << std::endl;
                return;
            }
            
            virtual DNekMatSharedPtr v_GetMassMatrix() 
            {
                std::cout << "Implement me" << std::endl;
                int foo = 0;
                return DNekMatSharedPtr();
            } 
            
            virtual DNekMatSharedPtr v_GetLapMatrix()
            {
                std::cout << "Implement me" << std::endl;
                int foo = 0;
                return DNekMatSharedPtr();
            }  

        };
        typedef boost::shared_ptr<StdHexExp> StdHexExpSharedPtr;


    } //end of namespace
} //end of namespace

#endif //STDHEXEXP_H

/**
* $Log: StdHexExp.h,v $
* Revision 1.17  2008/04/06 06:04:15  bnelson
* Changed ConstArray to Array<const>
*
* Revision 1.16  2008/03/25 08:39:45  ehan
* Added GetEdgeNcoeffs() and GetEdgeBasisType().
*
* Revision 1.15  2008/03/17 10:37:32  pvos
* Clean up of the code
*
* Revision 1.14  2008/01/20 06:09:37  bnelson
* Fixed visual c++ compile errors.
*
* Revision 1.13  2008/01/08 22:30:43  ehan
* Clean up the codes.
*
* Revision 1.12  2007/12/17 13:03:51  sherwin
* Modified StdMatrixKey to contain a list of constants and GenMatrix to take a StdMatrixKey
*
* Revision 1.11  2007/12/01 00:52:32  ehan
* Completed implementing and testing following functions:
* Integral, IProductWRTBase, PhysDeriv. BwdTrans, FwdTrans, and PhysEvaluate.
*
* Revision 1.10  2007/07/20 02:16:54  bnelson
* Replaced boost::shared_ptr with Nektar::ptr
*
* Revision 1.9  2007/07/10 21:05:16  kirby
* even more fixes
*
* Revision 1.7  2007/01/17 16:36:58  pvos
* updating doxygen documentation
*
* Revision 1.6  2007/01/17 16:05:40  pvos
* updated doxygen documentation
*
* Revision 1.5  2006/12/10 19:00:54  sherwin
* Modifications to handle nodal expansions
*
* Revision 1.4  2006/07/02 17:16:18  sherwin
*
* Modifications to make MultiRegions work for a connected domain in 2D (Tris)
*
* Revision 1.3  2006/06/01 14:13:36  kirby
* *** empty log message ***
*
* Revision 1.2  2006/05/23 15:08:19  jfrazier
* Minor cleanup to correct compile warnings.
*
* Revision 1.1  2006/05/04 18:58:31  kirby
* *** empty log message ***
*
* Revision 1.30  2006/03/12 14:20:44  sherwin
*
* First compiling version of SpatialDomains and associated modifications
*
* Revision 1.29  2006/03/06 17:12:45  sherwin
*
* Updated to properly execute all current StdRegions Demos.
*
* Revision 1.28  2006/03/04 20:26:54  bnelson
* Added comments after #endif.
*
* Revision 1.27  2006/03/01 08:25:04  sherwin
*
* First compiling version of StdRegions
*
* Revision 1.26  2006/02/27 23:47:23  sherwin
*
* Standard coding update upto compilation of StdHexExp.cpp
*
*
**/



