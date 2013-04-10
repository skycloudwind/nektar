///////////////////////////////////////////////////////////////////////////////
//
// File Preconditioner.h
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
// Description: Preconditioner header
//
///////////////////////////////////////////////////////////////////////////////
#ifndef NEKTAR_LIB_MULTIREGIONS_PRECONDITIONERLOWENERGY_H
#define NEKTAR_LIB_MULTIREGIONS_PRECONDITIONERLOWENERGY_H
#include <MultiRegions/GlobalLinSys.h>
#include <MultiRegions/Preconditioner.h>
#include <MultiRegions/MultiRegionsDeclspec.h>
#include <MultiRegions/AssemblyMap/AssemblyMapCG.h>
#include <LocalRegions/TetExp.h>
#include <LocalRegions/PrismExp.h>


namespace Nektar
{
    namespace MultiRegions
    {
        class PreconditionerLowEnergy;
        typedef boost::shared_ptr<PreconditionerLowEnergy>  PreconditionerLowEnergySharedPtr;

        class PreconditionerLowEnergy: public Preconditioner
	{
        public:
            /// Creates an instance of this class
            static PreconditionerSharedPtr create(
                        const boost::shared_ptr<GlobalLinSys> &plinsys,
                        const boost::shared_ptr<AssemblyMap>
                                                               &pLocToGloMap)
            {
	        PreconditionerSharedPtr p = MemoryManager<PreconditionerLowEnergy>::AllocateSharedPtr(plinsys,pLocToGloMap);
	        p->InitObject();
	        return p;
            }

            /// Name of class
            static std::string className;

            MULTI_REGIONS_EXPORT PreconditionerLowEnergy(
                         const boost::shared_ptr<GlobalLinSys> &plinsys,
	                 const AssemblyMapSharedPtr &pLocToGloMap);

            MULTI_REGIONS_EXPORT
            virtual ~PreconditionerLowEnergy() {}

	protected:

            const boost::weak_ptr<GlobalLinSys>         m_linsys;

	    DNekBlkMatSharedPtr                         BlkMat;
            DNekScalMatSharedPtr                        bnd_mat;

	    DNekScalMatSharedPtr                        m_TetR;
	    DNekScalMatSharedPtr                        m_TetRT;
	    DNekScalMatSharedPtr                        m_PrismR;
	    DNekScalMatSharedPtr                        m_PrismRT;

            DNekScalBlkMatSharedPtr                     m_schurCompl;
            DNekScalBlkMatSharedPtr                     m_BinvD;
            DNekScalBlkMatSharedPtr                     m_C;
            DNekScalBlkMatSharedPtr                     m_invD;

            DNekScalBlkMatSharedPtr                     m_RBlk;
            DNekScalBlkMatSharedPtr                     m_RTBlk;
            DNekScalBlkMatSharedPtr                     m_InvRBlk;
            DNekScalBlkMatSharedPtr                     m_InvRTBlk;

            DNekScalBlkMatSharedPtr                     m_S1Blk;

            boost::shared_ptr<AssemblyMap>              m_locToGloMap;

            DNekScalMatSharedPtr Rtet;
            DNekScalMatSharedPtr RTtet;
            DNekScalMatSharedPtr InvRtet;
            DNekScalMatSharedPtr InvRTtet;
            DNekScalMatSharedPtr Rprismmod;
            DNekScalMatSharedPtr RTprismmod;
            DNekScalMatSharedPtr InvRprism;
            DNekScalMatSharedPtr InvRTprism;

            Array<OneD, NekDouble>      m_locToGloSignMult;

	private:

            void SetUpReferenceElements(void);

            void CreateMultiplicityMap(void);

            void SetupBlockTransformationMatrix(void);

            void ModifyPrismTransformationMatrix(
                LocalRegions::TetExpSharedPtr TetExp,
                LocalRegions::PrismExpSharedPtr PrismExp,
                DNekMatSharedPtr Rmodprism,
                DNekMatSharedPtr RTmodprism);

            SpatialDomains::TetGeomSharedPtr CreateRefTetGeom(void);
            SpatialDomains::PrismGeomSharedPtr CreateRefPrismGeom(void);

            virtual void v_InitObject();

            virtual void v_DoPreconditioner(                
                const Array<OneD, NekDouble>& pInput,
                Array<OneD, NekDouble>& pOutput);

            virtual void v_DoTransformToLowEnergy(
                Array<OneD, NekDouble>& pInOut,
                int offset);

            virtual void v_DoTransformToLowEnergy(
                const Array<OneD, NekDouble>& pInput,
                Array<OneD, NekDouble>& pOutput);

            virtual void v_DoTransformFromLowEnergy(
                Array<OneD, NekDouble>& pInOut);

            virtual void v_DoMultiplybyInverseTransformationMatrix(
                const Array<OneD, NekDouble>& pInput,
                Array<OneD, NekDouble>& pOutput);

            virtual void v_DoMultiplybyInverseTransposedTransformationMatrix(
                const Array<OneD, NekDouble>& pInput,
                Array<OneD, NekDouble>& pOutput);
            
            virtual void v_BuildPreconditioner();
            
            virtual DNekScalBlkMatSharedPtr
                v_TransformedSchurCompl(int offset, const boost::shared_ptr<DNekScalBlkMat > &loc_mat);
        };
    }
}

#endif
