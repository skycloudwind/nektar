///////////////////////////////////////////////////////////////////////////////
//
// File TriExp.cpp
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
#include <LocalRegions/LocalRegions.h>
#include <LocalRegions/LocalRegions.hpp>
#include <stdio.h>
#include <LocalRegions/TriExp.h>
#include <StdRegions/StdNodalTriExp.h>

namespace Nektar
{
    namespace LocalRegions
    {
        TriExp::TriExp(const LibUtilities::BasisKey &Ba,
                       const LibUtilities::BasisKey &Bb,
                       const SpatialDomains::TriGeomSharedPtr &geom):
            StdRegions::StdTriExp(Ba,Bb),
            m_geom(geom),
            m_metricinfo(m_geom->GetGeomFactors(m_base)),
            m_matrixManager(std::string("TriExpMatrix")),
            m_staticCondMatrixManager(std::string("TriExpStaticCondMatrix"))
        {
            for(int i = 0; i < StdRegions::SIZE_MatrixType; ++i)
            {
                m_matrixManager.RegisterCreator(MatrixKey((StdRegions::MatrixType) i,StdRegions::eNoExpansionType,*this),
                                                boost::bind(&TriExp::CreateMatrix, this, _1));
                m_staticCondMatrixManager.RegisterCreator(MatrixKey((StdRegions::MatrixType) i, StdRegions::eNoExpansionType,*this),
                                                          boost::bind(&TriExp::CreateStaticCondMatrix, this, _1));
            }
        }

        TriExp::TriExp(const TriExp &T):
            StdRegions::StdTriExp(T),
            m_geom(T.m_geom),
            m_metricinfo(T.m_metricinfo),
            m_matrixManager(std::string("TriExpMatrix")),
            m_staticCondMatrixManager(std::string("TriExpStaticCondMatrix"))
        {
        }

        TriExp::~TriExp()
        {
        }


        //----------------------------
        // Integration Methods
        //----------------------------

        /** \brief Integrate the physical point list \a inarray over region
            and return the value

            Inputs:\n

            - \a inarray: definition of function to be returned at quadrature point
            of expansion.

            Outputs:\n

            - returns \f$\int^1_{-1}\int^1_{-1} u(\eta_1, \eta_2) J[i,j] d
            \eta_1 d \eta_2 \f$ where \f$inarray[i,j] = u(\eta_{1i},\eta_{2j})
            \f$ and \f$ J[i,j] \f$ is the Jacobian evaluated at the
            quadrature point.
        */
        NekDouble TriExp::Integral(const Array<OneD, const NekDouble> &inarray)
        {
            int    nquad0 = m_base[0]->GetNumPoints();
            int    nquad1 = m_base[1]->GetNumPoints();
            Array<OneD, const NekDouble> jac = m_metricinfo->GetJac();
            NekDouble ival;
            Array<OneD,NekDouble> tmp(nquad0*nquad1);

            // multiply inarray with Jacobian
            if(m_metricinfo->GetGtype() == SpatialDomains::eDeformed)
            {
                Vmath::Vmul(nquad0*nquad1, jac, 1, inarray, 1,tmp, 1);
            }
            else
            {
                Vmath::Smul(nquad0*nquad1, jac[0], inarray, 1, tmp, 1);
            }

            // call StdQuadExp version;
            ival = StdTriExp::Integral(tmp);
            return ival;
        }

        void TriExp::GeneralMatrixOp_MatOp(const Array<OneD, const NekDouble> &inarray,
                                           Array<OneD,NekDouble> &outarray,
                                           const StdRegions::StdMatrixKey &mkey)
        {
            int nConsts = mkey.GetNconstants();
            DNekScalMatSharedPtr   mat;

            switch(nConsts)
            {
            case 0:
                {
                    mat = GetLocMatrix(mkey.GetMatrixType());
                }
                break;
            case 1:
                {
                    mat = GetLocMatrix(mkey.GetMatrixType(),mkey.GetConstant(0));
                }
                break;
            case 2:
                {
                    mat = GetLocMatrix(mkey.GetMatrixType(),mkey.GetConstant(0),mkey.GetConstant(1));
                }
                break;

            default:
                {
                    NEKERROR(ErrorUtil::efatal, "Unknown number of constants");
                }
                break;
            }

            if(inarray.get() == outarray.get())
            {
                Array<OneD,NekDouble> tmp(m_ncoeffs);
                Vmath::Vcopy(m_ncoeffs,inarray.get(),1,tmp.get(),1);

                Blas::Dgemv('N',m_ncoeffs,m_ncoeffs,mat->Scale(),(mat->GetOwnedMatrix())->GetPtr().get(),
                            m_ncoeffs, tmp.get(), 1, 0.0, outarray.get(), 1);
            }
            else
            {
                Blas::Dgemv('N',m_ncoeffs,m_ncoeffs,mat->Scale(),(mat->GetOwnedMatrix())->GetPtr().get(),
                            m_ncoeffs, inarray.get(), 1, 0.0, outarray.get(), 1);
            }
        }


        void TriExp::MultiplyByQuadratureMetric(const Array<OneD, const NekDouble>& inarray,
                                                Array<OneD, NekDouble> &outarray)
        {
            if(m_metricinfo->IsUsingQuadMetrics())
            {
                int    nqtot = m_base[0]->GetNumPoints()*m_base[1]->GetNumPoints();
                const Array<OneD, const NekDouble>& metric = m_metricinfo->GetQuadratureMetrics();

                Vmath::Vmul(nqtot, metric, 1, inarray, 1, outarray, 1);
            }
            else
            {
                int    i;
                int    nquad0 = m_base[0]->GetNumPoints();
                int    nquad1 = m_base[1]->GetNumPoints();
                int    nqtot  = nquad0*nquad1;

                const Array<OneD, const NekDouble>& jac = m_metricinfo->GetJac();
                const Array<OneD, const NekDouble>& w0 = m_base[0]->GetW();
                const Array<OneD, const NekDouble>& w1 = m_base[1]->GetW();
                const Array<OneD, const NekDouble>& z1 = m_base[1]->GetZ();

                if(m_metricinfo->GetGtype() == SpatialDomains::eDeformed)
                {
                    Vmath::Vmul(nqtot, jac, 1, inarray, 1, outarray, 1);
                }
                else
                {
                    Vmath::Smul(nqtot, jac[0], inarray, 1, outarray, 1);
                }

                // multiply by integration constants
                for(i = 0; i < nquad1; ++i)
                {
                    Vmath::Vmul(nquad0,outarray.get()+i*nquad0,1,
                                w0.get(),1, outarray.get()+i*nquad0,1);
                }

                switch(m_base[1]->GetPointsType())
                {
                case LibUtilities::eGaussLobattoLegendre:  // Legendre inner product
                    for(i = 0; i < nquad1; ++i)
                    {
                        Blas::Dscal(nquad0,0.5*(1-z1[i])*w1[i], outarray.get()+i*nquad0,1);
                    }
                    break;
                case LibUtilities::eGaussRadauMAlpha1Beta0: // (1,0) Jacobi Inner product
                    for(i = 0; i < nquad1; ++i)
                    {
                        Blas::Dscal(nquad0,0.5*w1[i], outarray.get()+i*nquad0,1);
                    }
                    break;
                }
            }
        }

        void TriExp::LaplacianMatrixOp_MatFree(const Array<OneD, const NekDouble> &inarray,
                                                      Array<OneD,NekDouble> &outarray,
                                                      const StdRegions::StdMatrixKey &mkey)
        {
            if(mkey.GetNvariableLaplacianCoefficients() == 0)
            {
                // This implementation is only valid when there are no coefficients
                // associated to the Laplacian operator
                if(m_metricinfo->IsUsingLaplMetrics())
                {
                    int       nquad0  = m_base[0]->GetNumPoints();
                    int       nquad1  = m_base[1]->GetNumPoints();
                    int       nqtot   = nquad0*nquad1;
                    int       nmodes0 = m_base[0]->GetNumModes();
                    int       nmodes1 = m_base[1]->GetNumModes();
                    int       wspsize = max(max(max(nqtot,m_ncoeffs),nquad1*nmodes0),nquad0*nmodes1);

                    const Array<OneD, const NekDouble>& base0  = m_base[0]->GetBdata();
                    const Array<OneD, const NekDouble>& base1  = m_base[1]->GetBdata();
                    const Array<OneD, const NekDouble>& dbase0 = m_base[0]->GetDbdata();
                    const Array<OneD, const NekDouble>& dbase1 = m_base[1]->GetDbdata();
                    const Array<TwoD, const NekDouble>& metric = m_metricinfo->GetLaplacianMetrics();

                    // Allocate temporary storage
                    Array<OneD,NekDouble> wsp0(3*wspsize);
                    Array<OneD,NekDouble> wsp1(wsp0+wspsize);
                    Array<OneD,NekDouble> wsp2(wsp0+2*wspsize);

                    // LAPLACIAN MATRIX OPERATION
                    // wsp0 = u       = B   * u_hat
                    // wsp1 = du_dxi1 = D_xi1 * wsp0 = D_xi1 * u
                    // wsp2 = du_dxi2 = D_xi2 * wsp0 = D_xi2 * u
                    BwdTrans_SumFacKernel(base0,base1,inarray,wsp0,wsp1);
                    StdExpansion2D::PhysTensorDeriv(wsp0,wsp1,wsp2);

                    // wsp0 = k = g0 * wsp1 + g1 * wsp2 = g0 * du_dxi1 + g1 * du_dxi2
                    // wsp2 = l = g1 * wsp1 + g2 * wsp2 = g1 * du_dxi1 + g2 * du_dxi2
                    // where g0, g1 and g2 are the metric terms set up in the GeomFactors class
                    // especially for this purpose
                    Vmath::Vvtvvtp(nqtot,&metric[0][0],1,&wsp1[0],1,&metric[1][0],1,&wsp2[0],1,&wsp0[0],1);
                    Vmath::Vvtvvtp(nqtot,&metric[1][0],1,&wsp1[0],1,&metric[2][0],1,&wsp2[0],1,&wsp2[0],1);

                    // outarray = m = (D_xi1 * B)^T * k
                    // wsp1     = n = (D_xi2 * B)^T * l
                    IProductWRTBase_SumFacKernel(dbase0,base1,wsp0,outarray,wsp1);
                    IProductWRTBase_SumFacKernel(base0,dbase1,wsp2,wsp1,    wsp0);

                    // outarray = outarray + wsp1
                    //          = L * u_hat
                    Vmath::Vadd(m_ncoeffs,wsp1.get(),1,outarray.get(),1,outarray.get(),1);
                }
                else
                {
                    int       i;
                    int       dim = m_geom->GetCoordim();
                    int       nquad0  = m_base[0]->GetNumPoints();
                    int       nquad1  = m_base[1]->GetNumPoints();
                    int       nqtot   = nquad0*nquad1;
                    int       nmodes0 = m_base[0]->GetNumModes();
                    int       nmodes1 = m_base[1]->GetNumModes();
                    int       wspsize = max(max(max(nqtot,m_ncoeffs),nquad1*nmodes0),nquad0*nmodes1);

                    const Array<OneD, const NekDouble>& base0  = m_base[0]->GetBdata();
                    const Array<OneD, const NekDouble>& base1  = m_base[1]->GetBdata();
                    const Array<OneD, const NekDouble>& dbase0 = m_base[0]->GetDbdata();
                    const Array<OneD, const NekDouble>& dbase1 = m_base[1]->GetDbdata();

                    // Allocate temporary storage
                    Array<OneD,NekDouble> wsp0(9*wspsize);
                    Array<OneD,NekDouble> wsp1(wsp0+wspsize);
                    Array<OneD,NekDouble> wsp2(wsp0+2*wspsize);
                    Array<OneD,NekDouble> wsp3(wsp0+3*wspsize);
                    Array<OneD,NekDouble> wsp4(wsp0+4*wspsize);
                    Array<OneD,NekDouble> wsp5(wsp0+5*wspsize);
                    Array<OneD,NekDouble> wsp6(wsp0+6*wspsize);
                    Array<OneD,NekDouble> wsp7(wsp0+7*wspsize);
                    Array<OneD,NekDouble> wsp8(wsp0+8*wspsize);

                    // LAPLACIAN MATRIX OPERATION
                    // wsp0 = u       = B   * u_hat
                    // wsp1 = du_dxi1 = D_xi1 * wsp0 = D_xi1 * u
                    // wsp2 = du_dxi2 = D_xi2 * wsp0 = D_xi2 * u
                    BwdTrans_SumFacKernel(base0,base1,inarray,wsp0,wsp1);
                    StdExpansion2D::PhysTensorDeriv(wsp0,wsp1,wsp2);

                    // wsp0 = k = g0 * wsp1 + g1 * wsp2 = g0 * du_dxi1 + g1 * du_dxi2
                    // wsp2 = l = g1 * wsp1 + g2 * wsp2 = g0 * du_dxi1 + g1 * du_dxi2
                    const Array<TwoD, const NekDouble>& gmat = m_metricinfo->GetGmat();
                    const Array<OneD, const NekDouble>& z0 = m_base[0]->GetZ();
                    const Array<OneD, const NekDouble>& z1 = m_base[1]->GetZ();
                    // substep 1: calculate the metric terms of the collapsed coordinate
                    // transformation
                    Vmath::Fill(wspsize,1.0,wsp6,1);
                    Vmath::Fill(wspsize,1.0,wsp7,1);
                    for(i = 0; i < nquad1; i++)
                    {
                        Blas::Dscal(nquad0,2.0/(1-z1[i]),&wsp6[0]+i*nquad0,1);
                        Blas::Dscal(nquad0,2.0/(1-z1[i]),&wsp7[0]+i*nquad0,1);
                    }
                    for(i = 0; i < nquad0; i++)
                    {
                        Blas::Dscal(nquad1,0.5*(1+z0[i]),&wsp7[0]+i,nquad0);
                    }
                    // substep2: calculate g0,g1,g2
                    // g0 = wsp3
                    // g1 = wsp4
                    // g1 = wsp5
                    if(m_metricinfo->GetGtype() == SpatialDomains::eDeformed)
                    {
                        Vmath::Vmul (nqtot,&gmat[0][0],1,&wsp6[0],1,&wsp8[0],1);
                        Vmath::Vvtvp(nqtot,&gmat[1][0],1,&wsp7[0],1,&wsp8[0],1,&wsp8[0],1);

                        Vmath::Vmul (nqtot,&wsp8[0],   1,&wsp8[0],   1,&wsp3[0],1);
                        Vmath::Vmul (nqtot,&gmat[1][0],1,&wsp8[0],   1,&wsp4[0],1);
                        Vmath::Vmul (nqtot,&gmat[1][0],1,&gmat[1][0],1,&wsp5[0],1);


                        Vmath::Vmul (nqtot,&gmat[2][0],1,&wsp6[0],1,&wsp8[0],1);
                        Vmath::Vvtvp(nqtot,&gmat[3][0],1,&wsp7[0],1,&wsp8[0],1,&wsp8[0],1);

                        Vmath::Vvtvp(nqtot,&wsp8[0],   1,&wsp8[0],   1,&wsp3[0],1,&wsp3[0],1);
                        Vmath::Vvtvp(nqtot,&gmat[3][0],1,&wsp8[0],   1,&wsp4[0],1,&wsp4[0],1);
                        Vmath::Vvtvp(nqtot,&gmat[3][0],1,&gmat[3][0],1,&wsp5[0],1,&wsp5[0],1);

                        if(dim == 3)
                        {
                            Vmath::Vmul (nqtot,&gmat[4][0],1,&wsp6[0],1,&wsp8[0],1);
                            Vmath::Vvtvp(nqtot,&gmat[5][0],1,&wsp7[0],1,&wsp8[0],1,&wsp8[0],1);

                            Vmath::Vvtvp(nqtot,&wsp8[0],   1,&wsp8[0],   1,&wsp3[0],1,&wsp3[0],1);
                            Vmath::Vvtvp(nqtot,&gmat[5][0],1,&wsp8[0],   1,&wsp4[0],1,&wsp4[0],1);
                            Vmath::Vvtvp(nqtot,&gmat[5][0],1,&gmat[5][0],1,&wsp5[0],1,&wsp5[0],1);
                        }
                    }
                    else
                    {
                        Vmath::Smul (nqtot,gmat[0][0],&wsp6[0],1,&wsp8[0],1);
                        Vmath::Svtvp(nqtot,gmat[1][0],&wsp7[0],1,&wsp8[0],1,&wsp8[0],1);

                        Vmath::Vmul (nqtot,&wsp8[0],1,&wsp8[0],1,&wsp3[0],1);
                        Vmath::Smul (nqtot,gmat[1][0],&wsp8[0],1,&wsp4[0],1);


                        Vmath::Smul (nqtot,gmat[2][0],&wsp6[0],1,&wsp8[0],1);
                        Vmath::Svtvp(nqtot,gmat[3][0],&wsp7[0],1,&wsp8[0],1,&wsp8[0],1);

                        Vmath::Vvtvp(nqtot,&wsp8[0],1,&wsp8[0],1,&wsp3[0],1,&wsp3[0],1);
                        Vmath::Svtvp(nqtot,gmat[3][0],&wsp8[0],1,&wsp4[0],1,&wsp4[0],1);

                        if(dim == 3)
                        {
                            Vmath::Smul (nqtot,gmat[4][0],&wsp6[0],1,&wsp8[0],1);
                            Vmath::Svtvp(nqtot,gmat[5][0],&wsp7[0],1,&wsp8[0],1,&wsp8[0],1);

                            Vmath::Vvtvp(nqtot,&wsp8[0],1,&wsp8[0],1,&wsp3[0],1,&wsp3[0],1);
                            Vmath::Svtvp(nqtot,gmat[5][0],&wsp8[0],1,&wsp4[0],1,&wsp4[0],1);
                        }

                        NekDouble g2 = gmat[1][0]*gmat[1][0] + gmat[3][0]*gmat[3][0];
                        if(dim == 3)
                        {
                            g2 += gmat[5][0]*gmat[5][0];
                        }
                        Vmath::Fill(nqtot,g2,&wsp5[0],1);
                    }
                    // substep 3:
                    // wsp0 = k = wsp3 * wsp1 + wsp4 * wsp2 = g0 * du_dxi1 + g1 * du_dxi2
                    // wsp2 = l = wsp4 * wsp1 + wsp5 * wsp2 = g0 * du_dxi1 + g1 * du_dxi2
                    Vmath::Vvtvvtp(nqtot,&wsp3[0],1,&wsp1[0],1,&wsp4[0],1,&wsp2[0],1,&wsp0[0],1);
                    Vmath::Vvtvvtp(nqtot,&wsp4[0],1,&wsp1[0],1,&wsp5[0],1,&wsp2[0],1,&wsp2[0],1);
                    // substep 4: multiply by jacobian and quadrature weights
                    MultiplyByQuadratureMetric(wsp0,wsp0);
                    MultiplyByQuadratureMetric(wsp2,wsp2);

                    // outarray = m = (D_xi1 * B)^T * k
                    // wsp1     = n = (D_xi2 * B)^T * l
                    IProductWRTBase_SumFacKernel(dbase0,base1,wsp0,outarray,wsp1);
                    IProductWRTBase_SumFacKernel(base0,dbase1,wsp2,wsp1,    wsp0);

                    // outarray = outarray + wsp1
                    //          = L * u_hat
                    Vmath::Vadd(m_ncoeffs,wsp1.get(),1,outarray.get(),1,outarray.get(),1);
                }
            }
            else
            {
                StdExpansion::LaplacianMatrixOp_MatFree_GenericImpl(inarray,outarray,mkey);
            }
        }

        void TriExp::HelmholtzMatrixOp_MatFree(const Array<OneD, const NekDouble> &inarray,
                                                      Array<OneD,NekDouble> &outarray,
                                                      const StdRegions::StdMatrixKey &mkey)
        {
            if(m_metricinfo->IsUsingLaplMetrics())
            {
                int       nquad0  = m_base[0]->GetNumPoints();
                int       nquad1  = m_base[1]->GetNumPoints();
                int       nqtot   = nquad0*nquad1;
                int       nmodes0 = m_base[0]->GetNumModes();
                int       nmodes1 = m_base[1]->GetNumModes();
                int       wspsize = max(max(max(nqtot,m_ncoeffs),nquad1*nmodes0),nquad0*nmodes1);
                NekDouble lambda  = mkey.GetConstant(0);

                const Array<OneD, const NekDouble>& base0  = m_base[0]->GetBdata();
                const Array<OneD, const NekDouble>& base1  = m_base[1]->GetBdata();
                const Array<OneD, const NekDouble>& dbase0 = m_base[0]->GetDbdata();
                const Array<OneD, const NekDouble>& dbase1 = m_base[1]->GetDbdata();
                const Array<TwoD, const NekDouble>& metric = m_metricinfo->GetLaplacianMetrics();

                // Allocate temporary storage
                Array<OneD,NekDouble> wsp0(4*wspsize);
                Array<OneD,NekDouble> wsp1(wsp0+wspsize);
                Array<OneD,NekDouble> wsp2(wsp0+2*wspsize);
                Array<OneD,NekDouble> wsp3(wsp0+3*wspsize);

                // MASS MATRIX OPERATION
                // The following is being calculated:
                // wsp0     = B   * u_hat = u
                // wsp1     = W   * wsp0
                // outarray = B^T * wsp1  = B^T * W * B * u_hat = M * u_hat
                BwdTrans_SumFacKernel       (base0,base1,inarray,wsp0,    wsp1);
                MultiplyByQuadratureMetric  (wsp0,wsp2);
                IProductWRTBase_SumFacKernel(base0,base1,wsp2,   outarray,wsp1);

                // LAPLACIAN MATRIX OPERATION
                // wsp1 = du_dxi1 = D_xi1 * wsp0 = D_xi1 * u
                // wsp2 = du_dxi2 = D_xi2 * wsp0 = D_xi2 * u
                StdExpansion2D::PhysTensorDeriv(wsp0,wsp1,wsp2);

                // wsp0 = k = g0 * wsp1 + g1 * wsp2 = g0 * du_dxi1 + g1 * du_dxi2
                // wsp2 = l = g1 * wsp1 + g2 * wsp2 = g0 * du_dxi1 + g1 * du_dxi2
                // where g0, g1 and g2 are the metric terms set up in the GeomFactors class
                // especially for this purpose
                Vmath::Vvtvvtp(nqtot,&metric[0][0],1,&wsp1[0],1,&metric[1][0],1,&wsp2[0],1,&wsp0[0],1);
                Vmath::Vvtvvtp(nqtot,&metric[1][0],1,&wsp1[0],1,&metric[2][0],1,&wsp2[0],1,&wsp2[0],1);

                // wsp1 = m = (D_xi1 * B)^T * k
                // wsp0 = n = (D_xi2 * B)^T * l
                IProductWRTBase_SumFacKernel(dbase0,base1,wsp0,wsp1,wsp3);
                IProductWRTBase_SumFacKernel(base0,dbase1,wsp2,wsp0,wsp3);

                // outarray = lambda * outarray + (wsp0 + wsp1)
                //          = (lambda * M + L ) * u_hat
                Vmath::Vstvpp(m_ncoeffs,lambda,&outarray[0],1,&wsp1[0],1,&wsp0[0],1,&outarray[0],1);
            }
            else
            {
                int       i;
                int       dim = m_geom->GetCoordim();
                int       nquad0  = m_base[0]->GetNumPoints();
                int       nquad1  = m_base[1]->GetNumPoints();
                int       nqtot   = nquad0*nquad1;
                int       nmodes0 = m_base[0]->GetNumModes();
                int       nmodes1 = m_base[1]->GetNumModes();
                int       wspsize = max(max(max(nqtot,m_ncoeffs),nquad1*nmodes0),nquad0*nmodes1);
                NekDouble lambda  = mkey.GetConstant(0);

                const Array<OneD, const NekDouble>& base0  = m_base[0]->GetBdata();
                const Array<OneD, const NekDouble>& base1  = m_base[1]->GetBdata();
                const Array<OneD, const NekDouble>& dbase0 = m_base[0]->GetDbdata();
                const Array<OneD, const NekDouble>& dbase1 = m_base[1]->GetDbdata();

                // Allocate temporary storage
                Array<OneD,NekDouble> wsp0(9*wspsize);
                Array<OneD,NekDouble> wsp1(wsp0+wspsize);
                Array<OneD,NekDouble> wsp2(wsp0+2*wspsize);
                Array<OneD,NekDouble> wsp3(wsp0+3*wspsize);
                Array<OneD,NekDouble> wsp4(wsp0+4*wspsize);
                Array<OneD,NekDouble> wsp5(wsp0+5*wspsize);
                Array<OneD,NekDouble> wsp6(wsp0+6*wspsize);
                Array<OneD,NekDouble> wsp7(wsp0+7*wspsize);
                Array<OneD,NekDouble> wsp8(wsp0+8*wspsize);

                // MASS MATRIX OPERATION
                // The following is being calculated:
                // wsp0     = B   * u_hat = u
                // wsp1     = W   * wsp0
                // outarray = B^T * wsp1  = B^T * W * B * u_hat = M * u_hat
                BwdTrans_SumFacKernel       (base0,base1,inarray,wsp0,    wsp1);
                MultiplyByQuadratureMetric  (wsp0,wsp2);
                IProductWRTBase_SumFacKernel(base0,base1,wsp2,   outarray,wsp1);

                // LAPLACIAN MATRIX OPERATION
                // wsp1 = du_dxi1 = D_xi1 * wsp0 = D_xi1 * u
                // wsp2 = du_dxi2 = D_xi2 * wsp0 = D_xi2 * u
                StdExpansion2D::PhysTensorDeriv(wsp0,wsp1,wsp2);

                // wsp0 = k = g0 * wsp1 + g1 * wsp2 = g0 * du_dxi1 + g1 * du_dxi2
                // wsp2 = l = g1 * wsp1 + g2 * wsp2 = g0 * du_dxi1 + g1 * du_dxi2
                const Array<TwoD, const NekDouble>& gmat = m_metricinfo->GetGmat();
                const Array<OneD, const NekDouble>& z0 = m_base[0]->GetZ();
                const Array<OneD, const NekDouble>& z1 = m_base[1]->GetZ();
                // substep 1: calculate the metric terms of the collapsed coordinate
                // transformation
                for(i = 0; i < nquad1; i++)
                {
                    Vmath::Fill(nquad0,2.0/(1-z1[i]),&wsp6[0]+i*nquad0,1);
                    Vmath::Fill(nquad0,2.0/(1-z1[i]),&wsp7[0]+i*nquad0,1);
                }
                for(i = 0; i < nquad0; i++)
                {
                    Blas::Dscal(nquad1,0.5*(1+z0[i]),&wsp7[0]+i,nquad0);
                }
                // substep2: calculate g0,g1,g2
                // g0 = wsp3
                // g1 = wsp4
                // g1 = wsp5
                if(m_metricinfo->GetGtype() == SpatialDomains::eDeformed)
                {
                    Vmath::Vmul (nqtot,&gmat[0][0],1,&wsp6[0],1,&wsp8[0],1);
                    Vmath::Vvtvp(nqtot,&gmat[1][0],1,&wsp7[0],1,&wsp8[0],1,&wsp8[0],1);

                    Vmath::Vmul (nqtot,&wsp8[0],   1,&wsp8[0],   1,&wsp3[0],1);
                    Vmath::Vmul (nqtot,&gmat[1][0],1,&wsp8[0],   1,&wsp4[0],1);
                    Vmath::Vmul (nqtot,&gmat[1][0],1,&gmat[1][0],1,&wsp5[0],1);


                    Vmath::Vmul (nqtot,&gmat[2][0],1,&wsp6[0],1,&wsp8[0],1);
                    Vmath::Vvtvp(nqtot,&gmat[3][0],1,&wsp7[0],1,&wsp8[0],1,&wsp8[0],1);

                    Vmath::Vvtvp(nqtot,&wsp8[0],   1,&wsp8[0],   1,&wsp3[0],1,&wsp3[0],1);
                    Vmath::Vvtvp(nqtot,&gmat[3][0],1,&wsp8[0],   1,&wsp4[0],1,&wsp4[0],1);
                    Vmath::Vvtvp(nqtot,&gmat[3][0],1,&gmat[3][0],1,&wsp5[0],1,&wsp5[0],1);

                    if(dim == 3)
                    {
                        Vmath::Vmul (nqtot,&gmat[4][0],1,&wsp6[0],1,&wsp8[0],1);
                        Vmath::Vvtvp(nqtot,&gmat[5][0],1,&wsp7[0],1,&wsp8[0],1,&wsp8[0],1);

                        Vmath::Vvtvp(nqtot,&wsp8[0],   1,&wsp8[0],   1,&wsp3[0],1,&wsp3[0],1);
                        Vmath::Vvtvp(nqtot,&gmat[5][0],1,&wsp8[0],   1,&wsp4[0],1,&wsp4[0],1);
                        Vmath::Vvtvp(nqtot,&gmat[5][0],1,&gmat[5][0],1,&wsp5[0],1,&wsp5[0],1);
                    }
                }
                else
                {
                    Vmath::Smul (nqtot,gmat[0][0],&wsp6[0],1,&wsp8[0],1);
                    Vmath::Svtvp(nqtot,gmat[1][0],&wsp7[0],1,&wsp8[0],1,&wsp8[0],1);

                    Vmath::Vmul (nqtot,&wsp8[0],1,&wsp8[0],1,&wsp3[0],1);
                    Vmath::Smul (nqtot,gmat[1][0],&wsp8[0],1,&wsp4[0],1);


                    Vmath::Smul (nqtot,gmat[2][0],&wsp6[0],1,&wsp8[0],1);
                    Vmath::Svtvp(nqtot,gmat[3][0],&wsp7[0],1,&wsp8[0],1,&wsp8[0],1);

                    Vmath::Vvtvp(nqtot,&wsp8[0],1,&wsp8[0],1,&wsp3[0],1,&wsp3[0],1);
                    Vmath::Svtvp(nqtot,gmat[3][0],&wsp8[0],1,&wsp4[0],1,&wsp4[0],1);

                    if(dim == 3)
                    {
                        Vmath::Smul (nqtot,gmat[4][0],&wsp6[0],1,&wsp8[0],1);
                        Vmath::Svtvp(nqtot,gmat[5][0],&wsp7[0],1,&wsp8[0],1,&wsp8[0],1);

                        Vmath::Vvtvp(nqtot,&wsp8[0],1,&wsp8[0],1,&wsp3[0],1,&wsp3[0],1);
                        Vmath::Svtvp(nqtot,gmat[5][0],&wsp8[0],1,&wsp4[0],1,&wsp4[0],1);
                    }

                    NekDouble g2 = gmat[1][0]*gmat[1][0] + gmat[3][0]*gmat[3][0];
                    if(dim == 3)
                    {
                        g2 += gmat[5][0]*gmat[5][0];
                    }
                    Vmath::Fill(nqtot,g2,&wsp5[0],1);
                }

                // substep 3:
                // wsp0 = k = wsp3 * wsp1 + wsp4 * wsp2 = g0 * du_dxi1 + g1 * du_dxi2
                // wsp2 = l = wsp4 * wsp1 + wsp5 * wsp2 = g0 * du_dxi1 + g1 * du_dxi2
                Vmath::Vvtvvtp(nqtot,&wsp3[0],1,&wsp1[0],1,&wsp4[0],1,&wsp2[0],1,&wsp0[0],1);
                Vmath::Vvtvvtp(nqtot,&wsp4[0],1,&wsp1[0],1,&wsp5[0],1,&wsp2[0],1,&wsp2[0],1);
                // substep 4: multiply by jacobian and quadrature weights
                MultiplyByQuadratureMetric(wsp0,wsp0);
                MultiplyByQuadratureMetric(wsp2,wsp2);

                // wsp1 = m = (D_xi1 * B)^T * k
                // wsp0 = n = (D_xi2 * B)^T * l
                IProductWRTBase_SumFacKernel(dbase0,base1,wsp0,wsp1,wsp3);
                IProductWRTBase_SumFacKernel(base0,dbase1,wsp2,wsp0,wsp3);

                // outarray = lambda * outarray + (wsp0 + wsp1)
                //          = (lambda * M + L ) * u_hat
                Vmath::Vstvpp(m_ncoeffs,lambda,&outarray[0],1,&wsp1[0],1,&wsp0[0],1,&outarray[0],1);
            }
        }

        void TriExp::IProductWRTBase_SumFac(const Array<OneD, const NekDouble>& inarray,
                                             Array<OneD, NekDouble> &outarray)
        {
            int    nquad0 = m_base[0]->GetNumPoints();
            int    nquad1 = m_base[1]->GetNumPoints();
            int    order0 = m_base[0]->GetNumModes();

            Array<OneD,NekDouble> tmp(nquad0*nquad1+nquad1*order0);
            Array<OneD,NekDouble> wsp(tmp+nquad0*nquad1);

            MultiplyByQuadratureMetric(inarray,tmp);
            StdTriExp::IProductWRTBase_SumFacKernel(m_base[0]->GetBdata(),m_base[1]->GetBdata(),tmp,outarray,wsp);
        }

        void TriExp::IProductWRTBase_MatOp(const Array<OneD, const NekDouble>& inarray,
                                           Array<OneD, NekDouble> &outarray)
        {
            int nq = GetTotPoints();
            MatrixKey      iprodmatkey(StdRegions::eIProductWRTBase,DetExpansionType(),*this);
            DNekScalMatSharedPtr& iprodmat = m_matrixManager[iprodmatkey];

            Blas::Dgemv('N',m_ncoeffs,nq,iprodmat->Scale(),(iprodmat->GetOwnedMatrix())->GetPtr().get(),
                        m_ncoeffs, inarray.get(), 1, 0.0, outarray.get(), 1);

        }

        void TriExp::IProductWRTDerivBase_SumFac(const int dir,
                                                 const Array<OneD, const NekDouble>& inarray,
                                                 Array<OneD, NekDouble> & outarray)
        {
            ASSERTL1((dir==0)||(dir==1)||(dir==2),"Invalid direction.");
            ASSERTL1((dir==2)?(m_geom->GetCoordim()==3):true,"Invalid direction.");

            int    i;
            int    nquad0 = m_base[0]->GetNumPoints();
            int    nquad1 = m_base[1]->GetNumPoints();
            int    nqtot  = nquad0*nquad1;
            int    nmodes0 = m_base[0]->GetNumModes();
            int    wspsize = max(max(nqtot,m_ncoeffs),nquad1*nmodes0);

            const Array<TwoD, const NekDouble>& gmat = m_metricinfo->GetGmat();

            Array<OneD, NekDouble> tmp0 (6*wspsize);
            Array<OneD, NekDouble> tmp1 (tmp0 +   wspsize);
            Array<OneD, NekDouble> tmp2 (tmp0 + 2*wspsize);
            Array<OneD, NekDouble> tmp3 (tmp0 + 3*wspsize);
            Array<OneD, NekDouble> gfac0(tmp0 + 4*wspsize);
            Array<OneD, NekDouble> gfac1(tmp0 + 5*wspsize);

            const Array<OneD, const NekDouble>& z0 = m_base[0]->GetZ();
            const Array<OneD, const NekDouble>& z1 = m_base[1]->GetZ();

            // set up geometric factor: 2/(1-z1)
            for(i = 0; i < nquad1; ++i)
            {
                gfac0[i] = 2.0/(1-z1[i]);
            }
            for(i = 0; i < nquad0; ++i)
            {
                gfac1[i] = 0.5*(1+z0[i]);
            }

            for(i = 0; i < nquad1; ++i)
            {
                Vmath::Smul(nquad0,gfac0[i],&inarray[0]+i*nquad0,1,&tmp0[0]+i*nquad0,1);
            }

            for(i = 0; i < nquad1; ++i)
            {
                Vmath::Vmul(nquad0,&gfac1[0],1,&tmp0[0]+i*nquad0,1,&tmp1[0]+i*nquad0,1);
            }

            if(m_metricinfo->GetGtype() == SpatialDomains::eDeformed)
            {
                Vmath::Vmul(nqtot,&gmat[2*dir][0],  1,&tmp0[0],   1,&tmp0[0],1);
                Vmath::Vmul(nqtot,&gmat[2*dir+1][0],1,&tmp1[0],   1,&tmp1[0],1);
                Vmath::Vmul(nqtot,&gmat[2*dir+1][0],1,&inarray[0],1,&tmp2[0],1);
            }
            else
            {
                Vmath::Smul(nqtot, gmat[2*dir][0],   tmp0,    1, tmp0, 1);
                Vmath::Smul(nqtot, gmat[2*dir+1][0], tmp1,    1, tmp1, 1);
                Vmath::Smul(nqtot, gmat[2*dir+1][0], inarray, 1, tmp2, 1);
            }
            Vmath::Vadd(nqtot, tmp0, 1, tmp1, 1, tmp1, 1);

            MultiplyByQuadratureMetric(tmp1,tmp1);
            MultiplyByQuadratureMetric(tmp2,tmp2);

            IProductWRTBase_SumFacKernel(m_base[0]->GetDbdata(),m_base[1]->GetBdata() ,tmp1,tmp3    ,tmp0);
            IProductWRTBase_SumFacKernel(m_base[0]->GetBdata() ,m_base[1]->GetDbdata(),tmp2,outarray,tmp0);
            Vmath::Vadd(m_ncoeffs, tmp3, 1, outarray, 1, outarray, 1);
        }

        void TriExp::IProductWRTDerivBase_MatOp(const int dir,
                                                const Array<OneD, const NekDouble>& inarray,
                                                Array<OneD, NekDouble> &outarray)
        {
            int nq = GetTotPoints();
            StdRegions::MatrixType mtype;

            switch(dir)
            {
            case 0:
                {
                    mtype = StdRegions::eIProductWRTDerivBase0;
                }
                break;
            case 1:
                {
                    mtype = StdRegions::eIProductWRTDerivBase1;
                }
                break;
            case 2:
                {
                    mtype = StdRegions::eIProductWRTDerivBase2;
                }
                break;
            default:
                {
                    ASSERTL1(false,"input dir is out of range");
                }
                break;
            }

            MatrixKey      iprodmatkey(mtype,DetExpansionType(),*this);
            DNekScalMatSharedPtr& iprodmat = m_matrixManager[iprodmatkey];

            Blas::Dgemv('N',m_ncoeffs,nq,iprodmat->Scale(),(iprodmat->GetOwnedMatrix())->GetPtr().get(),
                        m_ncoeffs, inarray.get(), 1, 0.0, outarray.get(), 1);

        }

        ///////////////////////////////
        /// Differentiation Methods
        ///////////////////////////////

        /**
            \brief Calculate the deritive of the physical points
        **/
        void TriExp::PhysDeriv(const Array<OneD, const NekDouble> & inarray,
                               Array<OneD,NekDouble> &out_d0,
                               Array<OneD,NekDouble> &out_d1,
                               Array<OneD,NekDouble> &out_d2)
        {
            int    nquad0 = m_base[0]->GetNumPoints();
            int    nquad1 = m_base[1]->GetNumPoints();
            int     nqtot = nquad0*nquad1;
            const Array<TwoD, const NekDouble>& gmat = m_metricinfo->GetGmat();

            Array<OneD,NekDouble> diff0(2*nqtot);
            Array<OneD,NekDouble> diff1(diff0+nqtot);

            StdTriExp::PhysDeriv(inarray, diff0, diff1);

            if(m_metricinfo->GetGtype() == SpatialDomains::eDeformed)
            {
                if(out_d0.num_elements())
                {
                    Vmath::Vmul  (nqtot,&gmat[0][0],1,&diff0[0],1, &out_d0[0], 1);
                    Vmath::Vvtvp (nqtot,&gmat[1][0],1,&diff1[0],1, &out_d0[0], 1,
                                  &out_d0[0],1);
                }

                if(out_d1.num_elements())
                {
                    Vmath::Vmul  (nqtot,&gmat[2][0],1,&diff0[0],1, &out_d1[0], 1);
                    Vmath::Vvtvp (nqtot,&gmat[3][0],1,&diff1[0],1, &out_d1[0], 1,
                                  &out_d1[0],1);
                }

                if(out_d2.num_elements())
                {
                    Vmath::Vmul  (nqtot,&gmat[4][0],1,&diff0[0],1, &out_d2[0], 1);
                    Vmath::Vvtvp (nqtot,&gmat[5][0],1,&diff1[0],1, &out_d2[0], 1,
                                  &out_d2[0],1);
                }
            }
            else // regular geometry
            {
                if(out_d0.num_elements())
                {
                    Vmath::Smul (nqtot, gmat[0][0], diff0 , 1, out_d0, 1);
                    Blas::Daxpy (nqtot, gmat[1][0], diff1 , 1, out_d0, 1);
                }

                if(out_d1.num_elements())
                {
                    Vmath::Smul (nqtot, gmat[2][0], diff0, 1, out_d1, 1);
                    Blas::Daxpy (nqtot, gmat[3][0], diff1, 1, out_d1, 1);
                }

                if(out_d2.num_elements())
                {
                    Vmath::Smul (nqtot, gmat[4][0], diff0, 1, out_d2, 1);
                    Blas::Daxpy (nqtot, gmat[5][0], diff1, 1, out_d2, 1);
                }
            }
        }

        void TriExp::PhysDeriv(const int dir,
                               const Array<OneD, const NekDouble>& inarray,
                               Array<OneD, NekDouble> &outarray)
        {
            switch(dir)
            {
            case 0:
                {
                    PhysDeriv(inarray, outarray, NullNekDouble1DArray, NullNekDouble1DArray);
                }
                break;
            case 1:
                {
                    PhysDeriv(inarray, NullNekDouble1DArray, outarray, NullNekDouble1DArray);
                }
                break;
            case 2:
                {
                    PhysDeriv(inarray, NullNekDouble1DArray, NullNekDouble1DArray, outarray);
                }
                break;
            default:
                {
                    ASSERTL1(false,"input dir is out of range");
                }
                break;
            }
        }

        // Physical Derivation along direction vector
        void TriExp::PhysDirectionalDeriv(const Array<OneD, const NekDouble> & inarray,
                                          const Array<OneD, const NekDouble>& direction,
                                          Array<OneD,NekDouble> &out)
        {
            int    nquad0 = m_base[0]->GetNumPoints();
            int    nquad1 = m_base[1]->GetNumPoints();
            int    nqtot = nquad0*nquad1;

            const Array<TwoD, const NekDouble>& gmat = m_metricinfo->GetGmat();

            Array<OneD,NekDouble> diff0(2*nqtot);
            Array<OneD,NekDouble> diff1(diff0+nqtot);

            // diff0 = du/d_xi, diff1 = du/d_eta
            StdTriExp::PhysDeriv(inarray, diff0, diff1);

            if(m_metricinfo->GetGtype() == SpatialDomains::eDeformed)
            {
                Array<OneD, Array<OneD, NekDouble> > tangmat(2);

                // D^v_xi = v_x*d_xi/dx + v_y*d_xi/dy + v_z*d_xi/dz
                // D^v_eta = v_x*d_eta/dx + v_y*d_eta/dy + v_z*d_eta/dz
                for (int i=0; i< 2; ++i)
                {
                    tangmat[i] = Array<OneD, NekDouble>(nqtot,0.0);
                    for (int k=0; k<(m_geom->GetCoordim()); ++k)
                    {
                        Vmath::Vvtvp(nqtot,&gmat[2*k+i][0],1,&direction[k*nqtot],1,&tangmat[i][0],1,&tangmat[i][0],1);
                    }
                }

                /// D_v = D^v_xi * du/d_xi + D^v_eta * du/d_eta
                if(out.num_elements())
                {
                    Vmath::Vmul  (nqtot,&tangmat[0][0],1,&diff0[0],1, &out[0], 1);
                    Vmath::Vvtvp (nqtot,&tangmat[1][0],1,&diff1[0],1, &out[0], 1, &out[0],1);
                }

            }
            else
            {
                ASSERTL1(m_metricinfo->GetGtype() == SpatialDomains::eDeformed,"Wrong route");
            }
        }

        /** \brief Forward transform from physical quadrature space
            stored in \a inarray and evaluate the expansion coefficients and
            store in \a (this)->m_coeffs

            Inputs:\n

            - \a inarray: array of physical quadrature points to be transformed

            Outputs:\n

            - \a (this)->m_coeffs: updated array of expansion coefficients.

        */
        void TriExp::FwdTrans(const Array<OneD, const NekDouble> & inarray,
                              Array<OneD,NekDouble> &outarray)
        {
            IProductWRTBase(inarray,outarray);

            // get Mass matrix inverse
            MatrixKey             masskey(StdRegions::eInvMass,
                                          DetExpansionType(),*this);
            DNekScalMatSharedPtr& matsys = m_matrixManager[masskey];

            // copy inarray in case inarray == outarray
            NekVector<const NekDouble> in (m_ncoeffs,outarray,eCopy);
            NekVector<NekDouble> out(m_ncoeffs,outarray,eWrapper);

            out = (*matsys)*in;
        }

        void TriExp::FwdTrans_BndConstrained(const Array<OneD, const NekDouble>& inarray,
                                             Array<OneD, NekDouble> &outarray)
        {
            int i,j;
            int npoints[2] = {m_base[0]->GetNumPoints(),
                              m_base[1]->GetNumPoints()};
            int nmodes[2]  = {m_base[0]->GetNumModes(),
                              m_base[1]->GetNumModes()};

            fill(outarray.get(), outarray.get()+m_ncoeffs, 0.0 );

            Array<OneD, NekDouble> physEdge[3];
            Array<OneD, NekDouble> coeffEdge[3];
            StdRegions::EdgeOrientation orient[3];
            for(i = 0; i < 3; i++)
            {
                physEdge[i]  = Array<OneD, NekDouble>(npoints[i!=0]);
                coeffEdge[i] = Array<OneD, NekDouble>(nmodes[i!=0]);
                orient[i]    = GetEorient(i);
            }

            for(i = 0; i < npoints[0]; i++)
            {
                physEdge[0][i] = inarray[i];
            }

            for(i = 0; i < npoints[1]; i++)
            {
                physEdge[1][i] = inarray[npoints[0]-1+i*npoints[0]];
                physEdge[2][i] = inarray[(npoints[1]-1)*npoints[0]-i*npoints[0]];
            }

            for(i = 0; i < 3; i++)
            {
                if( orient[i] == StdRegions::eBackwards )
                {
                    reverse( (physEdge[i]).get() , (physEdge[i]).get() + npoints[i!=0] );
                }
            }

            SegExpSharedPtr segexp[3];
            for(i = 0; i < 3; i++)
            {
                segexp[i] = MemoryManager<LocalRegions::SegExp>::AllocateSharedPtr(m_base[i!=0]->GetBasisKey(),GetGeom2D()->GetEdge(i));
            }

            Array<OneD, unsigned int> mapArray;
            Array<OneD, int>          signArray;
            NekDouble sign;

            for(i = 0; i < 3; i++)
            {
                segexp[i!=0]->FwdTrans_BndConstrained(physEdge[i],coeffEdge[i]);

                GetEdgeToElementMap(i,orient[i],mapArray,signArray);
                for(j=0; j < nmodes[i!=0]; j++)
                {
                    sign = (NekDouble) signArray[j];
                    outarray[ mapArray[j] ] = sign * coeffEdge[i][j];
                }
            }

            if (m_ncoeffs > 6) {
				Array<OneD, NekDouble> tmp0(m_ncoeffs);
				Array<OneD, NekDouble> tmp1(m_ncoeffs);

				StdRegions::StdMatrixKey  stdmasskey(StdRegions::eMass,DetExpansionType(),*this);
				MassMatrixOp(outarray,tmp0,stdmasskey);
				IProductWRTBase(inarray,tmp1);

				Vmath::Vsub(m_ncoeffs, tmp1, 1, tmp0, 1, tmp1, 1);

				// get Mass matrix inverse (only of interior DOF)
				// use block (1,1) of the static condensed system
				// note: this block alreay contains the inverse matrix
				MatrixKey             masskey(StdRegions::eMass,DetExpansionType(),*this);
				DNekScalMatSharedPtr  matsys = (m_staticCondMatrixManager[masskey])->GetBlock(1,1);

				int nBoundaryDofs = NumBndryCoeffs();
				int nInteriorDofs = m_ncoeffs - nBoundaryDofs;

				Array<OneD, NekDouble> rhs(nInteriorDofs);
				Array<OneD, NekDouble> result(nInteriorDofs);

				GetInteriorMap(mapArray);

				for(i = 0; i < nInteriorDofs; i++)
				{
					rhs[i] = tmp1[ mapArray[i] ];
				}

				Blas::Dgemv('N', nInteriorDofs, nInteriorDofs, matsys->Scale(), &((matsys->GetOwnedMatrix())->GetPtr())[0],
							nInteriorDofs,rhs.get(),1,0.0,result.get(),1);

				for(i = 0; i < nInteriorDofs; i++)
				{
					outarray[ mapArray[i] ] = result[i];
				}
            }
        }

        void TriExp::GetSurfaceNormal(Array<OneD,NekDouble> &SurfaceNormal, const int k)
        {
            int m_num = m_base[0]->GetNumPoints()*m_base[1]->GetNumPoints();

            Vmath::Vcopy(m_num, m_metricinfo->GetNormal()[k], 1, SurfaceNormal, 1);
      	}

        void TriExp::GetCoords(Array<OneD,NekDouble> &coords_0,
                               Array<OneD,NekDouble> &coords_1,
                               Array<OneD,NekDouble> &coords_2)
        {
            LibUtilities::BasisSharedPtr CBasis0;
            LibUtilities::BasisSharedPtr CBasis1;
            Array<OneD,NekDouble>  x;

            ASSERTL0(m_geom, "m_geom not define");

            // get physical points defined in Geom
            m_geom->FillGeom();

            switch(m_geom->GetCoordim())
            {
            case 3:
                ASSERTL0(coords_2.num_elements() != 0,"output coords_2 is not defined");

                CBasis0 = m_geom->GetBasis(2,0);
                CBasis1 = m_geom->GetBasis(2,1);

                if((m_base[0]->GetBasisKey().SamePoints(CBasis0->GetBasisKey()))&&
                   (m_base[1]->GetBasisKey().SamePoints(CBasis1->GetBasisKey())))
                {
                    x = m_geom->UpdatePhys(2);
                    Blas::Dcopy(m_base[0]->GetNumPoints()*m_base[1]->GetNumPoints(),
                                x, 1, coords_2, 1);
                }
                else // Interpolate to Expansion point distribution
                {
                    LibUtilities::Interp2D(CBasis0->GetPointsKey(), CBasis1->GetPointsKey(),&(m_geom->UpdatePhys(2))[0],
                                           m_base[0]->GetPointsKey(),m_base[1]->GetPointsKey(),&coords_2[0]);
                }
            case 2:
                ASSERTL0(coords_1.num_elements(),
                         "output coords_1 is not defined");

                CBasis0 = m_geom->GetBasis(1,0);
                CBasis1 = m_geom->GetBasis(1,1);

                if((m_base[0]->GetBasisKey().SamePoints(CBasis0->GetBasisKey()))&&
                   (m_base[1]->GetBasisKey().SamePoints(CBasis1->GetBasisKey())))
                {
                    x = m_geom->UpdatePhys(1);
                    Blas::Dcopy(m_base[0]->GetNumPoints()*m_base[1]->GetNumPoints(),
                                x, 1, coords_1, 1);
                }
                else // LibUtilities::Interpolate to Expansion point distribution
                {
                    LibUtilities::Interp2D(CBasis0->GetPointsKey(), CBasis1->GetPointsKey(), &(m_geom->UpdatePhys(1))[0],
                             m_base[0]->GetPointsKey(),m_base[1]->GetPointsKey(),&coords_1[0]);
                }
            case 1:
                ASSERTL0(coords_0.num_elements(),
                         "output coords_0 is not defined");

                CBasis0 = m_geom->GetBasis(0,0);
                CBasis1 = m_geom->GetBasis(0,1);

                if((m_base[0]->GetBasisKey().SamePoints(CBasis0->GetBasisKey()))&&
                   (m_base[1]->GetBasisKey().SamePoints(CBasis1->GetBasisKey())))
                {
                    x = m_geom->UpdatePhys(0);
                    Blas::Dcopy(m_base[0]->GetNumPoints()*m_base[1]->GetNumPoints(),
                                x, 1, coords_0, 1);
                }
                else // Interpolate to Expansion point distribution
                {
                    LibUtilities::Interp2D(CBasis0->GetPointsKey(), CBasis1->GetPointsKey(), &(m_geom->UpdatePhys(0))[0],
                             m_base[0]->GetPointsKey(),m_base[1]->GetPointsKey(),&coords_0[0]);
                }
                break;
            default:
                ASSERTL0(false,"Number of dimensions are greater than 2");
                break;
            }
        }


        // get the coordinates "coords" at the local coordinates "Lcoords"
        void TriExp::GetCoord(const Array<OneD, const NekDouble> &Lcoords,
                              Array<OneD,NekDouble> &coords)
        {
            int  i;

            ASSERTL1(Lcoords[0] >= -1.0 && Lcoords[1] <= 1.0 &&
                     Lcoords[1] >= -1.0 && Lcoords[1]  <=1.0,
                     "Local coordinates are not in region [-1,1]");

            m_geom->FillGeom();

            for(i = 0; i < m_geom->GetCoordDim(); ++i)
            {
                coords[i] = m_geom->GetCoord(i,Lcoords);
            }
        }


        void TriExp::WriteToFile(std::ofstream &outfile, OutputFormat format, const bool dumpVar, std::string var)
        {
            if(format==eTecplot)
            {
                int i,j;
                int nquad0 = m_base[0]->GetNumPoints();
                int nquad1 = m_base[1]->GetNumPoints();
                Array<OneD,NekDouble> coords[3];

                ASSERTL0(m_geom,"m_geom not defined");

                int     coordim  = m_geom->GetCoordim();

                coords[0] = Array<OneD,NekDouble>(nquad0*nquad1);
                coords[1] = Array<OneD,NekDouble>(nquad0*nquad1);
                coords[2] = Array<OneD,NekDouble>(nquad0*nquad1);

                GetCoords(coords[0],coords[1],coords[2]);

                if(dumpVar)
                {
                    outfile << "Variables = x";

                    if(coordim == 2)
                    {
                        outfile << ", y";
                    }
                    else if (coordim == 3)
                    {
                        outfile << ", y, z";
                    }
                    outfile << ", "<< var << std::endl << std::endl;
                }

                outfile << "Zone, I=" << nquad0 << ", J=" <<
                    nquad1 <<", F=Point" << std::endl;

                for(i = 0; i < nquad0*nquad1; ++i)
                {
                    for(j = 0; j < coordim; ++j)
                    {
                        outfile << coords[j][i] << " ";
                    }
                    outfile << m_phys[i] << std::endl;
                }
            }
            else if(format==eGmsh)
            {
                if(dumpVar)
                {
                    outfile<<"View.MaxRecursionLevel = 8;"<<endl;
                    outfile<<"View.TargetError = 0.00;"<<endl;
                    outfile<<"View \" \" {"<<endl;
                }

                outfile<<"ST("<<endl;
                // write the coordinates of the vertices of the triangle
                Array<OneD,NekDouble> coordVert1(2);
                Array<OneD,NekDouble> coordVert2(2);
                Array<OneD,NekDouble> coordVert3(2);
                coordVert1[0]=-1.0;
                coordVert1[1]=-1.0;
                coordVert2[0]=1.0;
                coordVert2[1]=-1.0;
                coordVert3[0]=-1.0;
                coordVert3[1]=1.0;
                outfile<<m_geom->GetCoord(0,coordVert1)<<", ";
                outfile<<m_geom->GetCoord(1,coordVert1)<<", 0.0,"<<endl;
                outfile<<m_geom->GetCoord(0,coordVert2)<<", ";
                outfile<<m_geom->GetCoord(1,coordVert2)<<", 0.0,"<<endl;
                outfile<<m_geom->GetCoord(0,coordVert3)<<", ";
                outfile<<m_geom->GetCoord(1,coordVert3)<<", 0.0"<<endl;
                outfile<<")"<<endl;

                // calculate the coefficients (monomial format)
                int i,j;
                int maxnummodes = max(m_base[0]->GetNumModes(),m_base[1]->GetNumModes());

                const LibUtilities::PointsKey Pkey1Gmsh(maxnummodes,LibUtilities::eGaussGaussLegendre);
                const LibUtilities::PointsKey Pkey2Gmsh(maxnummodes,LibUtilities::eGaussGaussLegendre);
                const LibUtilities::BasisKey  Bkey1Gmsh(m_base[0]->GetBasisType(),maxnummodes,Pkey1Gmsh);
                const LibUtilities::BasisKey  Bkey2Gmsh(m_base[1]->GetBasisType(),maxnummodes,Pkey2Gmsh);
                LibUtilities::PointsType ptype = LibUtilities::eNodalTriElec;

                StdRegions::StdNodalTriExpSharedPtr EGmsh;
                EGmsh = MemoryManager<StdRegions::StdNodalTriExp>::
                    AllocateSharedPtr(Bkey1Gmsh,Bkey2Gmsh,ptype);

                Array<OneD,NekDouble> xi1(EGmsh->GetNcoeffs());
                Array<OneD,NekDouble> xi2(EGmsh->GetNcoeffs());
                EGmsh->GetNodalPoints(xi1,xi2);

                Array<OneD,NekDouble> x(EGmsh->GetNcoeffs());
                Array<OneD,NekDouble> y(EGmsh->GetNcoeffs());

                for(i=0;i<EGmsh->GetNcoeffs();i++)
                {
                    x[i] = 0.5*(1.0+xi1[i]);
                    y[i] = 0.5*(1.0+xi2[i]);
                }

                int cnt  = 0;
                int cnt2 = 0;
                int nDumpCoeffs = maxnummodes*maxnummodes;
                Array<TwoD, int> dumpExponentMap(nDumpCoeffs,3,0);
                Array<OneD, int> indexMap(EGmsh->GetNcoeffs(),0);
                Array<TwoD, int> exponentMap(EGmsh->GetNcoeffs(),3,0);
                for(i = 0; i < maxnummodes; i++)
                {
                    for(j = 0; j < maxnummodes; j++)
                    {
                        if(j<maxnummodes-i)
                        {
                            exponentMap[cnt][0] = j;
                            exponentMap[cnt][1] = i;
                            indexMap[cnt++]  = cnt2;
                        }

                        dumpExponentMap[cnt2][0]   = j;
                        dumpExponentMap[cnt2++][1] = i;
                    }
                }

                NekMatrix<NekDouble> vdm(EGmsh->GetNcoeffs(),EGmsh->GetNcoeffs());
                for(i = 0 ; i < EGmsh->GetNcoeffs(); i++)
                {
                    for(j = 0 ; j < EGmsh->GetNcoeffs(); j++)
                    {
                        vdm(i,j) = pow(x[i],exponentMap[j][0])*pow(y[i],exponentMap[j][1]);
                    }
                }

                vdm.Invert();

                Array<OneD, NekDouble> tmp2(EGmsh->GetNcoeffs());
                EGmsh->ModalToNodal(m_coeffs,tmp2);

                NekVector<const NekDouble> in(EGmsh->GetNcoeffs(),tmp2,eWrapper);
                NekVector<NekDouble> out(EGmsh->GetNcoeffs());
                out = vdm*in;

                Array<OneD,NekDouble> dumpOut(nDumpCoeffs,0.0);
                for(i = 0 ; i < EGmsh->GetNcoeffs(); i++)
                {
                    dumpOut[ indexMap[i]  ] = out[i];
                }

                //write the coefficients
                outfile<<"{";
                for(i = 0; i < nDumpCoeffs; i++)
                {
                    outfile<<dumpOut[i];
                    if(i < nDumpCoeffs - 1)
                    {
                        outfile<<", ";
                    }
                }
                outfile<<"};"<<endl;

                if(dumpVar)
                {
                    outfile<<"INTERPOLATION_SCHEME"<<endl;
                    outfile<<"{"<<endl;
                    for(i=0; i < nDumpCoeffs; i++)
                    {
                        outfile<<"{";
                        for(j = 0; j < nDumpCoeffs; j++)
                        {
                            if(i==j)
                            {
                                outfile<<"1.00";
                            }
                            else
                            {
                                outfile<<"0.00";
                            }
                            if(j < nDumpCoeffs - 1)
                            {
                                outfile<<", ";
                            }
                        }
                        if(i < nDumpCoeffs - 1)
                        {
                            outfile<<"},"<<endl;
                        }
                        else
                        {
                            outfile<<"}"<<endl<<"}"<<endl;
                        }
                    }

                    outfile<<"{"<<endl;
                    for(i=0; i < nDumpCoeffs; i++)
                    {
                        outfile<<"{";
                        for(j = 0; j < 3; j++)
                        {
                            outfile<<dumpExponentMap[i][j];
                            if(j < 2)
                            {
                                outfile<<", ";
                            }
                        }
                        if(i < nDumpCoeffs  - 1)
                        {
                            outfile<<"},"<<endl;
                        }
                        else
                        {
                            outfile<<"}"<<endl<<"};"<<endl;
                        }
                    }
                    outfile<<"};"<<endl;
                }
            }
            else
            {
                ASSERTL0(false, "Output routine not implemented for requested type of output");
            }
        }

        DNekMatSharedPtr TriExp::GenMatrix(const StdRegions::StdMatrixKey &mkey)
        {
            DNekMatSharedPtr returnval;

            switch(mkey.GetMatrixType())
            {
            case StdRegions::eHybridDGHelmholtz:
            case StdRegions::eHybridDGLamToU:
            case StdRegions::eHybridDGLamToQ0:
            case StdRegions::eHybridDGLamToQ1:
            case StdRegions::eHybridDGLamToQ2:
            case StdRegions::eHybridDGHelmBndLam:
                returnval = Expansion2D::GenMatrix(mkey);
                break;
            default:
                returnval = StdTriExp::GenMatrix(mkey);
                break;
            }
            return returnval;
        }

        DNekMatSharedPtr TriExp::CreateStdMatrix(const StdRegions::StdMatrixKey &mkey)
        {
            LibUtilities::BasisKey bkey0 = m_base[0]->GetBasisKey();
            LibUtilities::BasisKey bkey1 = m_base[1]->GetBasisKey();
            StdRegions::StdTriExpSharedPtr tmp = MemoryManager<StdTriExp>::
                AllocateSharedPtr(bkey0,bkey1);

            return tmp->GetStdMatrix(mkey);
        }

        NekDouble TriExp::PhysEvaluate(const Array<OneD, const NekDouble> &coord)
        {
            Array<OneD,NekDouble> Lcoord = Array<OneD,NekDouble>(2);

            ASSERTL0(m_geom,"m_geom not defined");
            m_geom->GetLocCoords(coord,Lcoord);

            return StdTriExp::PhysEvaluate(Lcoord);
        }

        DNekScalMatSharedPtr TriExp::CreateMatrix(const MatrixKey &mkey)
        {
            DNekScalMatSharedPtr returnval;

            switch(mkey.GetMatrixType())
            {
            case StdRegions::eMass:
                {
                    if(m_metricinfo->GetGtype() == SpatialDomains::eDeformed)
                    {
                        NekDouble one = 1.0;
                        DNekMatSharedPtr mat = GenMatrix(*mkey.GetStdMatKey());
                        returnval = MemoryManager<DNekScalMat>::AllocateSharedPtr(one,mat);
                    }
                    else
                    {
                        NekDouble jac = (m_metricinfo->GetJac())[0];
                        DNekMatSharedPtr mat = GetStdMatrix(*mkey.GetStdMatKey());
                        returnval = MemoryManager<DNekScalMat>::AllocateSharedPtr(jac,mat);
                    }
                }
                break;
            case StdRegions::eInvMass:
                {
                    if(m_metricinfo->GetGtype() == SpatialDomains::eDeformed)
                    {
                        NekDouble one = 1.0;
                        StdRegions::StdMatrixKey masskey(StdRegions::eMass,DetExpansionType(),
                                                         *this);
                        DNekMatSharedPtr mat = GenMatrix(masskey);
                        mat->Invert();

                        returnval = MemoryManager<DNekScalMat>::AllocateSharedPtr(one,mat);
                    }
                    else
                    {
                        NekDouble fac = 1.0/(m_metricinfo->GetJac())[0];
                        DNekMatSharedPtr mat = GetStdMatrix(*mkey.GetStdMatKey());
                        returnval = MemoryManager<DNekScalMat>::AllocateSharedPtr(fac,mat);

                    }
                }
                break;
            case StdRegions::eWeakDeriv0:
            case StdRegions::eWeakDeriv1:
            case StdRegions::eWeakDeriv2:
                {
                    if(m_metricinfo->GetGtype() == SpatialDomains::eDeformed)
                    {
                        NekDouble one = 1.0;
                        DNekMatSharedPtr mat = GenMatrix(*mkey.GetStdMatKey());

                        returnval = MemoryManager<DNekScalMat>::AllocateSharedPtr(one,mat);
                    }
                    else
                    {
                        NekDouble jac = (m_metricinfo->GetJac())[0];
                        Array<TwoD, const NekDouble> gmat = m_metricinfo->GetGmat();
                        int dir;

                        switch(mkey.GetMatrixType())
                        {
                        case StdRegions::eWeakDeriv0:
                            dir = 0;
                            break;
                        case StdRegions::eWeakDeriv1:
                            dir = 1;
                            break;
                        case StdRegions::eWeakDeriv2:
                            dir = 2;
                            break;
                        }

                        MatrixKey deriv0key(StdRegions::eWeakDeriv0,
                                            mkey.GetExpansionType(), *this);
                        MatrixKey deriv1key(StdRegions::eWeakDeriv1,
                                            mkey.GetExpansionType(), *this);

                        DNekMat &deriv0 = *GetStdMatrix(*deriv0key.GetStdMatKey());
                        DNekMat &deriv1 = *GetStdMatrix(*deriv1key.GetStdMatKey());

                        int rows = deriv0.GetRows();
                        int cols = deriv1.GetColumns();

                        DNekMatSharedPtr WeakDeriv = MemoryManager<DNekMat>::AllocateSharedPtr(rows,cols);
                        (*WeakDeriv) = gmat[2*dir][0]*deriv0 + gmat[2*dir+1][0]*deriv1;

                        returnval = MemoryManager<DNekScalMat>::AllocateSharedPtr(jac,WeakDeriv);
                    }
                }
                break;
            case StdRegions::eWeakDirectionalDeriv:
                {
                    int matrixid = mkey.GetMatrixID();
                    int dim = m_geom->GetCoordim();
                    int nqtot   = (m_base[0]->GetNumPoints())*(m_base[1]->GetNumPoints());
                    int nvarcoeffs = mkey.GetNvariableCoefficients();

                    NekDouble jac = (m_metricinfo->GetJac())[0];
                    Array<TwoD, const NekDouble> gmat = m_metricinfo->GetGmat();

                    if(m_metricinfo->GetGtype() == SpatialDomains::eDeformed)
                    {
                        NekDouble one = 1.0;

                        DNekMatSharedPtr WeakDirectionalDeriv = GenMatrix(*mkey.GetStdMatKey());

                        Array<OneD, Array<OneD, NekDouble> > Weight(1+2*dim);

                        // Store tangential basis in Weighted[0-dim]
                        Weight[0] = mkey.GetVariableCoefficient(0);

                        // Store gmat info in Weight[dim+1]
                        for (int k=0; k < 2*dim; ++k)
                        {
                            Weight[k+1] = Array<OneD, NekDouble>(nqtot);
                            Vmath::Vcopy(nqtot, &gmat[k][0], 1, &Weight[k+1][0], 1);
                        }

                        StdRegions::StdMatrixKey  stdmasskey(StdRegions::eMassLevelCurvature,DetExpansionType(),*this,Weight,matrixid);
                        DNekMatSharedPtr MassLevelCurvaturemat = GenMatrix(stdmasskey);

                        (*WeakDirectionalDeriv) = (*WeakDirectionalDeriv) + (*MassLevelCurvaturemat);

                        returnval = MemoryManager<DNekScalMat>::AllocateSharedPtr(one,WeakDirectionalDeriv);
                    }
                    else
                    {
                        Array<OneD, Array<OneD, NekDouble> > Cxi(1);
                        Array<OneD, Array<OneD, NekDouble> > Ceta(1);

                        // Directional Forcing is applied
                        Cxi[0] = Array<OneD, NekDouble> (nqtot,0.0);
                        Ceta[0] = Array<OneD, NekDouble> (nqtot,0.0);

                        // Cxi = tan_{xi_x} * d \xi/dx + tan_{xi_y} * d \xi/dy + tan_{xi_z} * d \xi/dz
                        // Ceta = tan_{eta_x} * d \eta/dx + tan_{xi_y} * d \xi/dy + tan_{xi_z} * d \xi/dz
                        for (int k=0; k<dim; ++k)
                        {
                            Vmath::Svtvp(nqtot,gmat[2*k][0],&(mkey.GetVariableCoefficient(0))[k*nqtot],1,
                                         &Cxi[0][0],1,&Cxi[0][0],1);
                            Vmath::Svtvp(nqtot,gmat[2*k+1][0],&(mkey.GetVariableCoefficient(0))[k*nqtot],1,
                                         &Ceta[0][0],1,&Ceta[0][0],1);
                        }

                        // derivxi = Cxi * ( B * D_{\xi} *B^T )
                        // deriveta = Ceta * ( B * D_{\eta} * B^T )
                        MatrixKey derivxikey(StdRegions::eWeakDeriv0, mkey.GetExpansionType(), *this, Cxi, matrixid);
                        MatrixKey derivetakey(StdRegions::eWeakDeriv1, mkey.GetExpansionType(), *this, Ceta, matrixid+10000);

                        DNekMat &derivxi = *GetStdMatrix(*derivxikey.GetStdMatKey());
                        DNekMat &deriveta = *GetStdMatrix(*derivetakey.GetStdMatKey());

                        int rows = derivxi.GetRows();
                        int cols = deriveta.GetColumns();

                        DNekMatSharedPtr WeakDirectionalDeriv = MemoryManager<DNekMat>::AllocateSharedPtr(rows,cols);

                        // D_t = D_xi
                        (*WeakDirectionalDeriv) = derivxi + deriveta;

                        // Add Weighted Mass with (\grad \cdot u )
                        Array<OneD, Array<OneD, NekDouble> > Weight(1+2*dim);

                        // Store tangential basis in Weighted[0-dim]
                        Weight[0] = mkey.GetVariableCoefficient(0);

                        // Store gmat info in Weight[dim+1]
                        for (int k=0; k < 2*dim; ++k)
                        {
                            Weight[k+1] = Array<OneD, NekDouble>(gmat[k].num_elements());
                            Weight[k+1][0] = gmat[k][0];
                        }

                        StdRegions::StdMatrixKey  stdmasskey(StdRegions::eMassLevelCurvature,DetExpansionType(),*this,Weight,matrixid);
                        DNekMatSharedPtr MassLevelCurvaturemat = GetStdMatrix(stdmasskey);

                        (*WeakDirectionalDeriv) = (*WeakDirectionalDeriv) + (*MassLevelCurvaturemat);

                        returnval = MemoryManager<DNekScalMat>::AllocateSharedPtr(jac,WeakDirectionalDeriv);
                    }
                }
                break;
            case StdRegions::eLaplacian:
                {
                    if( (m_metricinfo->GetGtype() == SpatialDomains::eDeformed) ||
                        (mkey.GetNvariableLaplacianCoefficients() > 0) )
                    {
                        NekDouble one = 1.0;
                        DNekMatSharedPtr mat = GenMatrix(*mkey.GetStdMatKey());

                        returnval = MemoryManager<DNekScalMat>::AllocateSharedPtr(one,mat);
                    }
                    else
                    {
                        MatrixKey lap00key(StdRegions::eLaplacian00,
                                           mkey.GetExpansionType(), *this);
                        MatrixKey lap01key(StdRegions::eLaplacian01,
                                           mkey.GetExpansionType(), *this);
                        MatrixKey lap11key(StdRegions::eLaplacian11,
                                           mkey.GetExpansionType(), *this);

                        DNekMatSharedPtr& lap00 = GetStdMatrix(*lap00key.GetStdMatKey());
                        DNekMatSharedPtr& lap01 = GetStdMatrix(*lap01key.GetStdMatKey());
                        DNekMatSharedPtr& lap11 = GetStdMatrix(*lap11key.GetStdMatKey());

                        NekDouble jac = (m_metricinfo->GetJac())[0];
                        Array<TwoD, const NekDouble> gmat = m_metricinfo->GetGmat();

                        int rows = lap00->GetRows();
                        int cols = lap00->GetColumns();

                        DNekMatSharedPtr lap = MemoryManager<DNekMat>::AllocateSharedPtr(rows,cols);

                        // Additional terms if Tri embedded in 3D coordinate system
                        if (m_geom->GetCoordDim() == 3)
                        {
                            (*lap) = (gmat[0][0]*gmat[0][0]+gmat[2][0]*gmat[2][0]+gmat[4][0]*gmat[4][0])* (*lap00) +
                                (gmat[0][0]*gmat[1][0] + gmat[2][0]*gmat[3][0] + gmat[4][0]*gmat[5][0])*(*lap01 + Transpose(*lap01)) +
                                (gmat[1][0]*gmat[1][0] + gmat[3][0]*gmat[3][0] + gmat[5][0]*gmat[5][0])* (*lap11);
                        }
                        else {
                            (*lap) = (gmat[0][0]*gmat[0][0]+gmat[2][0]*gmat[2][0])* (*lap00) +
                                (gmat[0][0]*gmat[1][0] + gmat[2][0]*gmat[3][0])*(*lap01 + Transpose(*lap01)) +
                                (gmat[1][0]*gmat[1][0] + gmat[3][0]*gmat[3][0])* (*lap11);
                        }

                        returnval = MemoryManager<DNekScalMat>::AllocateSharedPtr(jac,lap);
                    }
                }
                break;
            case StdRegions::eHelmholtz:
                {
                    NekDouble factor = mkey.GetConstant(0);
                    MatrixKey masskey(StdRegions::eMass,
                                      mkey.GetExpansionType(), *this);
                    DNekScalMat &MassMat = *(this->m_matrixManager[masskey]);
                    MatrixKey lapkey(StdRegions::eLaplacian,
                                     mkey.GetExpansionType(), *this);
                    DNekScalMat &LapMat = *(this->m_matrixManager[lapkey]);

                    int rows = LapMat.GetRows();
                    int cols = LapMat.GetColumns();

                    DNekMatSharedPtr helm = MemoryManager<DNekMat>::AllocateSharedPtr(rows,cols);

                    NekDouble one = 1.0;
                    (*helm) = LapMat + factor*MassMat;

                    returnval = MemoryManager<DNekScalMat>::AllocateSharedPtr(one,helm);
                }
                break;
            case StdRegions::eIProductWRTBase:
                {
                    if(m_metricinfo->GetGtype() == SpatialDomains::eDeformed)
                    {
                        NekDouble one = 1.0;
                        DNekMatSharedPtr mat = GenMatrix(*mkey.GetStdMatKey());
                        returnval = MemoryManager<DNekScalMat>::AllocateSharedPtr(one,mat);
                    }
                    else
                    {
                        NekDouble jac = (m_metricinfo->GetJac())[0];
                        DNekMatSharedPtr mat = GetStdMatrix(*mkey.GetStdMatKey());
                        returnval = MemoryManager<DNekScalMat>::AllocateSharedPtr(jac,mat);
                    }
                }
                break;
            case StdRegions::eIProductWRTDerivBase0:
            case StdRegions::eIProductWRTDerivBase1:
            case StdRegions::eIProductWRTDerivBase2:
                {
                    if(m_metricinfo->GetGtype() == SpatialDomains::eDeformed)
                    {
                        NekDouble one = 1.0;
                        DNekMatSharedPtr mat = GenMatrix(*mkey.GetStdMatKey());
                        returnval = MemoryManager<DNekScalMat>::AllocateSharedPtr(one,mat);
                    }
                    else
                    {
                        NekDouble jac = (m_metricinfo->GetJac())[0];
                        const Array<TwoD, const NekDouble>& gmat = m_metricinfo->GetGmat();
                        int dir;

                        switch(mkey.GetMatrixType())
                        {
                        case StdRegions::eIProductWRTDerivBase0:
                            dir = 0;
                            break;
                        case StdRegions::eIProductWRTDerivBase1:
                            dir = 1;
                            break;
                        case StdRegions::eIProductWRTDerivBase2:
                            dir = 2;
                            break;
                        }

                        MatrixKey iProdDeriv0Key(StdRegions::eIProductWRTDerivBase0,
                                                 mkey.GetExpansionType(), *this);
                        MatrixKey iProdDeriv1Key(StdRegions::eIProductWRTDerivBase1,
                                                 mkey.GetExpansionType(), *this);

                        DNekMat &stdiprod0 = *GetStdMatrix(*iProdDeriv0Key.GetStdMatKey());
                        DNekMat &stdiprod1 = *GetStdMatrix(*iProdDeriv0Key.GetStdMatKey());

                        int rows = stdiprod0.GetRows();
                        int cols = stdiprod1.GetColumns();

                        DNekMatSharedPtr mat = MemoryManager<DNekMat>::AllocateSharedPtr(rows,cols);
                        (*mat) = gmat[2*dir][0]*stdiprod0 + gmat[2*dir+1][0]*stdiprod1;

                        returnval = MemoryManager<DNekScalMat>::AllocateSharedPtr(jac,mat);
                    }
                }
                break;

            case StdRegions::eInvHybridDGHelmholtz:
                {
                    NekDouble one = 1.0;

                    int nvarcoeffs = mkey.GetNvariableCoefficients();
                    Array<OneD, Array<OneD,NekDouble> > varcoeffs(nvarcoeffs);

                    if(nvarcoeffs>0)
                    {
                        for(int j=0; j<nvarcoeffs; j++)
                        {
                            varcoeffs[j] = mkey.GetVariableCoefficient(j);
                        }
                    }

                    StdRegions::StdMatrixKey hkey(StdRegions::eHybridDGHelmholtz,
                                                  DetExpansionType(),*this,
                                                  mkey.GetConstant(0),
                                                  mkey.GetConstant(1),
                                                  varcoeffs,
                                                  mkey.GetMatrixID());

                    DNekMatSharedPtr mat = GenMatrix(hkey);

                    mat->Invert();
                    returnval = MemoryManager<DNekScalMat>::AllocateSharedPtr(one,mat);
                }
                break;
            default:
                {
                    NekDouble        one = 1.0;
                    DNekMatSharedPtr mat = GenMatrix(*mkey.GetStdMatKey());

                    returnval = MemoryManager<DNekScalMat>::AllocateSharedPtr(one,mat);
                }
                break;
            }

            return returnval;
        }

        DNekScalBlkMatSharedPtr TriExp::CreateStaticCondMatrix(const MatrixKey &mkey)
        {
            DNekScalBlkMatSharedPtr returnval;

            ASSERTL2(m_metricinfo->GetGtype() == SpatialDomains::eNoGeomType,"Geometric information is not set up");

            // set up block matrix system
            int nbdry = NumBndryCoeffs();
            int nint = m_ncoeffs - nbdry;
            unsigned int exp_size[] = {nbdry,nint};
            int nblks = 2;
            returnval = MemoryManager<DNekScalBlkMat>::AllocateSharedPtr(nblks,nblks,exp_size,exp_size); //Really need a constructor which takes Arrays
            NekDouble factor = 1.0;

            switch(mkey.GetMatrixType())
            {
                case StdRegions::eLaplacian:
                case StdRegions::eHelmholtz: // special case since Helmholtz not defined in StdRegions

                    // use Deformed case for both regular and deformed geometries
                    factor = 1.0;
                    goto UseLocRegionsMatrix;
                    break;
                default:
                    if(m_metricinfo->GetGtype() == SpatialDomains::eDeformed)
                    {
                        factor = 1.0;
                        goto UseLocRegionsMatrix;
                    }
                    else
                    {
                        factor = 1.0;
                        goto UseLocRegionsMatrix;
                    }
                    break;

                UseStdRegionsMatrix:
                {
                    NekDouble            invfactor = 1.0/factor;
                    NekDouble            one = 1.0;
                    DNekBlkMatSharedPtr  mat = GetStdStaticCondMatrix(*(mkey.GetStdMatKey()));
                    DNekScalMatSharedPtr Atmp;
                    DNekMatSharedPtr     Asubmat;

                    returnval->SetBlock(0,0,Atmp = MemoryManager<DNekScalMat>::AllocateSharedPtr(factor,Asubmat = mat->GetBlock(0,0)));
                    returnval->SetBlock(0,1,Atmp = MemoryManager<DNekScalMat>::AllocateSharedPtr(one,Asubmat = mat->GetBlock(0,1)));
                    returnval->SetBlock(1,0,Atmp = MemoryManager<DNekScalMat>::AllocateSharedPtr(factor,Asubmat = mat->GetBlock(1,0)));
                    returnval->SetBlock(1,1,Atmp = MemoryManager<DNekScalMat>::AllocateSharedPtr(invfactor,Asubmat = mat->GetBlock(1,1)));
                }
                break;

                UseLocRegionsMatrix:
                {
                    int i,j;
                    NekDouble            invfactor = 1.0/factor;
                    NekDouble            one = 1.0;

                    DNekScalMat &mat = *GetLocMatrix(mkey);

                    DNekMatSharedPtr A = MemoryManager<DNekMat>::AllocateSharedPtr(nbdry,nbdry);
                    DNekMatSharedPtr B = MemoryManager<DNekMat>::AllocateSharedPtr(nbdry,nint);
                    DNekMatSharedPtr C = MemoryManager<DNekMat>::AllocateSharedPtr(nint,nbdry);
                    DNekMatSharedPtr D = MemoryManager<DNekMat>::AllocateSharedPtr(nint,nint);

                    Array<OneD,unsigned int> bmap(nbdry);
                    Array<OneD,unsigned int> imap(nint);
                    GetBoundaryMap(bmap);
                    GetInteriorMap(imap);

                    for(i = 0; i < nbdry; ++i)
                    {
                        for(j = 0; j < nbdry; ++j)
                        {
                            (*A)(i,j) = mat(bmap[i],bmap[j]);
                        }

                        for(j = 0; j < nint; ++j)
                        {
                            (*B)(i,j) = mat(bmap[i],imap[j]);
                        }
                    }

                    for(i = 0; i < nint; ++i)
                    {
                        for(j = 0; j < nbdry; ++j)
                        {
                            (*C)(i,j) = mat(imap[i],bmap[j]);
                        }

                        for(j = 0; j < nint; ++j)
                        {
                            (*D)(i,j) = mat(imap[i],imap[j]);
                        }
                    }

                    // Calculate static condensed system
                    if(nint)
                    {
                        D->Invert();
                        (*B) = (*B)*(*D);
                        (*A) = (*A) - (*B)*(*C);
                    }

                    DNekScalMatSharedPtr     Atmp;

                    returnval->SetBlock(0,0,Atmp = MemoryManager<DNekScalMat>::AllocateSharedPtr(factor,A));
                    returnval->SetBlock(0,1,Atmp = MemoryManager<DNekScalMat>::AllocateSharedPtr(one,B));
                    returnval->SetBlock(1,0,Atmp = MemoryManager<DNekScalMat>::AllocateSharedPtr(factor,C));
                    returnval->SetBlock(1,1,Atmp = MemoryManager<DNekScalMat>::AllocateSharedPtr(invfactor,D));

                }
            }

            return returnval;
        }

        StdRegions::StdExpansion1DSharedPtr TriExp::GetEdgeExp(int edge, bool SetUpNormals)
        {
            if (m_edgeExp.size() > 0)
            {
                return m_edgeExp[edge];
            }
ASSERTL0(false,"Cannot find trace space expansion for this edge.");

            SegExpSharedPtr returnval;
            SpatialDomains::Geometry1DSharedPtr edg = m_geom->GetEdge(edge);

            returnval = MemoryManager<SegExp>::AllocateSharedPtr(DetEdgeBasisKey(edge),edg);

            if (SetUpNormals)
            {
                returnval->GetMetricInfo()->ComputeNormals(m_geom, edge, returnval->GetBasis(0)->GetPointsKey());
            }
/*
            if(SetUpNormals)
            {
                int i;
                int coordim = GetCoordim();
                int npoints = returnval->GetNumPoints(0);
                StdRegions::EdgeOrientation edgedir = GetEorient(edge);

                Array<OneD,NekDouble> phys_normals = m_metricinfo->GenNormals2D(StdRegions::eTriangle,edge,returnval->GetBasis(0)->GetPointsKey());

                if(edgedir == StdRegions::eBackwards)
                {
                    if(m_metricinfo->GetGtype() == SpatialDomains::eDeformed)
                    {
                        for(i = 0; i < coordim; ++i)
                        {
                            Vmath::Reverse(npoints,&phys_normals[i*npoints],1,
                                           &phys_normals[i*npoints],1);
                        }
                    }

                    Vmath::Neg(coordim*npoints,phys_normals,1);
                }

                returnval->SetPhysNormals(phys_normals);
            }
*/
            return returnval;
        }

        void TriExp::GetEdgePhysVals(const int edge, const StdRegions::StdExpansion1DSharedPtr &EdgeExp,
                                     const Array<OneD, const NekDouble> &inarray,
                                     Array<OneD,NekDouble> &outarray)
        {
            int nquad0 = m_base[0]->GetNumPoints();
            int nquad1 = m_base[1]->GetNumPoints();

            Array<OneD,const NekDouble> e_tmp;
            Array<OneD,NekDouble>       outtmp(max(nquad0,nquad1));

            // get points in Cartesian orientation
            switch(edge)
            {
            case 0:
                Vmath::Vcopy(nquad0,inarray,1,outtmp,1);
                break;
            case 1:
                Vmath::Vcopy(nquad1,e_tmp = inarray+(nquad0-1),nquad0,outtmp,1);
                break;
            case 2:
                Vmath::Vcopy(nquad1,inarray,nquad0,outtmp,1);
                break;
            default:
                ASSERTL0(false,"edge value (< 3) is out of range");
                break;
            }

            // Interpolate if required
            LibUtilities::Interp1D(m_base[edge?1:0]->GetPointsKey(),outtmp,
                     EdgeExp->GetBasis(0)->GetPointsKey(),outarray);

            //Reverse data if necessary
            if(GetCartesianEorient(edge) == StdRegions::eBackwards)
            {
                Vmath::Reverse(EdgeExp->GetNumPoints(0),&outarray[0],1,
                               &outarray[0],1);
            }

        }

    }//end of namespace
}//end of namespace

/**
 *    $Log: TriExp.cpp,v $
 *    Revision 1.68  2010/01/10 16:53:44  cantwell
 *    Support for embedded regular Quad and Tri in 3D coord system.
 *    Cleaned up Helmholtz2D solver.
 *
 *    Revision 1.67  2009/12/17 23:43:25  bnelson
 *    Fixed windows compiler warnings.
 *
 *    Revision 1.66  2009/12/17 17:48:22  bnelson
 *    Fixed visual studio compiler warning.
 *
 *    Revision 1.65  2009/12/15 18:09:02  cantwell
 *    Split GeomFactors into 1D, 2D and 3D
 *    Added generation of tangential basis into GeomFactors
 *    Updated ADR2DManifold solver to use GeomFactors for tangents
 *    Added <GEOMINFO> XML session section support in MeshGraph
 *    Fixed const-correctness in VmathArray
 *    Cleaned up LocalRegions code to generate GeomFactors
 *    Removed GenSegExp
 *    Temporary fix to SubStructuredGraph
 *    Documentation for GlobalLinSys and GlobalMatrix classes
 *
 *    Revision 1.64  2009/11/13 16:18:34  sehunchun
 *    *** empty log message ***
 *
 *    Revision 1.63  2009/11/11 18:45:09  sehunchun
 *    *** empty log message ***
 *
 *    Revision 1.62  2009/11/10 19:04:25  sehunchun
 *    Variable coefficients for HDG2D Solver
 *
 *    Revision 1.61  2009/11/09 15:43:50  sehunchun
 *    HDG2DManifold Solver with Variable coefficients
 *
 *    Revision 1.60  2009/09/24 10:50:51  cbiotto
 *    Updates for variable order expansions
 *
 *    Revision 1.59  2009/09/23 12:42:40  pvos
 *    Updates for variable order expansions
 *
 *    Revision 1.58  2009/09/06 22:24:00  sherwin
 *    Updates for Navier-Stokes solver
 *
 *    Revision 1.57  2009/08/19 14:13:35  claes
 *    Removed Gauss-Kronrod parts
 *
 *    Revision 1.56  2009/07/08 17:19:48  sehunchun
 *    Deleting GetTanBasis
 *
 *    Revision 1.55  2009/07/08 11:11:24  sehunchun
 *    Adding GetSurfaceNormal Function
 *
 *    Revision 1.54  2009/07/03 15:34:52  sehunchun
 *    Adding GetTanBasis function
 *
 *    Revision 1.53  2009/06/18 11:47:24  claes
 *    changes supporting the static use of Kronrod points
 *
 *    Revision 1.52  2009/04/27 21:34:07  sherwin
 *    Updated WriteToField
 *
 *    Revision 1.51  2009/04/03 15:02:36  sherwin
 *    Made default Create Matrix the call through to the StdRegions generalised operators
 *
 *    Revision 1.50  2009/01/21 16:59:57  pvos
 *    Added additional geometric factors to improve efficiency
 *
 *    Revision 1.49  2008/12/16 11:32:33  pvos
 *    Performance updates
 *
 *    Revision 1.48  2008/11/24 10:31:14  pvos
 *    Changed name from _PartitionedOp to _MatFree
 *
 *    Revision 1.47  2008/11/19 16:01:41  pvos
 *    Added functionality for variable Laplacian coeffcients
 *
 *    Revision 1.46  2008/11/05 16:08:15  pvos
 *    Added elemental optimisation functionality
 *
 *    Revision 1.45  2008/09/23 18:20:25  pvos
 *    Updates for working ProjectContField3D demo
 *
 *    Revision 1.44  2008/09/09 15:05:09  sherwin
 *    Updates related to cuved geometries. Normals have been removed from m_metricinfo and replaced with a direct evaluation call. Interp methods have been moved to LibUtilities
 *
 *    Revision 1.43  2008/08/28 15:03:37  pvos
 *    small efficiency updates
 *
 *    Revision 1.42  2008/08/14 22:12:57  sherwin
 *    Introduced Expansion classes and used them to define HDG routines, has required quite a number of virtual functions to be added
 *
 *    Revision 1.41  2008/07/31 21:25:13  sherwin
 *    Mods for DG Advection
 *
 *    Revision 1.40  2008/07/31 11:13:22  sherwin
 *    Depracated GetEdgeBasis and replaced with DetEdgeBasisKey
 *
 *    Revision 1.39  2008/07/19 21:15:38  sherwin
 *    Removed MapTo function, made orientation anticlockwise, changed enum from BndSys to BndLam
 *
 *    Revision 1.38  2008/07/09 11:44:49  sherwin
 *    Replaced GetScaleFactor call with GetConstant(0)
 *
 *    Revision 1.37  2008/07/04 10:19:05  pvos
 *    Some updates
 *
 *    Revision 1.36  2008/07/02 14:09:18  pvos
 *    Implementation of HelmholtzMatOp and LapMatOp on shape level
 *
 *    Revision 1.35  2008/06/06 23:26:02  ehan
 *    Added doxygen documentation
 *
 *    Revision 1.34  2008/06/05 20:19:01  ehan
 *    Fixed undefined function GetGtype() in the ASSERTL2().
 *
 *    Revision 1.33  2008/06/02 23:35:34  ehan
 *    Fixed warning : no new line at end of file
 *
 *    Revision 1.32  2008/05/30 00:33:48  delisi
 *    Renamed StdRegions::ShapeType to StdRegions::ExpansionType.
 *
 *    Revision 1.31  2008/05/29 21:33:37  pvos
 *    Added WriteToFile routines for Gmsh output format + modification of BndCond implementation in MultiRegions
 *
 *    Revision 1.30  2008/05/29 01:02:13  bnelson
 *    Added precompiled header support.
 *
 *    Revision 1.29  2008/05/07 16:05:21  pvos
 *    Mapping + Manager updates
 *
 *    Revision 1.28  2008/04/06 05:59:05  bnelson
 *    Changed ConstArray to Array<const>
 *
 *    Revision 1.27  2008/04/02 22:19:26  pvos
 *    Update for 2D local to global mapping
 *
 *    Revision 1.26  2008/03/18 14:12:53  pvos
 *    Update for nodal triangular helmholtz solver
 *
 *    Revision 1.25  2008/03/12 15:24:29  pvos
 *    Clean up of the code
 *
 *    Revision 1.24  2007/12/17 13:04:30  sherwin
 *    Modified GenMatrix to take a StdMatrixKey and removed m_constant from MatrixKey
 *
 *    Revision 1.23  2007/12/06 22:49:09  pvos
 *    2D Helmholtz solver updates
 *
 *    Revision 1.22  2007/11/08 16:54:27  pvos
 *    Updates towards 2D helmholtz solver
 *
 *    Revision 1.21  2007/08/11 23:41:22  sherwin
 *    Various updates
 *
 *    Revision 1.20  2007/07/28 05:09:33  sherwin
 *    Fixed version with updated MemoryManager
 *
 *    Revision 1.19  2007/07/20 00:45:52  bnelson
 *    Replaced boost::shared_ptr with Nektar::ptr
 *
 *    Revision 1.18  2007/07/12 12:53:01  sherwin
 *    Updated to have a helmholtz matrix
 *
 *    Revision 1.17  2007/07/11 19:26:04  sherwin
 *    update for new Manager structure
 *
 *    Revision 1.16  2007/07/11 06:36:23  sherwin
 *    Updates with MatrixManager update
 *
 *    Revision 1.15  2007/07/10 17:17:26  sherwin
 *    Introduced Scaled Matrices into the MatrixManager
 *
 *    Revision 1.14  2007/06/17 19:00:45  bnelson
 *    Removed unused variables.
 *
 *    Revision 1.13  2007/06/07 15:54:19  pvos
 *    Modificications to make Demos/MultiRegions/ProjectCont2D work correctly.
 *    Also made corrections to various ASSERTL2 calls
 *
 *    Revision 1.12  2007/06/06 11:29:31  pvos
 *    Changed ErrorUtil::Error into NEKERROR (modifications in ErrorUtil.hpp caused compiler errors)
 *
 *    Revision 1.11  2007/06/01 17:08:07  pvos
 *    Modification to make LocalRegions/Project2D run correctly (PART1)
 *
 *    Revision 1.10  2007/05/31 19:13:12  pvos
 *    Updated NodalTriExp + LocalRegions/Project2D + some other modifications
 *
 *    Revision 1.9  2007/05/31 11:38:17  pvos
 *    Updated QuadExp and TriExp
 *
 *    Revision 1.8  2006/12/10 18:59:46  sherwin
 *    Updates for Nodal points
 *
 *    Revision 1.7  2006/06/13 18:05:01  sherwin
 *    Modifications to make MultiRegions demo ProjectLoc2D execute properly.
 *
 *    Revision 1.6  2006/06/02 18:48:39  sherwin
 *    Modifications to make ProjectLoc2D run bit there are bus errors for order > 3
 *
 *    Revision 1.5  2006/06/01 14:15:58  sherwin
 *    Added typdef of boost wrappers and made GeoFac a boost shared pointer.
 *
 *    Revision 1.4  2006/05/30 14:00:04  sherwin
 *    Updates to make MultiRegions and its Demos work
 *
 *    Revision 1.3  2006/05/29 17:05:49  sherwin
 *    Modified to put shared_ptr around geom definitions
 *
 *    Revision 1.2  2006/05/06 20:36:16  sherwin
 *    Modifications to get LocalRegions/Project1D working
 *
 *    Revision 1.1  2006/05/04 18:58:46  kirby
 *    *** empty log message ***
 *
 *    Revision 1.17  2006/03/13 19:47:54  sherwin
 *
 *    Fixed bug related to constructor of GeoFac and also makde arguments to GeoFac all consts
 *
 *    Revision 1.16  2006/03/13 18:20:33  sherwin
 *
 *    Fixed error in ResetGmat
 *
 *    Revision 1.15  2006/03/12 21:59:48  sherwin
 *
 *    compiling version of LocalRegions
 *
 *    Revision 1.14  2006/03/12 07:43:32  sherwin
 *
 *    First revision to meet coding standard. Needs to be compiled
 *
 **/
