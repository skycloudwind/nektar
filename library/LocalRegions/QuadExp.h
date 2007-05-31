///////////////////////////////////////////////////////////////////////////////
//
// File QuadExp.h
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

#ifndef QUADEXP_H
#define QUADEXP_H

#include <LocalRegions/LocalRegions.hpp>

#include <StdRegions/StdQuadExp.h>
#include <SpatialDomains/QuadGeom.h>

#include <SpatialDomains/GeomFactors.h>

#include <LocalRegions/MatrixKey.h>
#include <LocalRegions/LinSys.hpp>

namespace Nektar
{

    namespace LocalRegions
    {

    class QuadExp: public StdRegions::StdQuadExp
    {
    public:

        /** \brief Constructor using BasisKey class for quadrature
        points and order definition */
        QuadExp(const LibUtilities::BasisKey &Ba,
                const LibUtilities::BasisKey &Bb,
                SpatialDomains::QuadGeomSharedPtr &geom);
        
        /// Copy Constructor
        QuadExp(const QuadExp &T);

        /// Destructor
        ~QuadExp();

        /// Return Shape of region, using ShapeType enum
        /// list. i.e. Quadrilateral
        StdRegions::ShapeType DetShapeType()
        {
            return StdRegions::eQuadrilateral;
        }

        void GetCoords(Array<OneD,NekDouble> &coords_1,
                       Array<OneD,NekDouble> &coords_2 = NullNekDouble1DArray, 
                       Array<OneD,NekDouble> &coords_3 = NullNekDouble1DArray);
        void GetCoord(const ConstArray<OneD,NekDouble>& Lcoords, 
                      Array<OneD,NekDouble> &coords);

        SpatialDomains::QuadGeomSharedPtr GetGeom()
        {
            return m_geom;
        }


        void WriteToFile(FILE *outfile);
        void WriteToFile(std::ofstream &outfile, const int dumpVar);


        //----------------------------
        // Integration Methods
        //----------------------------

         /// \brief Integrate the physical point list \a inarray over region
        NekDouble Integral(const ConstArray<OneD, NekDouble> &inarray);


        /** \brief  Inner product of \a inarray over region with respect to the
        expansion basis (this)->_Base[0] and return in \a outarray */
        void IProductWRTBase(const ConstArray<OneD, NekDouble> &inarray, 
                             Array<OneD, NekDouble> &outarray);

        //-----------------------------
        // Differentiation Methods
        //-----------------------------

        void PhysDeriv(const ConstArray<OneD, NekDouble> &inarray, 
                       Array<OneD, NekDouble> &out_d0 = NullNekDouble1DArray,
                       Array<OneD, NekDouble> &out_d1 = NullNekDouble1DArray,
                       Array<OneD, NekDouble> &out_d2 = NullNekDouble1DArray);      
        
        //----------------------------
        // Evaluations Methods
        //---------------------------
        
        /** \brief Forward transform from physical quadrature space
            stored in \a inarray and evaluate the expansion coefficients and
            store in \a (this)->_coeffs  */
        void FwdTrans(const ConstArray<OneD, NekDouble> &inarray, 
                      Array<OneD, NekDouble> &outarray);
        
        NekDouble PhysEvaluate(const ConstArray<OneD, NekDouble> &coord);        
        
    protected:

        void GenMetricInfo();

        DNekMatSharedPtr    CreateMatrix(const MatrixKey &mkey);
        DNekLinSysSharedPtr CreateLinSys(const LinSysKey &mkey);

        SpatialDomains::QuadGeomSharedPtr m_geom;
        SpatialDomains::GeomFactorsSharedPtr  m_metricinfo;

        LibUtilities::NekManager<MatrixKey, DNekMat, MatrixKey::opLess> m_matrixManager;
        
        LibUtilities::NekManager<LinSysKey, DNekLinSys, LinSysKey::opLess> m_linSysManager;

        /** \brief  Inner product of \a inarray over region with respect to
        the expansion basis \a base and return in \a outarray */
        inline void IProductWRTBase(const ConstArray<OneD, NekDouble> &base0, 
                                    const ConstArray<OneD, NekDouble> &base1, 
                                    const ConstArray<OneD, NekDouble> &inarray, 
                                    Array<OneD, NekDouble> &outarray, 
                                    const int coll_check);
        
    private:
        virtual StdRegions::ShapeType v_DetShapeType()
        {
            return DetShapeType();
        }
	
        virtual SpatialDomains::GeomFactorsSharedPtr v_GetMetricInfo() const
        {
            return m_metricinfo;
        }

        virtual void v_GetCoords(Array<OneD, NekDouble> &coords_0,
                                 Array<OneD, NekDouble> &coords_1 = NullNekDouble1DArray,
                                 Array<OneD, NekDouble> &coords_2 = NullNekDouble1DArray)
	 {
             GetCoords(coords_0, coords_1, coords_2);
         }

        virtual void v_GetCoord(const ConstArray<OneD, NekDouble> &lcoord, 
                                Array<OneD, NekDouble> &coord)
        {
            GetCoord(lcoord, coord);
        }

        virtual  int v_GetCoordim()
        {
	    return m_geom->GetCoordim();
        }


        virtual void v_WriteToFile(FILE *outfile)
        {
	    WriteToFile(outfile);
        }

        virtual void v_WriteToFile(std::ofstream &outfile, const int dumpVar)
        {
	    WriteToFile(outfile,dumpVar);
        }

        /** \brief Virtual call to integrate the physical point list \a inarray
        over region (see SegExp::Integral) */
        virtual NekDouble v_Integral(const ConstArray<OneD, NekDouble> &inarray )
        {
            return Integral(inarray);
        }

        /** \brief Virtual call to QuadExp::IProduct_WRT_B */
        virtual void v_IProductWRTBase(const ConstArray<OneD, NekDouble> &inarray,
                                       Array<OneD, NekDouble> &outarray)
        {
            IProductWRTBase(inarray,outarray);
        }


        /// Virtual call to SegExp::PhysDeriv
        virtual void v_StdPhysDeriv(const ConstArray<OneD, NekDouble> &inarray, 
                                    Array<OneD, NekDouble> &out_d0,
                                    Array<OneD, NekDouble> &out_d1)
        {
            StdQuadExp::PhysDeriv(inarray, out_d0, out_d1);
        }
        
        virtual void v_PhysDeriv(const ConstArray<OneD, NekDouble> &inarray, 
                                 Array<OneD, NekDouble> &out_d0 = NullNekDouble1DArray,
                                 Array<OneD, NekDouble> &out_d1 = NullNekDouble1DArray,
                                 Array<OneD, NekDouble> &out_d2 = NullNekDouble1DArray)
	 {
             PhysDeriv(inarray, out_d0, out_d1, out_d2);
         }
        
        virtual void v_StdDeriv(const ConstArray<OneD, NekDouble> &inarray, 
                                Array<OneD, NekDouble> &out_d0,
                                Array<OneD, NekDouble> &out_d1)

        {
	    StdQuadExp::PhysDeriv(inarray, out_d0, out_d1);
        }
	
        /// Virtual call to SegExp::FwdTrans
       virtual void v_FwdTrans(const ConstArray<OneD, NekDouble> &inarray, 
                                Array<OneD, NekDouble> &outarray)
        {
            FwdTrans(inarray,outarray);
        }
	
        /// Virtual call to QuadExp::Evaluate
        virtual NekDouble v_PhysEvaluate(const ConstArray<OneD, NekDouble> &coords)
        {
            return PhysEvaluate(coords);
        }
	
        virtual NekDouble v_Linf(const ConstArray<OneD, NekDouble> &sol)
        {
            return Linf(sol);
        }
	
        
        virtual NekDouble v_Linf()
        {
            return Linf();
        }
	
        virtual NekDouble v_L2(const ConstArray<OneD, NekDouble> &sol)
        {
            return StdExpansion::L2(sol);
        }

	
        virtual NekDouble v_L2()
        {
            return StdExpansion::L2();
        }
    };

    // type defines for use of QuadExp in a boost vector
    typedef boost::shared_ptr<QuadExp> QuadExpSharedPtr;
    typedef std::vector< QuadExpSharedPtr > QuadExpVector;
    typedef std::vector< QuadExpSharedPtr >::iterator QuadExpVectorIter;


    } //end of namespace
} //end of namespace

#endif

/**
 *    $Log: QuadExp.h,v $
 *    Revision 1.10  2007/05/27 16:10:28  bnelson
 *    Update to new Array type.
 *
 *    Revision 1.9  2007/04/26 15:00:16  sherwin
 *    SJS compiling working version using SHaredArrays
 *
 *    Revision 1.8  2007/01/15 21:12:26  sherwin
 *    First definition
 *
 *    Revision 1.7  2006/06/13 18:05:01  sherwin
 *    Modifications to make MultiRegions demo ProjectLoc2D execute properly.
 *
 *    Revision 1.6  2006/06/05 00:10:01  bnelson
 *    Fixed a gcc 4.1.0 compilation error (ClassName::method not allowed in class definition).
 *
 *    Revision 1.5  2006/06/02 18:48:39  sherwin
 *    Modifications to make ProjectLoc2D run bit there are bus errors for order > 3
 *
 *    Revision 1.4  2006/06/01 14:15:57  sherwin
 *    Added typdef of boost wrappers and made GeoFac a boost shared pointer.
 *
 *    Revision 1.3  2006/05/30 14:00:03  sherwin
 *    Updates to make MultiRegions and its Demos work
 *
 *    Revision 1.2  2006/05/29 17:05:49  sherwin
 *    Modified to put shared_ptr around geom definitions
 *
 *    Revision 1.1  2006/05/04 18:58:46  kirby
 *    *** empty log message ***
 *
 *    Revision 1.16  2006/03/12 21:59:48  sherwin
 *
 *    compiling version of LocalRegions
 *
 *    Revision 1.15  2006/03/12 07:43:32  sherwin
 *
 *    First revision to meet coding standard. Needs to be compiled
 *
 **/
