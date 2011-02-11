///////////////////////////////////////////////////////////////////////////////
//
// File ContField2D.cpp
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
// Description: Field definition for 2D domain with boundary conditions
//
///////////////////////////////////////////////////////////////////////////////

#include <MultiRegions/ContField2D.h>

namespace Nektar
{
    namespace MultiRegions
    {
        /**
         * @class ContField2D
         * The class #ContField2D is
         * able to incorporate the boundary conditions imposed to the problem
         * to be solved. Therefore, the class is equipped with three additional
         * data members:
         * - #m_bndCondExpansions
         * - #m_bndTypes
         * - #m_bndCondEquations
         *
         * The first data structure, #m_bndCondExpansions, contains the
         * one-dimensional spectral/hp expansion on the boundary,  #m_bndTypes
         * stores information about the type of boundary condition on the
         * different parts of the boundary while #m_bndCondEquations holds the
         * equation of the imposed boundary conditions.
         *
         * Furthermore, in case of Dirichlet boundary conditions, this class is
         * capable of lifting a known solution satisfying these boundary
         * conditions. If we denote the unknown solution by
         * \f$u^{\mathcal{H}}(\boldsymbol{x})\f$ and the known Dirichlet
         * boundary conditions by \f$u^{\mathcal{D}}(\boldsymbol{x})\f$, the
         * expansion then can be decomposed as
         * \f[ u^{\delta}(\boldsymbol{x}_i)=u^{\mathcal{D}}(\boldsymbol{x}_i)+
         * u^{\mathcal{H}}(\boldsymbol{x}_i)=\sum_{n=0}^{N^{\mathcal{D}}-1}
         * \hat{u}_n^{\mathcal{D}}\Phi_n(\boldsymbol{x}_i)+
         * \sum_{n={N^{\mathcal{D}}}}^{N_{\mathrm{dof}}-1}
         *  \hat{u}_n^{\mathcal{H}} \Phi_n(\boldsymbol{x}_i).\f]
         * This lifting is accomplished by ordering the known global degrees of
         * freedom, prescribed by the Dirichlet boundary conditions, first in
         * the global array
         * \f$\boldsymbol{\hat{u}}\f$, that is,
         * \f[\boldsymbol{\hat{u}}=\left[ \begin{array}{c}
         * \boldsymbol{\hat{u}}^{\mathcal{D}}\\
         * \boldsymbol{\hat{u}}^{\mathcal{H}}
         * \end{array} \right].\f]
         * Such kind of expansions are also referred to as continuous fields.
         * This class should be used when solving 2D problems using a standard
         * Galerkin approach.
         */

        /**
         *
         */
        ContField2D::ContField2D():
            DisContField2D(),
            m_locToGloMap(),
            m_contNcoeffs(0),
            m_contCoeffs(),
            m_globalMat()
        {
        }


        /**
         * Given a mesh \a graph2D, containing information about the domain and
         * the spectral/hp element expansion, this constructor fills the list
         * of local expansions #m_exp with the proper expansions, calculates
         * the total number of quadrature points \f$\boldsymbol{x}_i\f$ and
         * local expansion coefficients \f$\hat{u}^e_n\f$ and allocates memory
         * for the arrays #m_coeffs and #m_phys. Furthermore, it constructs the
         * mapping array (contained in #m_locToGloMap) for the transformation
         * between local elemental level and global level, it calculates the
         * total number global expansion coefficients \f$\hat{u}_n\f$ and
         * allocates memory for the array #m_contCoeffs.
         *
         * @param   graph2D     A mesh, containing information about the domain
         *                      and the spectral/hp element expansion.
         * @param   solnType    Type of global system to use.
         */
        ContField2D::ContField2D(SpatialDomains::MeshGraph2D &graph2D,
                                 const GlobalSysSolnType solnType):
            DisContField2D(graph2D,solnType,false),
            m_globalMat(MemoryManager<GlobalMatrixMap>::AllocateSharedPtr()),
            m_globalLinSys(MemoryManager<GlobalLinSysMap>::AllocateSharedPtr())
        {
            ApplyGeomInfo(graph2D);

            m_locToGloMap = MemoryManager<LocalToGlobalC0ContMap>
                ::AllocateSharedPtr(m_ncoeffs,*this,solnType);

            m_contNcoeffs = m_locToGloMap->GetNumGlobalCoeffs();
            m_contCoeffs  = Array<OneD,NekDouble>(m_contNcoeffs,0.0);
        }


        /**
         * Given a mesh \a graph2D, containing information about the domain and
         * the spectral/hp element expansion, this constructor fills the list
         * of local expansions #m_exp with the proper expansions, calculates
         * the total number of quadrature points \f$\boldsymbol{x}_i\f$ and
         * local expansion coefficients \f$\hat{u}^e_n\f$ and allocates memory
         * for the arrays #m_coeffs and #m_phys. Furthermore, it constructs the
         * mapping array (contained in #m_locToGloMap) for the transformation
         * between local elemental level and global level, it calculates the
         * total number global expansion coefficients \f$\hat{u}_n\f$ and
         * allocates memory for the array #m_contCoeffs. The constructor also
         * discretises the boundary conditions, specified by the argument \a
         * bcs, by expressing them in terms of the coefficient of the expansion
         * on the boundary.
         *
         * @param   graph2D     A mesh, containing information about the domain
         *                      and the spectral/hp element expansion.
         * @param   bcs         The boundary conditions.
         * @param   bc_loc      The index of the session variable associated
         *                      with the boundary conditions to enforce.
         * @param   solnType    Type of global system to use.
         */
        ContField2D::ContField2D(SpatialDomains::MeshGraph2D &graph2D,
                                 SpatialDomains::BoundaryConditions &bcs,
                                 const int bc_loc,
                                 const GlobalSysSolnType solnType,
                                 bool DeclareCoeffPhysArrays):
            DisContField2D(graph2D,bcs,bc_loc,solnType,false,DeclareCoeffPhysArrays),
            m_globalMat(MemoryManager<GlobalMatrixMap>::AllocateSharedPtr()),
            m_globalLinSys(MemoryManager<GlobalLinSysMap>::AllocateSharedPtr())
        {
            ApplyGeomInfo(graph2D);

            map<int,int> periodicEdges;
            vector<map<int,int> > periodicVertices;
            GetPeriodicEdges(graph2D,bcs,bcs.GetVariable(bc_loc),
                             periodicVertices,periodicEdges);

            m_locToGloMap = MemoryManager<LocalToGlobalC0ContMap>
                ::AllocateSharedPtr(m_ncoeffs,*this,
                                    solnType, m_bndCondExpansions,
                                    m_bndConditions, periodicVertices,
                                    periodicEdges);

            m_contNcoeffs = m_locToGloMap->GetNumGlobalCoeffs();
            m_contCoeffs  = Array<OneD,NekDouble>(m_contNcoeffs,0.0);
        }


        /**
         * Given a mesh \a graph2D, containing information about the domain and
         * the spectral/hp element expansion, this constructor fills the list
         * of local expansions #m_exp with the proper expansions, calculates
         * the total number of quadrature points \f$\boldsymbol{x}_i\f$ and
         * local expansion coefficients \f$\hat{u}^e_n\f$ and allocates memory
         * for the arrays #m_coeffs and #m_phys. Furthermore, it constructs the
         * mapping array (contained in #m_locToGloMap) for the transformation
         * between local elemental level and global level, it calculates the
         * total number global expansion coefficients \f$\hat{u}_n\f$ and
         * allocates memory for the array #m_contCoeffs. The constructor also
         * discretises the boundary conditions, specified by the argument \a
         * bcs, by expressing them in terms of the coefficient of the expansion
         * on the boundary.
         *
         * @param   In          Existing ContField2D object used to provide the
         *                      local to global mapping information and
         *                      global solution type.
         * @param   graph2D     A mesh, containing information about the domain
         *                      and the spectral/hp element expansion.
         * @param   bcs         The boundary conditions.
         * @param   bc_loc
         */
        ContField2D::ContField2D(const ContField2D &In,
                                 SpatialDomains::MeshGraph2D &graph2D,
                                 SpatialDomains::BoundaryConditions &bcs,
                                 const int bc_loc,
                                 bool DeclareCoeffPhysArrays):
            DisContField2D(In,graph2D,bcs,bc_loc,false,DeclareCoeffPhysArrays),
            m_globalMat   (MemoryManager<GlobalMatrixMap>::AllocateSharedPtr()),
            m_globalLinSys(MemoryManager<GlobalLinSysMap>::AllocateSharedPtr())
        {

            ApplyGeomInfo(graph2D);

            if(!SameTypeOfBoundaryConditions(In))
            {
                map<int,int> periodicEdges;
                vector<map<int,int> >periodicVertices;
                GetPeriodicEdges(graph2D,bcs,bcs.GetVariable(bc_loc),
                                 periodicVertices,periodicEdges);

                GlobalSysSolnType solnType
                                    = In.m_locToGloMap->GetGlobalSysSolnType();
                m_locToGloMap = MemoryManager<LocalToGlobalC0ContMap>
                    ::AllocateSharedPtr(m_ncoeffs,*this, solnType,
                                        m_bndCondExpansions,
                                        m_bndConditions,
                                        periodicVertices,
                                        periodicEdges);
            }
            else
            {
                m_locToGloMap = In.m_locToGloMap;
            }

            m_contNcoeffs = m_locToGloMap->GetNumGlobalCoeffs();
            m_contCoeffs  = Array<OneD,NekDouble>(m_contNcoeffs,0.0);
        }


        /**
         * Given a mesh \a graph2D, containing information about the domain and
         * the spectral/hp element expansion, this constructor fills the list
         * of local expansions #m_exp with the proper expansions, calculates
         * the total number of quadrature points \f$\boldsymbol{x}_i\f$ and
         * local expansion coefficients \f$\hat{u}^e_n\f$ and allocates memory
         * for the arrays #m_coeffs and #m_phys. Furthermore, it constructs the
         * mapping array (contained in #m_locToGloMap) for the transformation
         * between local elemental level and global level, it calculates the
         * total number global expansion coefficients \f$\hat{u}_n\f$ and
         * allocates memory for the array #m_contCoeffs. The constructor also
         * discretises the boundary conditions, specified by the argument \a
         * bcs, by expressing them in terms of the coefficient of the expansion
         * on the boundary.
         *
         * @param   graph2D     A mesh, containing information about the domain
         *                      and the spectral/hp element expansion.
         * @param   bcs         The boundary conditions.
         * @param   variable    An optional parameter to indicate for which
         *                      variable the field should be constructed.
         */
        ContField2D::ContField2D(SpatialDomains::MeshGraph2D &graph2D,
                                 SpatialDomains::BoundaryConditions &bcs,
                                 const std::string variable,
                                 const GlobalSysSolnType solnType):
            DisContField2D(graph2D,bcs,variable,solnType,false),
            m_globalMat(MemoryManager<GlobalMatrixMap>::AllocateSharedPtr()),
            m_globalLinSys(MemoryManager<GlobalLinSysMap>::AllocateSharedPtr())
        {
            GenerateBoundaryConditionExpansion(graph2D,bcs,variable);
            EvaluateBoundaryConditions();
            ApplyGeomInfo(graph2D);

            map<int,int> periodicEdges;
            vector<map<int,int> >periodicVertices;
            GetPeriodicEdges(graph2D,bcs,variable,periodicVertices,
                             periodicEdges);

            m_locToGloMap = MemoryManager<LocalToGlobalC0ContMap>
                ::AllocateSharedPtr(m_ncoeffs,*this,
                                    solnType,
                                    m_bndCondExpansions,
                                    m_bndConditions,
                                    periodicVertices,
                                    periodicEdges);

            m_contNcoeffs = m_locToGloMap->GetNumGlobalCoeffs();
            m_contCoeffs  = Array<OneD,NekDouble>(m_contNcoeffs,0.0);
        }


        /**
         * Initialises the object as a copy of an existing ContField2D object.
         * @param   In                       Existing ContField2D object.
         * @param DeclareCoeffPhysArrays     bool to declare if \a m_phys
         * and \a m_coeffs should be declared. Default is true
         */
        ContField2D::ContField2D(const ContField2D &In, bool DeclareCoeffPhysArrays):
            DisContField2D(In,DeclareCoeffPhysArrays),
            m_locToGloMap(In.m_locToGloMap),
            m_contNcoeffs(In.m_contNcoeffs),
            m_globalMat(In.m_globalMat),
            m_globalLinSys(In.m_globalLinSys)
        {
            if(DeclareCoeffPhysArrays)
            {
                m_contCoeffs = Array<OneD, NekDouble> (m_contNcoeffs,0.0);
            }
        }


        /**
         *
         */
        ContField2D::~ContField2D()
        {
        }


        /**
         * Given a function \f$f(\boldsymbol{x})\f$ defined at the quadrature
         * points, this function determines the unknown global coefficients
         * \f$\boldsymbol{\hat{u}}^{\mathcal{H}}\f$ employing a discrete
         * Galerkin projection from physical space to coefficient
         * space. The operation is evaluated by the function #GlobalSolve using
         * the global mass matrix.
         *
         * The values of the function \f$f(\boldsymbol{x})\f$ evaluated at the
         * quadrature points \f$\boldsymbol{x}_i\f$ should be contained in the
         * variable #m_phys of the ExpList object \a Sin. The resulting global
         * coefficients \f$\hat{u}_g\f$ are stored in the array #m_contCoeffs.
         *
         * @param   Sin         An ExpList, containing the discrete evaluation
         *                      of \f$f(\boldsymbol{x})\f$ at the quadrature
         *                      points in its array #m_phys.
         */
        void ContField2D::FwdTrans(const Array<OneD, const NekDouble> &inarray,
                                         Array<OneD,       NekDouble> &outarray,
                                   bool  UseContCoeffs)
        {
            // Inner product of forcing
            Array<OneD,NekDouble> wsp(m_contNcoeffs);
            IProductWRTBase(inarray,wsp,true);

            // Solve the system
            GlobalLinSysKey key(StdRegions::eMass,
                                m_locToGloMap,
                                m_locToGloMap->GetGlobalSysSolnType());

            if(UseContCoeffs)
            {
                GlobalSolve(key,wsp,outarray);
            }
            else
            {
                Array<OneD,NekDouble> tmp(m_contNcoeffs,0.0);
                GlobalSolve(key,wsp,tmp);
                GlobalToLocal(tmp,outarray);
            }
        }


        /**
         * Computes the matrix vector product
         * @f$ \mathbf{y} = \mathbf{M}^{-1}\mathbf{x} @f$. If \a UseContCoeffs
         * is set then the elemental system is used directly. If not set, the
         * global system is assembled, the system is solved, and mapped back to
         * the local elemental system.
         *
         * @param   inarray     Input vector @f$\mathbf{x}@f$.
         * @param   outarray    Output vector @f$\mathbf{y}@f$.
         * @param   UseContCoeffs   Flag for using global system.
         */
        void ContField2D::MultiplyByInvMassMatrix(
                                const Array<OneD, const NekDouble> &inarray,
                                      Array<OneD,       NekDouble> &outarray,
                                bool  UseContCoeffs)

        {
            GlobalLinSysKey key(StdRegions::eMass,m_locToGloMap,
                                m_locToGloMap->GetGlobalSysSolnType());

            if(UseContCoeffs)
            {
                if(inarray.data() == outarray.data())
                {
                    Array<OneD, NekDouble> tmp(m_contNcoeffs,0.0);
                    Vmath::Vcopy(m_contNcoeffs,inarray,1,tmp,1);
                    GlobalSolve(key,tmp,outarray);
                }
                else
                {
                    GlobalSolve(key,inarray,outarray);
                }
            }
            else
            {
                Array<OneD, NekDouble> globaltmp(m_contNcoeffs,0.0);

                if(inarray.data() == outarray.data())
                {
                    Array<OneD,NekDouble> tmp(inarray.num_elements());
                    Vmath::Vcopy(inarray.num_elements(),inarray,1,tmp,1);
                    Assemble(tmp,outarray);
                }
                else
                {
                    Assemble(inarray,outarray);
                }

                GlobalSolve(key,outarray,globaltmp);
                GlobalToLocal(globaltmp,outarray);
            }
        }


        /**
         * Consider the two dimensional Laplace equation,
         * \f[\nabla\cdot\left(\boldsymbol{\sigma}\nabla
         * u(\boldsymbol{x})\right) = f(\boldsymbol{x}),\f] supplemented with
         * appropriate boundary conditions (which are contained in the data
         * member #m_bndCondExpansions). In the equation above
         * \f$\boldsymbol{\sigma}\f$ is the (symmetric positive definite)
         * diffusion tensor:
         * \f[ \sigma = \left[ \begin{array}{cc}
         * \sigma_{00}(\boldsymbol{x},t) & \sigma_{01}(\boldsymbol{x},t) \\
         * \sigma_{01}(\boldsymbol{x},t) & \sigma_{11}(\boldsymbol{x},t)
         * \end{array} \right]. \f]
         * Applying a \f$C^0\f$ continuous Galerkin discretisation, this
         * equation leads to the following linear system:
         * \f[\boldsymbol{L}
         * \boldsymbol{\hat{u}}_g=\boldsymbol{\hat{f}}\f]
         * where \f$\boldsymbol{L}\f$ is the Laplacian matrix. This function
         * solves the system above for the global coefficients
         * \f$\boldsymbol{\hat{u}}\f$ by a call to the function #GlobalSolve.
         *
         * The values of the function \f$f(\boldsymbol{x})\f$ evaluated at the
         * quadrature points \f$\boldsymbol{x}_i\f$ should be contained in the
         * variable #m_phys of the ExpList object \a Sin. The resulting global
         * coefficients \f$\boldsymbol{\hat{u}}_g\f$ are stored in the array
         * #m_contCoeffs.
         *
         * @param   Sin         An ExpList, containing the discrete evaluation
         *                      of the forcing function \f$f(\boldsymbol{x})\f$
         *                      at the quadrature points in its array #m_phys.
         * @param   variablecoeffs The (optional) parameter containing the
         *                      coefficients evaluated at the quadrature
         *                      points. It is an Array of (three) arrays which
         *                      stores the laplacian coefficients in the
         *                      following way
         * \f[\mathrm{variablecoeffs} = \left[ \begin{array}{c}
         * \left[\sigma_{00}(\boldsymbol{x_i},t)\right]_i \\
         * \left[\sigma_{01}(\boldsymbol{x_i},t)\right]_i \\
         * \left[\sigma_{11}(\boldsymbol{x_i},t)\right]_i
         * \end{array}\right]
         * \f]
         * If this argument is not passed to the function, the following
         * equation will be solved:
         * \f[\nabla^2u(\boldsymbol{x}) = f(\boldsymbol{x}),\f]
         *
         * @param   time        The time-level at which the coefficients are
         *                      evaluated
         */
        void ContField2D::LaplaceSolve(
                const Array<OneD, const NekDouble> &inarray,
                      Array<OneD,       NekDouble> &outarray,
                const Array<OneD, const NekDouble> &dirForcing,
                const Array<OneD,       Array<OneD,NekDouble> >& variablecoeffs,
                NekDouble time,
                bool UseContCoeffs)
        {
            // Inner product of forcing
            Array<OneD,NekDouble> wsp(m_contNcoeffs);
            IProductWRTBase(inarray,wsp,true);
            // Note -1.0 term necessary to invert forcing function to
            // be consistent with matrix definition
            Vmath::Neg(m_contNcoeffs, wsp, 1);

            // Forcing function with weak boundary conditions
            int i,j;
            int bndcnt=m_locToGloMap->GetNumLocalDirBndCoeffs();
            for(i = m_numDirBndCondExpansions;
                                    i < m_bndCondExpansions.num_elements(); ++i)
            {
                for(j = 0; j < (m_bndCondExpansions[i])->GetNcoeffs(); j++)
                {
                    wsp[m_locToGloMap
                                ->GetBndCondCoeffsToGlobalCoeffsMap(bndcnt++)]
                        += (m_bndCondExpansions[i]->GetCoeffs())[j];
                }
            }

            // Solve the system
            GlobalLinSysKey key(StdRegions::eLaplacian,m_locToGloMap,time,
                                variablecoeffs);

            if(UseContCoeffs)
            {
                GlobalSolve(key,wsp,outarray,dirForcing);
            }
            else
            {
                Array<OneD,NekDouble> tmp(m_contNcoeffs,0.0);
                GlobalSolve(key,wsp,tmp,dirForcing);
                GlobalToLocal(tmp,outarray);
            }
        }


        /**
         * Constructs the GlobalLinearSysKey for the linear advection operator
         * with the supplied parameters, and computes the eigenvectors and
         * eigenvalues of the associated matrix.
         * @param   ax          Advection parameter, x.
         * @param   ay          Advection parameter, y.
         * @param   Real        Computed eigenvalues, real component.
         * @param   Imag        Computed eigenvalues, imag component.
         * @param   Evecs       Computed eigenvectors.
         */
        void ContField2D::LinearAdvectionEigs(const NekDouble ax,
                                              const NekDouble ay,
                                              Array<OneD, NekDouble> &Real,
                                              Array<OneD, NekDouble> &Imag,
                                              Array<OneD, NekDouble> &Evecs)
        {
            // Solve the system
            Array<OneD, Array<OneD, NekDouble> > vel(2);
            Array<OneD, NekDouble> vel_x(m_npoints,ax);
            Array<OneD, NekDouble> vel_y(m_npoints,ay);
            vel[0] = vel_x;
            vel[1] = vel_y;

            GlobalLinSysKey key(StdRegions::eLinearAdvectionReaction,m_locToGloMap,
                                0.0,vel);

            DNekMatSharedPtr   Gmat = GenGlobalMatrixFull(key,m_locToGloMap);
            Gmat->EigenSolve(Real,Imag,Evecs);
        }


        /**
         * Given a linear system specified by the key \a key,
         * \f[\boldsymbol{M}\boldsymbol{\hat{u}}_g=\boldsymbol{\hat{f}},\f]
         * this function solves this linear system taking into account the
         * boundary conditions specified in the data member
         * #m_bndCondExpansions. Therefore, it adds an array
         * \f$\boldsymbol{\hat{g}}\f$ which represents the non-zero surface
         * integral resulting from the weak boundary conditions (e.g. Neumann
         * boundary conditions) to the right hand side, that is,
         * \f[\boldsymbol{M}\boldsymbol{\hat{u}}_g=\boldsymbol{\hat{f}}+
         * \boldsymbol{\hat{g}}.\f]
         * Furthermore, it lifts the known degrees of freedom which are
         * prescribed by the Dirichlet boundary conditions. As these known
         * coefficients \f$\boldsymbol{\hat{u}}^{\mathcal{D}}\f$ are numbered
         * first in the global coefficient array \f$\boldsymbol{\hat{u}}_g\f$,
         * the linear system can be decomposed as,
         * \f[\left[\begin{array}{cc}
         * \boldsymbol{M}^{\mathcal{DD}}&\boldsymbol{M}^{\mathcal{DH}}\\
         * \boldsymbol{M}^{\mathcal{HD}}&\boldsymbol{M}^{\mathcal{HH}}
         * \end{array}\right]
         * \left[\begin{array}{c}
         * \boldsymbol{\hat{u}}^{\mathcal{D}}\\
         * \boldsymbol{\hat{u}}^{\mathcal{H}}
         * \end{array}\right]=
         * \left[\begin{array}{c}
         * \boldsymbol{\hat{f}}^{\mathcal{D}}\\
         * \boldsymbol{\hat{f}}^{\mathcal{H}}
         * \end{array}\right]+
         * \left[\begin{array}{c}
         * \boldsymbol{\hat{g}}^{\mathcal{D}}\\
         * \boldsymbol{\hat{g}}^{\mathcal{H}}
         * \end{array}\right]
         * \f]
         * which will then be solved for the unknown coefficients
         * \f$\boldsymbol{\hat{u}}^{\mathcal{H}}\f$ as,
         * \f[
         * \boldsymbol{M}^{\mathcal{HH}}\boldsymbol{\hat{u}}^{\mathcal{H}}=
         * \boldsymbol{\hat{f}}^{\mathcal{H}}+
         * \boldsymbol{\hat{g}}^{\mathcal{H}}-
         * \boldsymbol{M}^{\mathcal{HD}}\boldsymbol{\hat{u}}^{\mathcal{D}}\f]
         *
         * @param   mkey        This key uniquely defines the linear system to
         *                      be solved.
         * @param   Sin         An ExpList, containing the discrete evaluation
         *                      of the forcing function \f$f(\boldsymbol{x})\f$
         *                      at the quadrature points in its array #m_phys.
         * @param   ScaleForcing An optional parameter with which the forcing
         *                      vector \f$\boldsymbol{\hat{f}}\f$ should be
         *                      multiplied.
         * @note    inout contains initial guess and final output.
         */
        void ContField2D::GlobalSolve(
                                const GlobalLinSysKey &key,
                                const Array<OneD, const NekDouble>& rhs,
                                      Array<OneD,       NekDouble>& inout,
                                const Array<OneD, const NekDouble>& dirForcing)
        {
            int i,j;
            int bndcnt=0;
            int NumDirBcs = m_locToGloMap->GetNumGlobalDirBndCoeffs();

            // STEP 1: SET THE DIRICHLET DOFS TO THE RIGHT VALUE
            //         IN THE SOLUTION ARRAY
            const Array<OneD,const int>& map
                        = m_locToGloMap->GetBndCondCoeffsToGlobalCoeffsMap();
            for(i = 0; i < m_numDirBndCondExpansions; ++i)
            {
                const Array<OneD,const NekDouble>& coeffs
                                        = m_bndCondExpansions[i]->GetCoeffs();
                for(j = 0; j < (m_bndCondExpansions[i])->GetNcoeffs(); ++j)
                {
                    inout[map[bndcnt++]] = coeffs[j];
                }
            }
            // STEP 2: CALCULATE THE HOMOGENEOUS COEFFICIENTS
            if(m_contNcoeffs - NumDirBcs > 0)
            {
                GlobalLinSysSharedPtr LinSys = GetGlobalLinSys(key);
                LinSys->Solve(rhs,inout,m_locToGloMap,dirForcing);
            }
        }


        /**
         * Returns the global matrix associated with the given GlobalMatrixKey.
         * If the global matrix has not yet been constructed on this field,
         * it is first constructed using GenGlobalMatrix().
         * @param   mkey        Global matrix key.
         * @returns Assocated global matrix.
         */
        GlobalMatrixSharedPtr ContField2D::GetGlobalMatrix(
                                const GlobalMatrixKey &mkey)
        {
            ASSERTL1(mkey.LocToGloMapIsDefined(),
                     "To use method must have a LocalToGlobalBaseMap "
                     "attached to key");

            GlobalMatrixSharedPtr glo_matrix;
            GlobalMatrixMap::iterator matrixIter = m_globalMat->find(mkey);

            if(matrixIter == m_globalMat->end())
            {
                glo_matrix = GenGlobalMatrix(mkey,m_locToGloMap);
                (*m_globalMat)[mkey] = glo_matrix;
            }
            else
            {
                glo_matrix = matrixIter->second;
            }

            return glo_matrix;
        }


        /**
         * The function searches the map #m_globalLinSys to see if the
         * global matrix has been created before. If not, it calls the function
         * #GenGlobalLinSys to generate the requested global system.
         *
         * @param   mkey        This key uniquely defines the requested
         *                      linear system.
         */
        GlobalLinSysSharedPtr ContField2D::GetGlobalLinSys(
                                const GlobalLinSysKey &mkey)
        {
            ASSERTL1(mkey.LocToGloMapIsDefined(),
                     "To use method must have a LocalToGlobalBaseMap "
                     "attached to key");

            GlobalLinSysSharedPtr glo_matrix;
            GlobalLinSysMap::iterator matrixIter = m_globalLinSys->find(mkey);

            if(matrixIter == m_globalLinSys->end())
            {
                glo_matrix = GenGlobalLinSys(mkey,m_locToGloMap);
                (*m_globalLinSys)[mkey] = glo_matrix;
            }
            else
            {
                glo_matrix = matrixIter->second;
            }

            return glo_matrix;
        }


        int ContField2D::v_GetContNcoeffs() const
        {
            return m_contNcoeffs;
        }

        /**
         *
         */
        Array<OneD, NekDouble> &ContField2D::v_UpdateContCoeffs(void)
        {
            return ContField2D::UpdateContCoeffs();
        };


        /**
         *
         */
        void ContField2D::v_SetContCoeffsArray(Array<OneD, NekDouble> &inarray)
        {
            m_contCoeffs = inarray;
        }



        /**
         *
         */
        const Array<OneD, const NekDouble> &ContField2D::v_GetContCoeffs(void) const
        {
            return ContField2D::GetContCoeffs();
        };


        /**
         *
         */
        void  ContField2D::v_LocalToGlobal(void)
        {
            return ContField2D::LocalToGlobal();
        };


        /**
         *
         */
        void  ContField2D::v_GlobalToLocal(void)
        {
            return ContField2D::GlobalToLocal();
        };


        /**
         *
         */
        void ContField2D::v_BwdTrans(
                                     const Array<OneD, const NekDouble> &inarray,
                                     Array<OneD,       NekDouble> &outarray,
                                     bool  UseContCoeffs)
        {
            BwdTrans(inarray,outarray,UseContCoeffs);
        }


        /**
         *
         */
        void ContField2D::v_FwdTrans(
                                const Array<OneD, const NekDouble> &inarray,
                                      Array<OneD,       NekDouble> &outarray,
                                bool  UseContCoeffs)
        {
            FwdTrans(inarray,outarray,UseContCoeffs);
        }


        /**
         *
         */
        void ContField2D::v_MultiplyByInvMassMatrix(
                                const Array<OneD, const NekDouble> &inarray,
                                      Array<OneD,       NekDouble> &outarray,
                                bool  UseContCoeffs)
        {
            MultiplyByInvMassMatrix(inarray,outarray,UseContCoeffs);
        }


        /**
         * Solve the two-dimensional Helmholtz equation.
         * @param   inarray     Input forcing function.
         * @param   outarray    Output solution.
         * @param   lambda      Coefficient @f$\lambda@f$.
         * @param   varLambda   Spatially-dependent form of coefficient.
         * @param   varCoeff    Diffusive coefficient.
         */
        void ContField2D::v_HelmSolve(
                    const Array<OneD, const NekDouble> &inarray,
                          Array<OneD,       NekDouble> &outarray,
                          NekDouble lambda,
                    const Array<OneD, const NekDouble> &varLambda,
                    const Array<OneD, const Array<OneD, NekDouble> > &varCoeff)
        {
            v_HelmSolveCG(inarray, outarray, lambda, varLambda, varCoeff,
                              false, NullNekDouble1DArray);
        }


        /**
         * Consider the two dimensional Helmholtz equation,
         * \f[\nabla^2u(\boldsymbol{x})-\lambda u(\boldsymbol{x})
         * = f(\boldsymbol{x}),\f] supplemented with appropriate boundary
         * conditions (which are contained in the data member
         * #m_bndCondExpansions). Applying a \f$C^0\f$ continuous Galerkin
         * discretisation, this equation leads to the following linear system:
         * \f[\left(\boldsymbol{L}+\lambda\boldsymbol{M}\right)
         * \boldsymbol{\hat{u}}_g=\boldsymbol{\hat{f}}\f] where
         * \f$\boldsymbol{L}\f$ and \f$\boldsymbol{M}\f$ are the Laplacian and
         * mass matrix respectively. This function solves the system above for
         * the global coefficients \f$\boldsymbol{\hat{u}}\f$ by a call to the
         * function #GlobalSolve. It is assumed #m_contCoeff contains an
         * initial estimate for the solution.
         *
         * The values of the function \f$f(\boldsymbol{x})\f$
         * evaluated at the quadrature points \f$\boldsymbol{x}_i\f$
         * should be contained in the variable #m_phys of the ExpList
         * object \a inarray. The resulting global coefficients
         * \f$\boldsymbol{\hat{u}}_g\f$ are stored in the array
         * #m_contCoeffs or #m_coeffs depending on whether
         * \a UseContCoeffs is true or false
         *
         * @param   inarray     An ExpList, containing the discrete evaluation
         *                      of the forcing function \f$f(\boldsymbol{x})\f$
         *                      at the quadrature points in its array #m_phys.
         * @param   lambda      The parameter \f$\lambda\f$ of the Helmholtz
         *                      equation
         */
        void ContField2D::v_HelmSolveCG(
                    const Array<OneD, const NekDouble> &inarray,
                          Array<OneD,       NekDouble> &outarray,
                          NekDouble lambda,
                    const Array<OneD, const NekDouble> &varLambda,
                    const Array<OneD, const Array<OneD, NekDouble> > &varCoeff,
                          bool UseContCoeffs,
                    const Array<OneD, const NekDouble> &dirForcing)
        {
            //----------------------------------
            //  Setup RHS Inner product
            //----------------------------------
            // Inner product of forcing
            Array<OneD,NekDouble> wsp(m_contNcoeffs);
            IProductWRTBase(inarray,wsp,true);
            // Note -1.0 term necessary to invert forcing function to
            // be consistent with matrix definition
            Vmath::Neg(m_contNcoeffs, wsp, 1);

            // Forcing function with weak boundary conditions
            int i,j;
            int bndcnt=m_locToGloMap->GetNumLocalDirBndCoeffs();
            for(i = m_numDirBndCondExpansions;
                i < m_bndCondExpansions.num_elements(); ++i)
            {
                for(j = 0; j < (m_bndCondExpansions[i])->GetNcoeffs(); j++)
                {
                    wsp[m_locToGloMap
                        ->GetBndCondCoeffsToGlobalCoeffsMap(bndcnt++)]
                        += (m_bndCondExpansions[i]->GetCoeffs())[j];
                }
            }

            // Solve the system
            GlobalLinSysKey key(StdRegions::eHelmholtz,m_locToGloMap,lambda);

            if(UseContCoeffs)
            {
                Vmath::Zero(m_contNcoeffs,outarray,1);
                GlobalSolve(key,wsp,outarray,dirForcing);
            }
            else
            {
                Array<OneD,NekDouble> tmp(m_contNcoeffs,0.0);
                GlobalSolve(key,wsp,tmp,dirForcing);
                GlobalToLocal(tmp,outarray);
            }
        }


        /**
         * This is equivalent to the operation:
         * \f[\boldsymbol{M\hat{u}}_g\f]
         * where \f$\boldsymbol{M}\f$ is the global matrix of type specified by
         * \a mkey. After scattering the global array \a inarray to local
         * level, this operation is evaluated locally by the function
         * ExpList#GeneralMatrixOp. The global result is then obtained by a
         * global assembly procedure.
         *
         * @param   mkey        This key uniquely defines the type matrix
         *                      required for the operation.
         * @param   inarray     The vector \f$\boldsymbol{\hat{u}}_g\f$ of size
         *                      \f$N_{\mathrm{dof}}\f$.
         * @param   outarray    The resulting vector of size
         *                      \f$N_{\mathrm{dof}}\f$.
         */
        void ContField2D::v_GeneralMatrixOp(
                                       const GlobalMatrixKey             &gkey,
                                       const Array<OneD,const NekDouble> &inarray,
                                       Array<OneD,      NekDouble> &outarray,
                                       bool  UseContCoeffs)
        {
            if(UseContCoeffs)
            {
                bool doGlobalOp = m_globalOptParam->DoGlobalMatOp(
                                                        gkey.GetMatrixType());

                if(doGlobalOp)
                {
                    GlobalMatrixSharedPtr mat = GetGlobalMatrix(gkey);
                    mat->Multiply(inarray,outarray);
                }
                else
                {
                    Array<OneD,NekDouble> tmp1(2*m_ncoeffs);
                    Array<OneD,NekDouble> tmp2(tmp1+m_ncoeffs);
                    GlobalToLocal(inarray,tmp1);
                    GeneralMatrixOp_IterPerExp(gkey,tmp1,tmp2);
                    Assemble(tmp2,outarray);
                }
            }
            else
            {
                GeneralMatrixOp_IterPerExp(gkey,inarray,outarray);
            }
        }

        /**
         * First compute the inner product of forcing function with respect to
         * base, and then solve the system with the linear advection operator.
         * @param   velocity    Array of advection velocities in physical space
         * @param   inarray     Forcing function.
         * @param   outarray    Result.
         * @param   lambda      reaction coefficient
         * @param   UseContCoeffs   Use continuous coefficients.
         * @param   dirForcing  Dirichlet Forcing.
         */

        // could combine this with HelmholtzCG.
        void ContField2D::v_LinearAdvectionDiffusionReactionSolve(const Array<OneD, Array<OneD, NekDouble> > &velocity,
                                                       const Array<OneD, const NekDouble> &inarray,
                                                       Array<OneD, NekDouble> &outarray,
                                                       const NekDouble lambda,
                                                       bool        UseContCoeffs,
                                                       const Array<OneD, const NekDouble>& dirForcing)
        {
            // Inner product of forcing
            Array<OneD,NekDouble> wsp(m_contNcoeffs);
            IProductWRTBase(inarray,wsp,true);
            // Note -1.0 term necessary to invert forcing function to
            // be consistent with matrix definition
            Vmath::Neg(m_contNcoeffs, wsp, 1);

            // Forcing function with weak boundary conditions
            int i,j;
            int bndcnt=m_locToGloMap->GetNumLocalDirBndCoeffs();
            for(i = m_numDirBndCondExpansions;
                i < m_bndCondExpansions.num_elements(); ++i)
            {
                for(j = 0; j < (m_bndCondExpansions[i])->GetNcoeffs(); j++)
                {
                    wsp[m_locToGloMap
                        ->GetBndCondCoeffsToGlobalCoeffsMap(bndcnt++)]
                        += (m_bndCondExpansions[i]->GetCoeffs())[j];
                }
            }

            // Solve the system
            GlobalLinSysKey key(StdRegions::eLinearAdvectionDiffusionReaction,m_locToGloMap,lambda,velocity);

            if(UseContCoeffs)
            {
                GlobalSolve(key,wsp,outarray,dirForcing);
            }
            else
            {
                Array<OneD,NekDouble> tmp(m_contNcoeffs,0.0);
                GlobalSolve(key,wsp,tmp,dirForcing);
                GlobalToLocal(tmp,outarray);
            }
        }

        /**
         * First compute the inner product of forcing function with respect to
         * base, and then solve the system with the linear advection operator.
         * @param   velocity    Array of advection velocities in physical space
         * @param   inarray     Forcing function.
         * @param   outarray    Result.
         * @param   lambda      reaction coefficient
         * @param   UseContCoeffs   Use continuous coefficients.
         * @param   dirForcing  Dirichlet Forcing.
         */
        void ContField2D::v_LinearAdvectionReactionSolve(const Array<OneD, Array<OneD, NekDouble> > &velocity,
                                                       const Array<OneD, const NekDouble> &inarray,
                                                       Array<OneD, NekDouble> &outarray,
                                                       const NekDouble lambda,
                                                       bool        UseContCoeffs,
                                                       const Array<OneD, const NekDouble>& dirForcing)
        {
            // Inner product of forcing
            Array<OneD,NekDouble> wsp(m_contNcoeffs);
            IProductWRTBase(inarray,wsp,true);

            // Solve the system
            GlobalLinSysKey key(StdRegions::eLinearAdvectionReaction,m_locToGloMap,lambda,velocity);

            if(UseContCoeffs)
            {
                GlobalSolve(key,wsp,outarray,dirForcing);
            }
            else
            {
                Array<OneD,NekDouble> tmp(m_contNcoeffs,0.0);
                GlobalSolve(key,wsp,tmp,dirForcing);
                GlobalToLocal(tmp,outarray);
            }
        }


        /**
         *
         */
        const Array<OneD,const SpatialDomains::BoundaryConditionShPtr>&
                                ContField2D::v_GetBndConditions()
        {
            return GetBndConditions();
        }

    } // end of namespace
} //end of namespace
