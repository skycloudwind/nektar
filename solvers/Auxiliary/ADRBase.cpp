///////////////////////////////////////////////////////////////////////////////
//
// File ADRBase.cpp
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
// Description: Base definitions definiton for
// AdvectionReactiondiffusion, Euler and ShallowWater classes.
//
///////////////////////////////////////////////////////////////////////////////

#include <Auxiliary/ADRBase.h>

#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <string>

namespace Nektar
{
    /**
     * Basic construnctor
     */
    ADRBase::ADRBase(void):
        m_fields(0)
    {     
    }
    
    /**
     * Constructor. Creates ... of #DisContField2D fields
     */
    ADRBase::ADRBase(string &fileNameString, bool UseInputFileForProjectionType,
                     bool UseContinuousField)
    {
        SpatialDomains::MeshGraph graph; 

        // Read the geometry and the expansion information
        m_graph = graph.Read(fileNameString);

        // Also read and store the boundary conditions
	SpatialDomains::MeshGraph *meshptr = m_graph.get();
        m_boundaryConditions = MemoryManager<SpatialDomains::BoundaryConditions>::AllocateSharedPtr(meshptr);
        m_boundaryConditions->Read(fileNameString); 
        
        // set space dimension for use in class
        m_spacedim = m_graph->GetSpaceDimension();
        // save the input file name for output details. 
        m_sessionName = fileNameString;
        m_sessionName = m_sessionName.substr(0,m_sessionName.find_first_of(".")); // Pull out ending
        
        // Options to determine type of projection from file or
        // directly from constructor
        if(UseInputFileForProjectionType == true)
        {
            if(m_boundaryConditions->SolverInfoExists("PROJECTION"))
            {
                
                std::string ProjectStr = m_boundaryConditions->GetSolverInfo("PROJECTION");
                
                if((ProjectStr == "Continuous")||(ProjectStr == "Galerkin")||
                   (ProjectStr == "CONTINUOUS")||(ProjectStr == "GALERKIN"))
                {
                    m_projectionType = eGalerkin;
                }
                else if(ProjectStr == "DisContinuous")
                {
                    m_projectionType = eDiscontinuousGalerkin;
                }
                else 
                {
                    ASSERTL0(false,"PROJECTION value not recognised");
                }
            }
            else
            {
                cerr << "Projection type not specified in SOLVERINFO, defaulting to continuous Galerkin" << endl;
                
                m_projectionType = eGalerkin;
            }
        }
        else 
        {
            if(UseContinuousField == true)
            {
                m_projectionType = eGalerkin;
                
            }
            else
            {
                m_projectionType = eDiscontinuousGalerkin;
            }
        }


        SetADRBase(m_graph,m_boundaryConditions->GetNumVariables());
    }
    
  void ADRBase::SetADRBase(SpatialDomains::MeshGraphSharedPtr &mesh,
			   int nvariables)
  {
    int i;
    
    m_fields   = Array<OneD, MultiRegions::ExpListSharedPtr>(nvariables);
    m_spacedim = mesh->GetSpaceDimension();
    m_expdim   = mesh->GetMeshDimension();
    
    if(m_projectionType == eGalerkin)
      {
	switch(m_expdim)
	  {
	  case 1:
	    {
	      SpatialDomains::MeshGraph1DSharedPtr mesh1D;
	      
	      if(!(mesh1D = boost::dynamic_pointer_cast<SpatialDomains::MeshGraph1D>(mesh)))
              {
		  ASSERTL0(false,"Dynamics cast failed");
              }
	      
	      for(i = 0 ; i < m_fields.num_elements(); i++)
              {
		  m_fields[i] = MemoryManager<MultiRegions::ContField1D>::AllocateSharedPtr(*mesh1D,*m_boundaryConditions,i);
              }
	    }
	    break;
	  case 2:
	    {
	      SpatialDomains::MeshGraph2DSharedPtr mesh2D;
              
	      if(!(mesh2D = boost::dynamic_pointer_cast<SpatialDomains::MeshGraph2D>(mesh)))
              {
		  ASSERTL0(false,"Dynamics cast failed");
              }
	      
              i = 0;
              MultiRegions::ContField2DSharedPtr firstfield = 
		MemoryManager<MultiRegions::ContField2D>::AllocateSharedPtr(*mesh2D,*m_boundaryConditions,i);
              
              m_fields[0] = firstfield;
	      for(i = 1 ; i < m_fields.num_elements(); i++)
              {
                  m_fields[i] = MemoryManager<MultiRegions::ContField2D>::AllocateSharedPtr(*firstfield,*mesh2D,*m_boundaryConditions,i);
              }
	      break;
	    }
	  case 3:
	    ASSERTL0(false,"3 D not set up");
	  default:
	    ASSERTL0(false,"Expansion dimension not recognised");
	    break;
	  }
      }
    else // Discontinuous Field
      {
	
	switch(m_expdim)
	  {
	  case 1:
	    {
	      SpatialDomains::MeshGraph1DSharedPtr mesh1D;
	      
	      if(!(mesh1D = boost::dynamic_pointer_cast<SpatialDomains::MeshGraph1D>(mesh)))
		{
		  ASSERTL0(false,"Dynamics cast failed");
		}
	      
	      for(i = 0 ; i < m_fields.num_elements(); i++)
		{
		  m_fields[i] = MemoryManager<MultiRegions::DisContField1D>::AllocateSharedPtr(*mesh1D,*m_boundaryConditions,i);
		}
	    }
	    break;
	  case 2:
	    {
	      SpatialDomains::MeshGraph2DSharedPtr mesh2D;
              
	      if(!(mesh2D = boost::dynamic_pointer_cast<SpatialDomains::MeshGraph2D>(mesh)))
		{
		  ASSERTL0(false,"Dynamics cast failed");
		}
	      
	      for(i = 0 ; i < m_fields.num_elements(); i++)
		{
                  m_fields[i] = MemoryManager<MultiRegions::DisContField2D>::AllocateSharedPtr(*mesh2D,*m_boundaryConditions,i);
		}
	    }
	    break;
	  case 3:
	    ASSERTL0(false,"3 D not set up");
	  default:
	    ASSERTL0(false,"Expansion dimension not recognised");
	    break;
	  }
	    
	// Set up Normals. 
	switch(m_expdim)
	  {
	  case 1:
	    // no need??... 
	    break;
	  case 2:
	    {
	      m_traceNormals = Array<OneD, Array<OneD, NekDouble> >(m_spacedim);
	      
	      for(i = 0; i < m_spacedim; ++i)
		{
		  m_traceNormals[i] = Array<OneD, NekDouble> (m_fields[0]->GetTrace()->GetNpoints());
		}
	      
	      m_fields[0]->GetTrace()->GetNormals(m_traceNormals);
	    }
	    break;
	  case 3:
	    ASSERTL0(false,"3 D not set up");
	    break;
	  default:
	    ASSERTL0(false,"Expansion dimension not recognised");
	    break;
	  }
      }
    // Set Default Parameter
    
    if(m_boundaryConditions->CheckForParameter("Time") == true)
    {
	m_time  = m_boundaryConditions->GetParameter("Time");
    }
    else
    {
	m_time  = 0.0;
    }

    if(m_boundaryConditions->CheckForParameter("TimeStep") == true)
    {
        m_timestep   = m_boundaryConditions->GetParameter("TimeStep");
    }

    if(m_boundaryConditions->CheckForParameter("NumSteps") == true)
    {
        m_steps      = m_boundaryConditions->GetParameter("NumSteps");
    }
    else
    {
        m_steps  = 0;
    }
    
    if(m_boundaryConditions->CheckForParameter("IO_CheckSteps") == true)
    {
	m_checksteps = m_boundaryConditions->GetParameter("IO_CheckSteps");
    }
    else
    {
	m_checksteps = m_steps;
    }
  }

    void ADRBase::ZeroPhysFields(void)
    {
        for(int i = 0; i < m_fields.num_elements(); i++)
	{
            Vmath::Zero(m_fields[i]->GetNpoints(),m_fields[i]->UpdatePhys(),1);
        }
    }
  
    void ADRBase::SetInitialConditions(NekDouble initialtime, bool dumpInitialConditions)
    {
        std::string restartstr = "RESTART";

        cout << "Initial Conditions:" << endl;
        // Check for restart file. 
        if(m_boundaryConditions->FoundInitialCondition(restartstr))
        {
            SpatialDomains::ConstInitialConditionShPtr ifunc = m_boundaryConditions->GetInitialCondition(restartstr);

            std::string restartfile = ifunc->GetEquation();
            cout << "\tRestart file: "<< restartfile << endl; 
            ImportFld(restartfile);
        }
        else
        {
            int nq = m_fields[0]->GetNpoints();
            
            Array<OneD,NekDouble> x0(nq);
            Array<OneD,NekDouble> x1(nq);
            Array<OneD,NekDouble> x2(nq);
            
            // get the coordinates (assuming all fields have the same
            // discretisation)
            m_fields[0]->GetCoords(x0,x1,x2);
            
            for(int i = 0 ; i < m_fields.num_elements(); i++)
            {
                SpatialDomains::ConstInitialConditionShPtr ifunc = m_boundaryConditions->GetInitialCondition(i);
                for(int j = 0; j < nq; j++)
                {
                    (m_fields[i]->UpdatePhys())[j] = ifunc->Evaluate(x0[j],x1[j],x2[j],initialtime);
                }
                m_fields[i]->SetPhysState(true);
                m_fields[i]->FwdTrans(m_fields[i]->GetPhys(),m_fields[i]->UpdateCoeffs());
                cout << "\tField "<< m_boundaryConditions->GetVariable(i) <<": " << ifunc->GetEquation() << endl;
            }
        }
        
        if(dumpInitialConditions)
        {
            // dump initial conditions to file
            std::string outname = m_sessionName + "_initial.chk";
            WriteFld(outname);
        }
    }

    void ADRBase::SetPhysForcingFunctions(Array<OneD, MultiRegions::ExpListSharedPtr> &force)
        
    {
        int nq = m_fields[0]->GetNpoints();
      
        Array<OneD,NekDouble> x0(nq);
        Array<OneD,NekDouble> x1(nq);
        Array<OneD,NekDouble> x2(nq);
      
        // get the coordinates (assuming all fields have the same discretisation)
        force[0]->GetCoords(x0,x1,x2);
      
        for(int i = 0 ; i < m_fields.num_elements(); i++)
	{
            SpatialDomains::ConstForcingFunctionShPtr ffunc = m_boundaryConditions->GetForcingFunction(i);

            for(int j = 0; j < nq; j++)
	    {
                (force[i]->UpdatePhys())[j] = ffunc->Evaluate(x0[j],x1[j],x2[j]);
             }
             force[i]->SetPhysState(true);
        }
    }


    void ADRBase::EvaluateExactSolution(int field, Array<OneD, NekDouble> &outfield, 
                                        const NekDouble time)
    {
        int nq = m_fields[field]->GetNpoints();
        bool Readit = true;
      
        Array<OneD,NekDouble> x0(nq);
        Array<OneD,NekDouble> x1(nq);
        Array<OneD,NekDouble> x2(nq);
      
        // get the coordinates of the quad points
        m_fields[field]->GetCoords(x0,x1,x2);
        
        SpatialDomains::ConstExactSolutionShPtr ifunc = m_boundaryConditions->GetExactSolution(field);
        for(int j = 0; j < nq; j++)
        {
            outfield[j] = ifunc->Evaluate(x0[j],x1[j],x2[j],time);
        }
    }

    
    void ADRBase::EvaluateUserDefinedEqn(Array<OneD, Array<OneD, NekDouble> > &outfield)
    {
        int nq = m_fields[0]->GetNpoints();
        
        Array<OneD,NekDouble> x0(nq);
        Array<OneD,NekDouble> x1(nq);
        Array<OneD,NekDouble> x2(nq);
      
        // get the coordinates (assuming all fields have the same
        // discretisation)
        m_fields[0]->GetCoords(x0,x1,x2);
      
        for(int i = 0 ; i < m_fields.num_elements(); i++)
	{
            SpatialDomains::ConstUserDefinedEqnShPtr ifunc = m_boundaryConditions->GetUserDefinedEqn(i);
            for(int j = 0; j < nq; j++)
	    {
                outfield[i][j] = ifunc->Evaluate(x0[j],x1[j],x2[j]);
	    }
	}
        
    }

    NekDouble ADRBase::L2Error(int field, const Array<OneD, NekDouble> &exactsoln)
    {
        if(m_fields[field]->GetPhysState() == false)
        {
            m_fields[field]->BwdTrans(m_fields[field]->GetCoeffs(),
                                       m_fields[field]->UpdatePhys());
        }
        
        if(exactsoln.num_elements())
        {
            return m_fields[field]->L2(exactsoln);
        }
        else
        {
            Array<OneD, NekDouble> exactsoln(m_fields[field]->GetNpoints());
            
            EvaluateExactSolution(field,exactsoln,m_time);
            
            return m_fields[field]->L2(exactsoln);
        }
    }


    NekDouble ADRBase::LinfError(int field, const Array<OneD, NekDouble> &exactsoln)
    {
        if(m_fields[field]->GetPhysState() == false)
        {
            m_fields[field]->BwdTrans(m_fields[field]->GetCoeffs(),
                                      m_fields[field]->UpdatePhys());
        }
        
        if(exactsoln.num_elements())
        {
            return m_fields[field]->Linf(exactsoln);
        }
        else
        {
            Array<OneD, NekDouble> exactsoln(m_fields[field]->GetNpoints());
            
            EvaluateExactSolution(field,exactsoln,m_time);
            
            return m_fields[field]->Linf(exactsoln);
        }
    }
    
    
    //-------------------------------------------------------------
    // Compute weak Green form of advection terms (without boundary
    // integral, i.e (\grad \phi \cdot F) where for example F = uV
    //
    // Note: Assuming all fields are of the same expansion and order
    // so that we can use the parameters of m_fields[0].
    // ------------------------------------------------------------
  
  void ADRBase::WeakAdvectionGreensDivergenceForm(const Array<OneD, Array<OneD, NekDouble> > &F, Array<OneD, NekDouble> &outarray)
    {
        // use dimension of Velocity vector to dictate dimension of operation
        int ndim    = F.num_elements();
        int nCoeffs = m_fields[0]->GetNcoeffs();

        Array<OneD, NekDouble> iprod(nCoeffs);
        Vmath::Zero(nCoeffs,outarray,1);
        
        for (int i = 0; i < ndim; ++i)
        {
            m_fields[0]->IProductWRTDerivBase(i,F[i],iprod);
            Vmath::Vadd(nCoeffs,iprod,1,outarray,1,outarray,1);
        }
       
    }

    //-------------------------------------------------------------
    // Calculate Inner product of the divergence advection form
    // .....(\phi, Div \cdot F) where for example F = uV
    // -------------------------------------------------------------
    
    void ADRBase::WeakAdvectionDivergenceForm(const Array<OneD, Array<OneD, NekDouble> > &F, Array<OneD, NekDouble> &outarray)
    {
        // use dimension of Velocity vector to dictate dimension of operation
        int ndim       = F.num_elements();
        int nPointsTot = m_fields[0]->GetNpoints();
        Array<OneD, NekDouble> tmp(nPointsTot);
        Array<OneD, NekDouble> div(nPointsTot,0.0);
        
        // Evaluate the divergence 
        for(int i = 0; i < ndim; ++i)
        {
            m_fields[0]->PhysDeriv(i,F[i],tmp);
            Vmath::Vadd(nPointsTot,tmp,1,div,1,div,1);
        }

        m_fields[0]->IProductWRTBase(div,outarray);
    }

    //-------------------------------------------------------------
    // Calculate Inner product of the divergence advection form
    // ..... (\phi, V\cdot Grad(u))
    // -------------------------------------------------------------

    void ADRBase::WeakAdvectionNonConservativeForm(const Array<OneD, Array<OneD, NekDouble> > &V, 
                                                   const Array<OneD, const NekDouble> &u, Array<OneD, NekDouble> &outarray)
    {
        // use dimension of Velocity vector to dictate dimension of operation
        int ndim       = V.num_elements();
        
        
        int nPointsTot = m_fields[0]->GetNpoints();
        Array<OneD, NekDouble> tmp(nPointsTot);
        Array<OneD, NekDouble> wk(ndim*nPointsTot,0.0);

        AdvectionNonConservativeForm(V,u,tmp,wk);
        
        m_fields[0]->IProductWRTBase_IterPerExp(tmp,outarray);
    }

    //-------------------------------------------------------------
    // Calculate  V\cdot Grad(u)
    // -------------------------------------------------------------

    void ADRBase::AdvectionNonConservativeForm(const Array<OneD, Array<OneD, NekDouble> > &V, 
                                               const Array<OneD, const NekDouble> &u, Array<OneD, NekDouble> &outarray, 
					       Array<OneD, NekDouble> &wk)
    {
        // use dimension of Velocity vector to dictate dimension of operation
        int ndim       = V.num_elements();
        //int ndim = m_expdim;
        
        // ToDo: here we should add a check that V has right dimension
      
        int nPointsTot = m_fields[0]->GetNpoints();
        Array<OneD, NekDouble> grad0,grad1,grad2;

        // check to see if wk space is defined
        if(wk.num_elements()) 
        {
            grad0 = Array<OneD, NekDouble> (ndim*nPointsTot);

        }
        else
        {
            grad0 = wk;
        }

        // Evaluate V\cdot Grad(u)
        switch(ndim)
        {
        case 1:
            m_fields[0]->PhysDeriv(u,grad0);
            Vmath::Vmul(nPointsTot,grad0,1,V[0],1,outarray,1);
            break;

        case 2:
            grad1 = grad0 + nPointsTot;
            m_fields[0]->PhysDeriv(u,grad0,grad1);
            Vmath::Vmul (nPointsTot,grad0,1,V[0],1,outarray,1);
            Vmath::Vvtvp(nPointsTot,grad1,1,V[1],1,outarray,1,outarray,1);
            break;

        case 3:
            grad1 = grad0 + nPointsTot;
            grad2 = grad1 + nPointsTot;
            m_fields[0]->PhysDeriv(u,grad0,grad1,grad2);
            Vmath::Vmul (nPointsTot,grad0,1,V[0],1,outarray,1);
            Vmath::Vvtvp(nPointsTot,grad1,1,V[1],1,outarray,1,outarray,1);
            Vmath::Vvtvp(nPointsTot,grad2,1,V[2],1,outarray,1,outarray,1);
            break;

        default:
            ASSERTL0(false,"dimension unknown");
        }
    }
                                       
    //-------------------------------------------------------------
    // Calculate weak DG advection in the form 
    //  <\phi, \hat{F}\cdot n> - (\grad \phi \cdot F)
    // -------------------------------------------------------------
  void ADRBase::WeakDGAdvection(const Array<OneD, Array<OneD, NekDouble> >& InField, 
				Array<OneD, Array<OneD, NekDouble> >& OutField,
				bool NumericalFluxIncludesNormal, bool InFieldIsInPhysSpace, int nvariables)
    {
        int i;
        int nVelDim         = m_spacedim;
        int nPointsTot      = GetNpoints();
        int ncoeffs         = GetNcoeffs();
        int nTracePointsTot = GetTraceNpoints();
	
	if (!nvariables)
	  {
	    nvariables      = m_fields.num_elements();
	  }
	
        Array<OneD, Array<OneD, NekDouble> > fluxvector(nVelDim);
	Array<OneD, Array<OneD, NekDouble> > physfield (nvariables);
        
        for(i = 0; i < nVelDim; ++i)
        {
            fluxvector[i]    = Array<OneD, NekDouble>(nPointsTot);
        }

	
	//--------------------------------------------
	// Get the variables in physical space
	
	// already in physical space
	if(InFieldIsInPhysSpace == true)
	  {
            for(i = 0; i < nvariables; ++i)
	      {
                physfield[i] = InField[i];
	      }
	  }
	// otherwise do a backward transformation
        else
	  {
	    for(i = 0; i < nvariables; ++i)
	      {
		// Could make this point to m_fields[i]->UpdatePhys();
		physfield[i] = Array<OneD, NekDouble>(nPointsTot);
		m_fields[i]->BwdTrans(InField[i],physfield[i]);
	      }
	  }
	//--------------------------------------------

        
	//--------------------------------------------
	// Get the advection part (without numerical flux) 
	
        for(i = 0; i < nvariables; ++i)
        {
            // Get the ith component of the  flux vector in (physical space)
            GetFluxVector(i, physfield, fluxvector);
            
            // Calculate the i^th value of (\grad_i \phi, F)
            WeakAdvectionGreensDivergenceForm(fluxvector,OutField[i]);
        }
	//--------------------------------------------
        

	//----------------------------------------------
	// Get the numerical flux and add to the modal coeffs
	
	// if the NumericalFluxs function already includes the
	// normal in the output
	if (NumericalFluxIncludesNormal == true)
	  {
	    
	    Array<OneD, Array<OneD, NekDouble> > numflux   (nvariables);

	    for(i = 0; i < nvariables; ++i)
	      {
		numflux[i]   = Array<OneD, NekDouble>(nTracePointsTot);
	      }
	    
	    // Evaluate numerical flux in physical space which may in
	    // general couple all component of vectors
	    NumericalFlux(physfield, numflux);
	    
	    // Evaulate  <\phi, \hat{F}\cdot n> - OutField[i] 
	    for(i = 0; i < nvariables; ++i)
	      {
		Vmath::Neg(ncoeffs,OutField[i],1);
		m_fields[i]->AddTraceIntegral(numflux[i],OutField[i]);
	      	m_fields[i]->SetPhysState(false);
	      }
	  }
	// if the NumericalFlux function does not include the
	// normal in the output
	else
	  {
	    Array<OneD, Array<OneD, NekDouble> > numfluxX   (nvariables);
	    Array<OneD, Array<OneD, NekDouble> > numfluxY   (nvariables);
	    
	    for(i = 0; i < nvariables; ++i)
	      {
		numfluxX[i]   = Array<OneD, NekDouble>(nTracePointsTot);
		numfluxY[i]   = Array<OneD, NekDouble>(nTracePointsTot);
	      }

	    // Evaluate numerical flux in physical space which may in
	    // general couple all component of vectors
	    NumericalFlux(physfield, numfluxX, numfluxY);
	    
	    // Evaulate  <\phi, \hat{F}\cdot n> - OutField[i] 
	    for(i = 0; i < nvariables; ++i)
	      {
		Vmath::Neg(ncoeffs,OutField[i],1);
		m_fields[i]->AddTraceIntegral(numfluxX[i],numfluxY[i],OutField[i]);
		m_fields[i]->SetPhysState(false);
	      }
	    
	  }
    }

    //-------------------------------------------------------------
    // Calculate weak DG Diffusion in the LDG form 
    //  <\psi, \hat{u} \cdot n> - (\grad \psi \cdot u)
    //  <\phi, \hat{q}\cdot n> - (\grad \phi \cdot q)
    // -------------------------------------------------------------
  void ADRBase::WeakDGDiffusion(const Array<OneD, Array<OneD, NekDouble> >& InField, 
				  Array<OneD, Array<OneD, NekDouble> >& OutField,
				  bool NumericalFluxIncludesNormal, bool InFieldIsInPhysSpace)
    {
        int i,j,k;
        int nPointsTot      = GetNpoints();
        int ncoeffs         = GetNcoeffs();
        int nTracePointsTot = GetTraceNpoints();
        int nvariables = m_fields.num_elements();
        int nqvar = 2;

	Array<OneD, NekDouble>  qcoeffs (ncoeffs);
	Array<OneD, NekDouble>  temp (ncoeffs);

	Array<OneD, Array<OneD, NekDouble> > fluxvector (m_spacedim);
	Array<OneD, Array<OneD, NekDouble> > ufield (nvariables);

	Array<OneD, Array<OneD, Array<OneD, NekDouble> > >  flux   (nqvar);
	Array<OneD, Array<OneD, Array<OneD, NekDouble> > >  qfield  (nqvar);
				  
	for(j = 0; j < nqvar; ++j)
	    {
                qfield[j] = Array<OneD, Array<OneD, NekDouble> >(nqvar);
                flux[j] = Array<OneD, Array<OneD, NekDouble> >(nqvar);

                for(i = 0; i< nvariables; ++i)
		{
                    ufield[i] = Array<OneD, NekDouble>(nPointsTot,0.0);
                    
                    qfield[j][i]  = Array<OneD, NekDouble>(nPointsTot,0.0);
                    flux[j][i] = Array<OneD, NekDouble>(nTracePointsTot,0.0);
		}      
	    }
        
        for(k = 0; k < m_spacedim; ++k)
        {
            fluxvector[k] = Array<OneD, NekDouble>(nPointsTot,0.0);
        }
	
	//--------------------------------------------
	// Get the variables in physical space
	
	// already in physical space
	if(InFieldIsInPhysSpace == true)
	  {
            for(i = 0; i < nvariables; ++i)
	      {
                ufield[i] = InField[i];
	      }
	  }
	// otherwise do a backward transformation
        else
	  {
	    for(i = 0; i < nvariables; ++i)
	      {
                  // Could make this point to m_fields[i]->UpdatePhys();
                  ufield[i] = Array<OneD, NekDouble>(nPointsTot);
                  m_fields[i]->BwdTrans(InField[i],ufield[i]);
	      }
	  }
        
        // ########################################################################
        //   Compute q_{\eta} and q_{\xi} from su
        // ########################################################################

        // Obtain Numerical Fluxes        
        NumFluxforScalar(ufield, flux);

        for(j = 0; j < nqvar; ++j)
        {        
            for(i = 0; i < nvariables; ++i)
            {
                // Get the ith component of the  flux vector in (physical space)
                // fluxvector = m_tanbasis * u, where m_tanbasis = 2 by m_spacedim by nPointsTot
	      if(m_tanbasis.num_elements())
		{
		  for (k = 0; k < m_spacedim; ++k)
		    {
		      Vmath::Vmul(nPointsTot, m_tanbasis[j][k], 1, ufield[i], 1, fluxvector[k], 1);
		    }
		}

	      else
		{
                    GetFluxVector(i, j, ufield, fluxvector);
		}
                
                // Calculate the i^th value of (\grad_i \phi, F)
                WeakAdvectionGreensDivergenceForm(fluxvector, qcoeffs);
                
                Vmath::Neg(ncoeffs,qcoeffs,1);
                m_fields[i]->AddTraceIntegral(flux[j][i], qcoeffs);
                m_fields[i]->SetPhysState(false);
  
                // Add weighted mass matrix = M ( \nabla \cdot Tanbasis )
		if(m_gradtan.num_elements())
		  {
		    MultiRegions::GlobalMatrixKey key(StdRegions::eMass,m_gradtan[j]);
		    m_fields[i]->MultiRegions::ExpList::GeneralMatrixOp(key, InField[i], temp);
		    Vmath::Svtvp(ncoeffs, -1.0, temp, 1, qcoeffs, 1, qcoeffs, 1);
		  }

                 //Multiply by the inverse of mass matrix
                m_fields[i]->MultiplyByElmtInvMass(qcoeffs, qcoeffs);
		
                // Back to physical space
                m_fields[i]->BwdTrans(qcoeffs, qfield[j][i]);
            }
        }
	

        // ########################################################################
        //   Compute u from q_{\eta} and q_{\xi}
        // ########################################################################

        // Obtain Numerical Fluxes
	NumFluxforVector(ufield, qfield, flux[0]);

	for (i = 0; i < nvariables; ++i)
        {
            // L = L(tan_eta) q_eta + L(tan_xi) q_xi
            OutField[i] = Array<OneD, NekDouble>(ncoeffs, 0.0);
            temp = Array<OneD, NekDouble>(ncoeffs, 0.0);

	    if(m_tanbasis.num_elements())
	      {
		for(j = 0; j < nqvar; ++j)
		  {
		    for (k = 0; k < m_spacedim; ++k)
		      {
			Vmath::Vmul(nPointsTot, m_tanbasis[j][k], 1, qfield[j][i], 1, fluxvector[k], 1);
		      }
		    
		    WeakAdvectionGreensDivergenceForm(fluxvector, temp);
		    Vmath::Vadd(ncoeffs, temp, 1, OutField[i], 1, OutField[i], 1);
		  }
	      }

	      else
		{
		  for (k = 0; k < m_spacedim; ++k)
		    {
		      Vmath::Vcopy(nPointsTot, qfield[k][i], 1, fluxvector[k], 1);
		    }

		    WeakAdvectionGreensDivergenceForm(fluxvector, OutField[i]);
		}
            
            // Evaulate  <\phi, \hat{F}\cdot n> - OutField[i] 
            Vmath::Neg(ncoeffs,OutField[i],1);
            m_fields[i]->AddTraceIntegral(flux[0][i], OutField[i]);
            m_fields[i]->SetPhysState(false);
	}
    }
  
    void ADRBase::Output(void)
    {
        std::string outname = m_sessionName + ".fld";
        WriteFld(outname);
    }

    void ADRBase::Checkpoint_Output(const int n)
    {
        char chkout[16] = "";
        sprintf(chkout, "%d", n);
        std::string outname = m_sessionName +"_" + chkout + ".chk";
        WriteFld(outname);
    }

  void ADRBase::WriteFld(std::string &outname)
    {
        std::vector<SpatialDomains::FieldDefinitionsSharedPtr> FieldDef = m_fields[0]->GetFieldDefinitions();
        std::vector<std::vector<NekDouble> > FieldData(FieldDef.size()); 

        // copy Data into FieldData and set variable
        for(int j = 0; j < m_fields.num_elements(); ++j)
        {
            for(int i = 0; i < FieldDef.size(); ++i)
            {
                // Could do a search here to find correct variable
                FieldDef[i]->m_Fields.push_back(m_boundaryConditions->GetVariable(j));
                m_fields[j]->AppendFieldData(FieldDef[i], FieldData[i]);
            }
        }

        m_graph->Write(outname,FieldDef,FieldData);
    }

    /** \brief Import field from infile and load into \a
        m_fields. This routine will also perform a \a BwdTrans to
        ensure data is in both the physical and coefficient storage.
    **/
    void ADRBase::ImportFld(std::string &infile)
    {
        std::vector<SpatialDomains::FieldDefinitionsSharedPtr> FieldDef;
        std::vector<std::vector<NekDouble> > FieldData; 

        m_graph->Import(infile,FieldDef,FieldData);

        // copy FieldData into m_fields 
        for(int j = 0; j < m_fields.num_elements(); ++j)
        {
            for(int i = 0; i < FieldDef.size(); ++i)
            {
                ASSERTL1(FieldDef[i]->m_fields[j] == m_boundaryConditions->GetVariable(j),(std::string("Order of ")+infile+std::string(" data and that defined in m_boundaryconditions differs")).c_str());

                m_fields[j]->ExtractDataToCoeffs(FieldDef[i],FieldData[i],FieldDef[i]->m_Fields[j]);
            }
            m_fields[j]->BwdTrans(m_fields[j]->GetCoeffs(),m_fields[j]->UpdatePhys());
        }
    }

  void ADRBase::Array_Output(const int n, std::string name, const Array<OneD, const NekDouble>&inarray, bool IsInPhysicalSpace)
  {
    
    int nq = m_fields[0]->GetTotPoints();
    
    Array<OneD, NekDouble> tmp(nq);

    // save values 
    Vmath::Vcopy(nq,(m_fields[0]->GetPhys()),1,tmp,1);
    
    // put inarray in m_phys
    if (IsInPhysicalSpace == false)
      {
	m_fields[0]->BwdTrans(inarray,(m_fields[0]->UpdatePhys()));
      }
    else
      {
	Vmath::Vcopy(nq,inarray,1,(m_fields[0]->UpdatePhys()),1);	
      }
	
    char chkout[16] = "";
    sprintf(chkout, "%d", n);
    std::string outname = m_sessionName +"_" + name + "_" + chkout + ".chk";
    ofstream outfile(outname.c_str());
    m_fields[0]->WriteToFile(outfile,eTecplot);
    
    // copy back the original values
    Vmath::Vcopy(nq,tmp,1,m_fields[0]->UpdatePhys(),1);
  }


    void ADRBase::Summary(std::ostream &out)
    {
        SessionSummary(out);
        TimeParamSummary(out);
    }
    
    void ADRBase::SessionSummary(std::ostream &out)
    {

        out << "\tSession Name    : " << m_sessionName << endl;
	out << "\tExpansion Dim.  : " << m_expdim << endl;
	out << "\tSpatial   Dim.  : " << m_spacedim << endl;
        out << "\tMax Exp. Order  : " << m_fields[0]->EvalBasisNumModesMax() << endl;
        if(m_projectionType == eGalerkin)
        {
            out << "\tProjection Type : Galerkin" <<endl;
        }
        else
        {
            out << "\tProjection Type : Discontinuous Galerkin" <<endl;
        }
    }


    void ADRBase::TimeParamSummary(std::ostream &out)
    {
        out << "\tTime Step       : " << m_timestep << endl;
        out << "\tNo. of Steps    : " << m_steps << endl;
        out << "\tCheckpoints     : " << m_checksteps <<" steps" <<endl;
    }

  
    // case insensitive string comparison from web
    // Return 0 when they are the same. 
  int ADRBase::NoCaseStringCompare(const string & s1, const string& s2) 
  {
    string::const_iterator it1=s1.begin();
    string::const_iterator it2=s2.begin();
    
    //stop when either string's end has been reached
    while ( (it1!=s1.end()) && (it2!=s2.end()) ) 
      { 
	if(::toupper(*it1) != ::toupper(*it2)) //letters differ?
	  {
	    // return -1 to indicate smaller than, 1 otherwise
	    return (::toupper(*it1)  < ::toupper(*it2)) ? -1 : 1; 
	  }
	//proceed to the next character in each string
	++it1;
	++it2;
      }
    size_t size1=s1.size(), size2=s2.size();// cache lengths
    
    //return -1,0 or 1 according to strings' lengths
    if (size1==size2) 
    {
	return 0;
    }
    return (size1 < size2) ? -1 : 1;
  }  

} //end of namespace

/**
* $Log: ADRBase.cpp,v $
* Revision 1.18  2009/09/07 11:21:58  sherwin
* Updates related to Navier-Stokes solver
*
* Revision 1.17  2009/08/14 09:30:01  cbiotto
* Add WriteVar function
*
* Revision 1.16  2009/07/29 09:19:42  sehunchun
* Generalization of WeakDGDiffusion
*
* Revision 1.15  2009/07/23 05:23:21  sehunchun
* WeakDiffusion operator is updated
*
* Revision 1.14  2009/07/11 23:39:23  sehunchun
* Move uncommon functions to each solvers
*
* Revision 1.13  2009/07/09 21:29:13  sehunchun
* Add SetUpSurfaceNormal function..
*
* Revision 1.12  2009/07/02 15:57:36  sehunchun
* "ReadBoundaryCondition" options with extenstion to 2D geometry imbedded in 3D
*
* Revision 1.11  2009/07/01 21:55:00  sehunchun
* Changes of WeakDiffusion according to updates
*
* Revision 1.10  2009/06/11 01:54:08  claes
* Added Inviscid Burger
*
* Revision 1.9  2009/04/29 20:45:55  sherwin
* Update for new definition of enum
*
* Revision 1.8  2009/04/27 21:37:14  sherwin
* Updated to dump .fld and .chk file in compressed coefficient format
*
* Revision 1.7  2009/03/10 23:37:14  claes
* Updated the ShallowWaterSolver to work with the general timestepping scheme
*
* Revision 1.6  2009/03/04 14:17:38  pvos
* Removed all methods that take and Expansion as argument
*
* Revision 1.5  2009/02/28 22:00:38  sehunchun
*  Explicit Diffusion solver is added
*
* Revision 1.4  2009/02/03 14:33:44  pvos
* Modifications for solvers with time-dependent dirichlet BC's
*
* Revision 1.3  2009/02/02 16:10:16  claes
* Update to make SWE, Euler and Boussinesq solvers up to date with the time integrator scheme. Linear and classical Boussinsq solver working
*
* Revision 1.2  2009/01/27 12:07:18  pvos
* Modifications to make cont. Galerkin Advection solver working
*
* Revision 1.1  2009/01/13 10:59:32  pvos
* added new solvers file
*
* Revision 1.8  2009/01/10 23:50:32  sherwin
* Update ContField1D/2D to use different variable in constructors
*
* Revision 1.7  2009/01/06 21:11:03  sherwin
* Updates for Virtual ExpList calls
*
* Revision 1.6  2008/11/17 08:10:07  claes
* Removed functions that were no longer used after the solver library was restructured
*
* Revision 1.5  2008/11/02 22:39:27  sherwin
* Updated naming convention
*
* Revision 1.4  2008/10/31 10:50:10  pvos
* Restructured directory and CMakeFiles
*
* Revision 1.3  2008/10/29 22:51:07  sherwin
* Updates for const correctness and ODEforcing
*
* Revision 1.2  2008/10/19 15:59:20  sherwin
* Added Summary method
*
* Revision 1.1  2008/10/16 15:25:45  sherwin
* Working verion of restructured AdvectionDiffusionReactionSolver
*
**/
