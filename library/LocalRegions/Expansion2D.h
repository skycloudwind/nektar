///////////////////////////////////////////////////////////////////////////////
//
// File Expansion2D.h
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
// Description: Header file for Expansion2D routines
//
///////////////////////////////////////////////////////////////////////////////

#ifndef EXPANSION2D_H

#include <LocalRegions/Expansion.h>
#include <LocalRegions/Expansion1D.h>

namespace Nektar
{
    namespace LocalRegions 
    {
        
        class Expansion2D: public Expansion
        {
        public:
            
            
            // Hybridized DG routines 
            void AddEdgeNormBoundaryInt(const int edge, 
                                        StdRegions::StdExpansion1DSharedPtr &EdgeExp,
                                        Array<OneD, NekDouble> &Fx,  
                                        Array<OneD, NekDouble> &Fy,  
                                        Array<OneD, NekDouble> &outarray);


            void SetTraceToGeomOrientation(Array<OneD, StdRegions::StdExpansion1DSharedPtr> &EdgeExp,
                                           Array<OneD, NekDouble> &inout);

            void AddNormTraceInt(const int dir,
                                 Array<OneD, const NekDouble> &inarray,
                                 Array<OneD,StdRegions::StdExpansion1DSharedPtr> &EdgeExp,
                                 Array<OneD,NekDouble> &outarray);
                
            void AddHDGHelmholtzMatrixBoundaryTerms(const NekDouble tau, 
                                                    const Array<OneD,
                                                    const NekDouble> &inarray,
                                                    Array<OneD, StdRegions::StdExpansion1DSharedPtr > &EdgeExp,
                                                    Array<OneD,NekDouble> &outarray);
                

            void AddHDGHelmholtzTraceTerms(const NekDouble tau, 
                                           const Array<OneD, const NekDouble> &inarray, Array<OneD,StdRegions::StdExpansion1DSharedPtr> &EdgeExp, Array<OneD,NekDouble> &outarray);
            
            
        protected:
            DNekMatSharedPtr GenMatrix(const StdRegions::StdMatrixKey &mkey);

            void AddHDGHelmholtzEdgeTerms(const NekDouble tau, 
                                          const int edge,
                                          Array <OneD,  StdRegions::StdExpansion1DSharedPtr > &EdgeExp, Array <OneD,NekDouble > &outarray);
            
            void AddEdgeBoundaryInt(const int edge, 
                                    StdRegions::StdExpansion1DSharedPtr &EdgeExp,
                                    Array <OneD,NekDouble > &outarray);
            

            private:
            // Do not add members here since it may lead to conflicts.
            // Only use this class for member functions

            virtual StdRegions::EdgeOrientation v_GetEorient(int edge)
            {
                NEKERROR(ErrorUtil::efatal, "This function is only valid for two-dimensional  LocalRegions");  
                return StdRegions::eForwards;              
            }


            virtual void v_GetEdgeToElementMap(const int eid, const StdRegions::EdgeOrientation edgeOrient, Array<OneD, unsigned int> &maparray, Array<OneD, int> &signarray)
            {
                NEKERROR(ErrorUtil::efatal,"Method does not exist for this shape" );
            }
            

            virtual int v_GetCoordim(void)
            {
                NEKERROR(ErrorUtil::efatal,  "Methods not valid in this class");        
                return -1;
            }

            virtual int v_GetNedges(void) const
            {
                NEKERROR(ErrorUtil::efatal, "Methods not valid in this class");        
                return -1;
            }

            virtual int v_GetEdgeNcoeffs(const int i) const
            {
                ASSERTL0(false, "This function is not valid or not defined");
                return 0;
            }


            virtual DNekMatSharedPtr v_GenMatrix(const StdRegions::StdMatrixKey &mkey)
            {
                return GenMatrix(mkey);
            }
            
            virtual StdRegions::StdExpansion1DSharedPtr v_GetEdgeExp(const int edge, bool SetUpNormals=true)
            {
                ASSERTL0(false,"Function only currently valid for 2D Local expansions");

            }

            virtual void v_GetEdgePhysVals(const int edge, const Array<OneD, const NekDouble> &inarray, Array<OneD,NekDouble> &outarray)
            {
                NEKERROR(ErrorUtil::efatal,"Method does not exist for this shape or library" );
            }
        

            virtual void v_GetEdgePhysVals(const int edge,  const boost::shared_ptr<StdRegions::StdExpansion1D>  &EdgeExp, const Array<OneD, const NekDouble> &inarray, Array<OneD,NekDouble> &outarray)
            {
                NEKERROR(ErrorUtil::efatal,"Method does not exist for this shape or library" );
            }

            
         };
        
        // type defines for use of PrismExp in a boost vector
        typedef boost::shared_ptr<Expansion2D> Expansion2DSharedPtr;
        typedef std::vector< Expansion2DSharedPtr > Expansion2DVector;
        typedef std::vector< Expansion2DSharedPtr >::iterator Expansion2DVectorIter;
        
    } //end of namespace
} //end of namespace

#define EXPANSION2D_H
#endif

/** 
 *    $Log: Expansion2D.h,v $
 *    Revision 1.1  2008/08/14 22:12:56  sherwin
 *    Introduced Expansion classes and used them to define HDG routines, has required quite a number of virtual functions to be added
 *
 *
 **/