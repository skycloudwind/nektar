///////////////////////////////////////////////////////////////////////////////
//
// File LinearisedAdvection.cpp
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
// Description: Evaluation of the linearised advective term
//
///////////////////////////////////////////////////////////////////////////////

#include <IncNavierStokesSolver/AdvectionTerms/LinearisedAdvection.h>
#include <cstdio>
#include <cstdlib>
#include <stdio.h>

namespace Nektar
{
    string LinearisedAdvection::className = GetAdvectionTermFactory().RegisterCreatorFunction("Linearised", LinearisedAdvection::create);

    /**
     * Constructor. Creates ...
     *
     * \param 
     * \param
     */

    LinearisedAdvection::LinearisedAdvection(
            const LibUtilities::SessionReaderSharedPtr&        pSession,
            const SpatialDomains::MeshGraphSharedPtr&          pGraph):
        AdvectionTerm(pSession, pGraph)
	{
	}
	

	void LinearisedAdvection::v_InitObject()
	{
	    AdvectionTerm::v_InitObject();
		
		m_boundaryConditions = MemoryManager<SpatialDomains::BoundaryConditions>
		::AllocateSharedPtr(m_session, m_graph);
		
		//Setting parameters for homogeneous problems
		m_HomoDirec       = 0;
        m_useFFT          = false;
        m_HomogeneousType = eNotHomogeneous;
		
        if(m_session->DefinesSolverInfo("HOMOGENEOUS"))
        {
            std::string HomoStr = m_session->GetSolverInfo("HOMOGENEOUS");
            m_spacedim          = 3;
			
            if((HomoStr == "HOMOGENEOUS1D")||(HomoStr == "Homogeneous1D")||
               (HomoStr == "1D")||(HomoStr == "Homo1D"))
            {
                m_HomogeneousType = eHomogeneous1D;
               // m_npointsZ        = m_session->GetParameter("HomModesZ");
                m_LhomZ           = m_session->GetParameter("LZ");
                m_HomoDirec       = 1;
				m_SingleMode	   =false;

				if(m_session->DefinesSolverInfo("SingleMode"))
				{
					if(m_session->GetSolverInfo("SingleMode")=="SpecifiedMode")
					{
						m_SingleMode=true;
						if(m_session->DefinesParameter("NumMode"))
						{
							//read mode from session file
							m_NumMode=m_session->GetParameter("NumMode");
						}
						else 
						{
							//first mode by default
							m_NumMode=1;
						}
						
						//number of plane to create in case of single modes analysis.
						m_npointsZ=2+2*m_NumMode;
						
					}
					else if(m_session->GetSolverInfo("SingleMode")=="ModifiedBasis") 
					{
						m_npointsZ=2;

					}
					else 
					{
						ASSERTL0(false, "SolverInfo Single Mode not valid");	
					}
					
					
				}
				else 
				{
					m_session->LoadParameter("HomModesZ",m_npointsZ);

				}

            }
			
            if((HomoStr == "HOMOGENEOUS2D")||(HomoStr == "Homogeneous2D")||
               (HomoStr == "2D")||(HomoStr == "Homo2D"))
            {
                m_HomogeneousType = eHomogeneous2D;
                m_session->LoadParameter("HomModesY", m_npointsY);
                m_session->LoadParameter("LY",        m_LhomY);
                m_session->LoadParameter("HomModesZ", m_npointsZ);
                m_session->LoadParameter("LZ",        m_LhomZ);
                m_HomoDirec       = 2;
            }
			
            if((HomoStr == "HOMOGENEOUS3D")||(HomoStr == "Homogeneous3D")||
               (HomoStr == "3D")||(HomoStr == "Homo3D"))
            {
                m_HomogeneousType = eHomogeneous3D;
                m_session->LoadParameter("HomModesX",m_npointsX);
                m_session->LoadParameter("LX",       m_LhomX   );
                m_session->LoadParameter("HomModesY",m_npointsY);
                m_session->LoadParameter("LY",       m_LhomY   );
                m_session->LoadParameter("HomModesZ",m_npointsZ);
                m_session->LoadParameter("LZ",       m_LhomZ   );
                m_HomoDirec       = 3;
            }
			
            if(m_session->DefinesSolverInfo("USEFFT"))
            {
                m_useFFT = true;
            }
        }
        else
        {
            m_npointsZ = 1; // set to default value so can use to identify 2d or 3D (homogeneous) expansions
        }
		
		if(m_session->DefinesSolverInfo("PROJECTION"))
        {
            std::string ProjectStr
			= m_session->GetSolverInfo("PROJECTION");
			
            if((ProjectStr == "Continuous")||(ProjectStr == "Galerkin")||
               (ProjectStr == "CONTINUOUS")||(ProjectStr == "GALERKIN"))
            {
                m_projectionType = MultiRegions::eGalerkin;
            }
            else if(ProjectStr == "DisContinuous")
            {
                m_projectionType = MultiRegions::eDiscontinuousGalerkin;
            }
            else
            {
                ASSERTL0(false,"PROJECTION value not recognised");
            }
        }
        else
        {
            cerr << "Projection type not specified in SOLVERINFO,"
			"defaulting to continuous Galerkin" << endl;
            m_projectionType = MultiRegions::eGalerkin;
        }
		
            SetUpBaseFields(m_graph);
            ASSERTL0(m_session->DefinesFunction("BaseFlow"),
                    "Base flow must be defined for linearised forms.");
            string file = m_session->GetFunctionFilename("BaseFlow");
		
		
		//Periodic base flows
		if(m_session->DefinesParameter("N_slices"))
		{
            m_session->LoadParameter("N_slices",m_slices);
            if(m_slices>1)
            {

				int npoints=m_base[0]->GetTotPoints();
				Array<OneD, NekDouble> fft_in(npoints*m_slices);
				Array<OneD, NekDouble> fft_out(npoints*m_slices);
				
				Array<OneD, NekDouble> m_tmpIN(m_slices);
				Array<OneD, NekDouble> m_tmpOUT(m_slices);

				//Convected fields
				int ConvectedFields=m_base.num_elements()-1;
				
				m_interp= Array<OneD, Array<OneD, NekDouble> > (ConvectedFields);
				for(int i=0; i<ConvectedFields;++i)
				{
					m_interp[i]=Array<OneD,NekDouble>(npoints*m_slices);
				}
				
				//Import the slides into auxiliary vector
				//The base flow should be stored in the form filename_i.bse
				for (int i=0; i< m_slices; ++i)
				{
					char chkout[16] = "";
					sprintf(chkout, "%d", i);
					ImportFldBase(file+"_"+chkout+".bse",m_graph,i);
				} 
				
				m_useFFTW=false;
				if(m_session->DefinesSolverInfo("USEFFT"))
				{
					m_useFFTW = true;
				}
				
				//Factory for FFT transformation
				if(m_useFFTW)
				{
					m_FFT = LibUtilities::GetNektarFFTFactory().CreateInstance("NekFFTW", m_slices);
				}
				else 
				{
					ASSERTL0(false, "Time interpolation not implemented");
				}
				
				// Discrete Fourier Transform of the fields
				for(int k=0; k< ConvectedFields;++k)
				{
					//Shuffle the data
					for(int j= 0; j < m_slices; ++j)
					{
						Vmath::Vcopy(npoints,&m_interp[k][j*npoints],1,&(fft_in[j]),m_slices);
					}
					
					//FFT Transform
					for(int i=0; i<npoints; i++)
					{
						m_FFT->FFTFwdTrans(m_tmpIN =fft_in + i*m_slices, m_tmpOUT =fft_out + i*m_slices);
					}
					
					//Reshuffle data
					for(int s = 0; s < m_slices; ++s)
					{						
						Vmath::Vcopy(npoints,&fft_out[s],m_slices,&m_interp[k][s*npoints],1);
						
					}
					
					for(int r=0; r<fft_in.num_elements();++r)
					{
						fft_in[0]=0;
						fft_out[0]=0;
					}
					
				}
				
				if(m_session->DefinesParameter("period"))
				{
					m_period=m_session->GetParameter("period");
				}
				else 
				{
					m_period=(m_session->GetParameter("TimeStep")*m_slices)/(m_slices-1.);
				}
			}
			else{
			
				ASSERTL0(false,"Number of slices must be a positive number");
			    }
			}
			//Steady base-flow
			else
			{
				m_slices=1;

				//BaseFlow from file
				if (m_session->GetFunctionType("BaseFlow")
					== LibUtilities::eFunctionTypeFile)
			    {
					ImportFldBase(file,m_graph,1);
					
				}
				//analytic base flow
				else
				{
					int nq = m_base[0]->GetNpoints();
					Array<OneD,NekDouble> x0(nq);
					Array<OneD,NekDouble> x1(nq);
					Array<OneD,NekDouble> x2(nq);
					
					// get the coordinates (assuming all fields have the same
					// discretisation)
					m_base[0]->GetCoords(x0,x1,x2);
					for(unsigned int i = 0 ; i < m_base.num_elements(); i++)
					{
						LibUtilities::EquationSharedPtr ifunc
                        = m_session->GetFunction("BaseFlow", i);

                        ifunc->Evaluate(x0,x1,x2,m_base[i]->UpdatePhys());

						m_base[i]->SetPhysState(true);						
						m_base[i]->FwdTrans_IterPerExp(m_base[i]->GetPhys(),
														 m_base[i]->UpdateCoeffs());
					}
					
				}
					
			}
				
	}

    LinearisedAdvection::~LinearisedAdvection()
    {
    }

    
    void LinearisedAdvection::SetUpBaseFields(SpatialDomains::MeshGraphSharedPtr &mesh)
    {
        int nvariables = m_session->GetVariables().size();
        int i;
        m_base = Array<OneD, MultiRegions::ExpListSharedPtr>(nvariables);
        
        if (m_projectionType == MultiRegions::eGalerkin)
        {
            switch (m_expdim)
            {
				case 1:
				{
					if(m_HomogeneousType == eHomogeneous2D)
					{
						const LibUtilities::PointsKey PkeyY(m_npointsY,LibUtilities::eFourierEvenlySpaced);
						const LibUtilities::BasisKey  BkeyY(LibUtilities::eFourier,m_npointsY,PkeyY);
						const LibUtilities::PointsKey PkeyZ(m_npointsZ,LibUtilities::eFourierEvenlySpaced);
						const LibUtilities::BasisKey  BkeyZ(LibUtilities::eFourier,m_npointsZ,PkeyZ);
						
						for(i = 0 ; i < m_base.num_elements(); i++)
						{
							m_base[i] = MemoryManager<MultiRegions::ContField3DHomogeneous2D>
							::AllocateSharedPtr(m_session,BkeyY,BkeyZ,m_LhomY,m_LhomZ,m_useFFT,m_dealiasing,m_graph,m_session->GetVariable(i));
						}
					}
					
					else {
						
						for(i = 0 ; i < m_base.num_elements(); i++)
						{
							m_base[i] = MemoryManager<MultiRegions::ContField1D>
							::AllocateSharedPtr(m_session,mesh,
												m_session->GetVariable(i));
						}
					}
					
				}
					break;
				case 2:
				{
					if(m_HomogeneousType == eHomogeneous1D)
					{

						if(m_session->DefinesSolverInfo("SingleMode")&& m_session->GetSolverInfo("SingleMode")=="ModifiedBasis")
						{
							const LibUtilities::PointsKey PkeyZ(m_npointsZ,LibUtilities::eFourierSingleModeSpaced);
							const LibUtilities::BasisKey  BkeyZ(LibUtilities::eFourierSingleMode,m_npointsZ,PkeyZ);
							
							for(i = 0 ; i < m_base.num_elements(); i++)
							{								
								m_base[i] = MemoryManager<MultiRegions::ContField3DHomogeneous1D>
								::AllocateSharedPtr(m_session,BkeyZ,m_LhomZ,m_useFFT,m_dealiasing,m_graph,m_session->GetVariable(i));
								
							} 
						}
						else 
						{
							const LibUtilities::PointsKey PkeyZ(m_npointsZ,LibUtilities::eFourierEvenlySpaced);
							const LibUtilities::BasisKey  BkeyZ(LibUtilities::eFourier,m_npointsZ,PkeyZ);
							
							
							for(i = 0 ; i < m_base.num_elements(); i++)
							{
								m_base[i] = MemoryManager<MultiRegions::ContField3DHomogeneous1D>
								::AllocateSharedPtr(m_session,BkeyZ,m_LhomZ,m_useFFT,m_dealiasing,m_graph,m_session->GetVariable(i));
								m_base[i]->SetWaveSpace(false);
							} 
							
						}
					}
					else
					{
						i = 0;
						MultiRegions::ContField2DSharedPtr firstbase =
                        MemoryManager<MultiRegions::ContField2D>
                        ::AllocateSharedPtr(m_session,mesh,
                                            m_session->GetVariable(i));
						m_base[0]=firstbase;
						
						for(i = 1 ; i < m_base.num_elements(); i++)
						{
							m_base[i] = MemoryManager<MultiRegions::ContField2D>
                            ::AllocateSharedPtr(*firstbase,mesh,
                                                m_session->GetVariable(i));
						}
					}
				}
					break;
				case 3:
				{
					if(m_HomogeneousType == eHomogeneous3D)
					{
						ASSERTL0(false,"3D fully periodic problems not implemented yet");
					}
					else
					{
						MultiRegions::ContField3DSharedPtr firstbase =
						MemoryManager<MultiRegions::ContField3D>
						::AllocateSharedPtr(m_session,mesh,
											m_session->GetVariable(i));
						m_base[0] = firstbase;
						
						for(i = 1 ; i < m_base.num_elements(); i++)
						{
							m_base[i] = MemoryManager<MultiRegions::ContField3D>
							::AllocateSharedPtr(*firstbase,mesh,
												m_session->GetVariable(i));
						}
					}	        
				}
					break;
				default:
					ASSERTL0(false,"Expansion dimension not recognised");
					break;
            }
        }
        else
        {
            switch(m_expdim)
            {
				case 1:
                {
					if(m_HomogeneousType == eHomogeneous2D)
                    {
                        const LibUtilities::PointsKey PkeyY(m_npointsY,LibUtilities::eFourierEvenlySpaced);
                        const LibUtilities::BasisKey  BkeyY(LibUtilities::eFourier,m_npointsY,PkeyY);
                        const LibUtilities::PointsKey PkeyZ(m_npointsZ,LibUtilities::eFourierEvenlySpaced);
                        const LibUtilities::BasisKey  BkeyZ(LibUtilities::eFourier,m_npointsZ,PkeyZ);
						
                        for(i = 0 ; i < m_base.num_elements(); i++)
                        {
                            m_base[i] = MemoryManager<MultiRegions::DisContField3DHomogeneous2D>
							::AllocateSharedPtr(m_session,BkeyY,BkeyZ,m_LhomY,m_LhomZ,m_useFFT,m_dealiasing,m_graph,m_session->GetVariable(i));
                        }
                    }
					else 
					{
						for(i = 0 ; i < m_base.num_elements(); i++)
						{
							m_base[i] = MemoryManager<MultiRegions
                            ::DisContField1D>::AllocateSharedPtr(m_session,mesh,
                                                                 m_session->GetVariable(i));
						}
					}
                    break;
                }
				case 2:
                {
					if(m_HomogeneousType == eHomogeneous1D)
                    {
						const LibUtilities::PointsKey PkeyZ(m_npointsZ,LibUtilities::eFourierEvenlySpaced);
						const LibUtilities::BasisKey  BkeyZ(LibUtilities::eFourier,m_npointsZ,PkeyZ);
						
						for(i = 0 ; i < m_base.num_elements(); i++)
						{
							m_base[i] = MemoryManager<MultiRegions::DisContField3DHomogeneous1D>
							::AllocateSharedPtr(m_session,BkeyZ,m_LhomZ,m_useFFT,m_dealiasing,m_graph,m_session->GetVariable(i));
						}
						
						
						
					}
					else
					{
						for(i = 0 ; i < m_base.num_elements(); i++)
						{
							m_base[i] = MemoryManager<MultiRegions
							::DisContField2D>::AllocateSharedPtr(m_session, mesh,
                                                                 m_session->GetVariable(i));
						}
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
        
    }
    
    /**
     * Import field from infile and load into \a m_fields. This routine will
     * also perform a \a BwdTrans to ensure data is in both the physical and
     * coefficient storage.
     * @param   infile          Filename to read.
     */
    void LinearisedAdvection::ImportFldBase(std::string pInfile,
            SpatialDomains::MeshGraphSharedPtr pGraph, int cnt)
    {
        std::vector<SpatialDomains::FieldDefinitionsSharedPtr> FieldDef;
        std::vector<std::vector<NekDouble> > FieldData;
		int numfields=m_base.num_elements();
		int nqtot = m_base[0]->GetTotPoints();

		//Get Homogeneous


        pGraph->Import(pInfile,FieldDef,FieldData);

        int nvar = m_session->GetVariables().size();
		int s;
		
		if(m_session->DefinesSolverInfo("HOMOGENEOUS"))
		{
			std::string HomoStr = m_session->GetSolverInfo("HOMOGENEOUS");
		}
		
        // copy FieldData into m_fields
        for(int j = 0; j < nvar; ++j)
        {
            for(int i = 0; i < FieldDef.size(); ++i)
            {

					
				if(m_session->DefinesSolverInfo("HOMOGENEOUS") && (m_session->GetSolverInfo("HOMOGENEOUS")=="HOMOGENEOUS1D"||
				m_session->GetSolverInfo("HOMOGENEOUS")=="1D"||m_session->GetSolverInfo("HOMOGENEOUS")=="Homo1D"))
				{
					// w-component must be ignored and set to zero.
					if(j!=nvar-2)
					{
						// p component (it is 4th variable of the 3D and corresponds 3nd variable of 2D)
						if(j==nvar-1)
						{
							s=2;
						}
						else 
						{
							s=j;	
						}
						
						//extraction of the 2D
						m_base[j]->ExtractDataToCoeffs(FieldDef[i], FieldData[i],
													   FieldDef[i]->m_fields[s],true);
						
						//Put zero on higher modes
						int ncplane=(m_base[0]->GetNcoeffs())/m_npointsZ;
						if(m_npointsZ>2)
						{
							Vmath::Zero(ncplane*(m_npointsZ-2),&m_base[j]->UpdateCoeffs()[2*ncplane],1);
						}
						
							
						
					}
				}
				//2D cases
				else
				{
					bool flag = FieldDef[i]->m_fields[j]
							  == m_session->GetVariable(j);
					ASSERTL1(flag, (std::string("Order of ") + pInfile
                                + std::string(" data and that defined in "
                                             "m_boundaryconditions differs")).c_str());
                
					m_base[j]->ExtractDataToCoeffs(FieldDef[i], FieldData[i],
                                                   FieldDef[i]->m_fields[j]);
				}
            }
			
			//In case ModifiedBasis it is used 
			if(m_session->DefinesSolverInfo("SingleMode") && m_session->GetSolverInfo("SingleMode")=="ModifiedBasis")
			{
				m_base[j]->SetWaveSpace(true);
			
				m_base[j]->BwdTrans(m_base[j]->GetCoeffs(),
									m_base[j]->UpdatePhys());

							
				//copy the bwd into the second plane for single Mode Analysis
			    int ncplane=(m_base[0]->GetNpoints())/m_npointsZ;
				Vmath::Vcopy(ncplane,&m_base[j]->GetPhys()[0],1,&m_base[j]->UpdatePhys()[ncplane],1);
			}
			else
			{
				m_base[j]->BwdTrans(m_base[j]->GetCoeffs(),
									m_base[j]->UpdatePhys());
				
			}
			
			FILE *pFile3;
			pFile3= fopen("Base_U.txt", "w");
			for(int s=0; s< m_base[0]->GetNpoints(); ++s)
			{
				fprintf(pFile3, "base[%i]= %10.20lf\t  \n",s,m_base[0]->GetPhys()[s]); 
			}
			fclose(pFile3);
			
        }
		
		//@ToDo: Post Processing for ModifiedBase
		//std::string outname ="BaseFlow.bse";
		//WriteFldBase(outname);
		
		if(m_session->DefinesParameter("N_slices"))
		{
			
			for(int i=0; i<m_nConvectiveFields;++i)
			{
				
				Vmath::Vcopy(nqtot, &m_base[i]->GetPhys()[0], 1, &m_interp[i][cnt*nqtot], 1);				
			}
			
		}
    }
        
   
    //Evaluation of the advective terms
    void LinearisedAdvection::v_ComputeAdvectionTerm(
            Array<OneD, MultiRegions::ExpListSharedPtr > &pFields,
            const Array<OneD, Array<OneD, NekDouble> > &pVelocity,
            const Array<OneD, const NekDouble> &pU,
            Array<OneD, NekDouble> &pOutarray,
            int pVelocityComponent,
			NekDouble m_time,
            Array<OneD, NekDouble> &pWk)
    {
        int ndim       = m_nConvectiveFields;
        int nPointsTot = pFields[0]->GetNpoints();
        Array<OneD, NekDouble> grad0,grad1,grad2;
	
		
        //Evaluation of the gradiend of each component of the base flow
        //\nabla U
        Array<OneD, NekDouble> grad_base_u0,grad_base_u1,grad_base_u2;
        // \nabla V
        Array<OneD, NekDouble> grad_base_v0,grad_base_v1,grad_base_v2;
        // \nabla W
        Array<OneD, NekDouble> grad_base_w0,grad_base_w1,grad_base_w2;
	
        
        grad0 = Array<OneD, NekDouble> (nPointsTot);
        grad_base_u0 = Array<OneD, NekDouble> (nPointsTot);
        grad_base_v0 = Array<OneD, NekDouble> (nPointsTot);
        grad_base_w0 = Array<OneD, NekDouble> (nPointsTot);		
		
		//Evaluation of the base flow for periodic cases
		//(it requires fld files)
			
			if(m_slices>1)
			{				
				if (m_session->GetFunctionType("BaseFlow")
					== LibUtilities::eFunctionTypeFile)
				{
					for(int i=0; i<m_nConvectiveFields;++i)
					{
						UpdateBase(m_slices,m_interp[i],m_base[i]->UpdatePhys(),m_time,m_period);
					}
				}
				else 
				{
					ASSERTL0(false, "Periodic Base flow requires .fld files");	
				}
			}
		

		
				
		//Evaluate the linearised advection term
        switch(ndim) 
        {
            // 1D
        case 1:
            pFields[0]->PhysDeriv(pVelocity[pVelocityComponent],grad0);
            pFields[0]->PhysDeriv(m_base[0]->GetPhys(),grad_base_u0);
            //Evaluate  U du'/dx
            Vmath::Vmul(nPointsTot,grad0,1,m_base[0]->GetPhys(),1,pOutarray,1);
            //Evaluate U du'/dx+ u' dU/dx
            Vmath::Vvtvp(nPointsTot,grad_base_u0,1,pVelocity[0],1,pOutarray,1,pOutarray,1);
            break;
            
            //2D
        case 2:
            
			grad1 = Array<OneD, NekDouble> (nPointsTot);
			grad_base_u1 = Array<OneD, NekDouble> (nPointsTot);
			grad_base_v1 = Array<OneD, NekDouble> (nPointsTot);
				
            pFields[0]->PhysDeriv(pVelocity[pVelocityComponent],grad0,grad1);

            //Derivates of the base flow
            pFields[0]-> PhysDeriv(m_base[0]->GetPhys(), grad_base_u0, grad_base_u1);
            pFields[0]-> PhysDeriv(m_base[1]->GetPhys(), grad_base_v0, grad_base_v1);
            
            //Since the components of the velocity are passed one by one, it is necessary to distinguish which
            //term is consider
            switch (pVelocityComponent)
            {
                //x-equation
            case 0:
                // Evaluate U du'/dx
                Vmath::Vmul (nPointsTot,grad0,1,m_base[0]->GetPhys(),1,pOutarray,1);
                //Evaluate U du'/dx+ V du'/dy
                Vmath::Vvtvp(nPointsTot,grad1,1,m_base[1]->GetPhys(),1,pOutarray,1,pOutarray,1);
                //Evaluate (U du'/dx+ V du'/dy)+u' dU/dx
                Vmath::Vvtvp(nPointsTot,grad_base_u0,1,pVelocity[0],1,pOutarray,1,pOutarray,1);
                //Evaluate (U du'/dx+ V du'/dy +u' dU/dx)+v' dU/dy
                Vmath::Vvtvp(nPointsTot,grad_base_u1,1,pVelocity[1],1,pOutarray,1,pOutarray,1);
                break;
		
                //y-equation
            case 1:
                // Evaluate U dv'/dx
                Vmath::Vmul (nPointsTot,grad0,1,m_base[0]->GetPhys(),1,pOutarray,1);
                //Evaluate U dv'/dx+ V dv'/dy
                Vmath::Vvtvp(nPointsTot,grad1,1,m_base[1]->GetPhys(),1,pOutarray,1,pOutarray,1);
                //Evaluate (U dv'/dx+ V dv'/dy)+u' dV/dx
                Vmath::Vvtvp(nPointsTot,grad_base_v0,1,pVelocity[0],1,pOutarray,1,pOutarray,1);
                //Evaluate (U dv'/dx+ V dv'/dy +u' dv/dx)+v' dV/dy
                Vmath::Vvtvp(nPointsTot,grad_base_v1,1,pVelocity[1],1,pOutarray,1,pOutarray,1);
                break;
            }
            break;
            
            //3D
        case 3:
				
			grad1 = Array<OneD, NekDouble> (nPointsTot);
			grad2 = Array<OneD, NekDouble> (nPointsTot);
			grad_base_u1 = Array<OneD, NekDouble> (nPointsTot);
			grad_base_v1 = Array<OneD, NekDouble> (nPointsTot);
			grad_base_w1 = Array<OneD, NekDouble> (nPointsTot);
			
			grad_base_u2 = Array<OneD, NekDouble> (nPointsTot);
			grad_base_v2 = Array<OneD, NekDouble> (nPointsTot);
			grad_base_w2 = Array<OneD, NekDouble> (nPointsTot);

			pFields[0]->PhysDeriv(pVelocity[pVelocityComponent], grad0, grad1, grad2);
			
            pFields[0]->PhysDeriv(m_base[0]->GetPhys(), grad_base_u0, grad_base_u1,grad_base_u2);
            pFields[0]->PhysDeriv(m_base[1]->GetPhys(), grad_base_v0, grad_base_v1,grad_base_v2);
            pFields[0]->PhysDeriv(m_base[2]->GetPhys(), grad_base_w0, grad_base_w1, grad_base_w2);
				         
				
			switch (pVelocityComponent)
            {
					
                //x-equation	
            case 0:
					
				if(m_session->DefinesSolverInfo("SingleMode")==false|| m_dealiasing)

				{
					//U du'/dx
					pFields[0]->DealiasedProd(m_base[0]->GetPhys(),grad0,grad0,m_UseContCoeff);

					//V du'/dy
					pFields[0]->DealiasedProd(m_base[1]->GetPhys(),grad1,grad1,m_UseContCoeff);

                    //W du'/dx
					pFields[0]->DealiasedProd(m_base[2]->GetPhys(),grad2,grad2,m_UseContCoeff);

					// u' dU/dx
					pFields[0]->DealiasedProd(pVelocity[0],grad_base_u0,grad_base_u0,m_UseContCoeff);
					// v' dU/dy
					pFields[0]->DealiasedProd(pVelocity[1],grad_base_u1,grad_base_u1,m_UseContCoeff);
					// w' dU/dz
					pFields[0]->DealiasedProd(pVelocity[2],grad_base_u2,grad_base_u2,m_UseContCoeff);

					Vmath::Vadd(nPointsTot,grad0,1,grad1,1,pOutarray,1);
					Vmath::Vadd(nPointsTot,grad2,1,pOutarray,1,pOutarray,1);
					Vmath::Vadd(nPointsTot,grad_base_u0,1,pOutarray,1,pOutarray,1);
					Vmath::Vadd(nPointsTot,grad_base_u1,1,pOutarray,1,pOutarray,1);
					Vmath::Vadd(nPointsTot,grad_base_u2,1,pOutarray,1,pOutarray,1);

				}
				else 
				{
					//Evaluate U du'/dx
					Vmath::Vmul (nPointsTot,grad0,1,m_base[0]->GetPhys(),1,pOutarray,1);
					//Evaluate U du'/dx+ V du'/dy
					Vmath::Vvtvp(nPointsTot,grad1,1,m_base[1]->GetPhys(),1,pOutarray,1,pOutarray,1);
					//Evaluate (U du'/dx+ V du'/dy)+u' dU/dx
					Vmath::Vvtvp(nPointsTot,grad_base_u0,1,pVelocity[0],1,pOutarray,1,pOutarray,1);
					//Evaluate (U du'/dx+ V du'/dy +u' dU/dx)+v' dU/dy
					Vmath::Vvtvp(nPointsTot,grad_base_u1,1,pVelocity[1],1,pOutarray,1,pOutarray,1);
					//Evaluate (U du'/dx+ V du'/dy +u' dU/dx +v' dU/dy) + W du'/dz
					Vmath::Vvtvp(nPointsTot,grad2,1,m_base[2]->GetPhys(),1,pOutarray,1,pOutarray,1);
					//Evaluate (U du'/dx+ V du'/dy +u' dU/dx +v' dU/dy + W du'/dz)+ w' dU/dz
					Vmath::Vvtvp(nPointsTot,grad_base_u2,1,pVelocity[2],1,pOutarray,1,pOutarray,1);
				}
                break;
                //y-equation	
            case 1:
					if(m_session->DefinesSolverInfo("SingleMode")==false|| m_dealiasing)

					{

						//U dv'/dx
						pFields[0]->DealiasedProd(m_base[0]->GetPhys(),grad0,grad0,m_UseContCoeff);
						//V dv'/dy
						pFields[0]->DealiasedProd(m_base[1]->GetPhys(),grad1,grad1,m_UseContCoeff);
						//W dv'/dx
						pFields[0]->DealiasedProd(m_base[2]->GetPhys(),grad2,grad2,m_UseContCoeff);
						// u' dV/dx
						pFields[0]->DealiasedProd(pVelocity[0],grad_base_v0,grad_base_v0,m_UseContCoeff);
						// v' dV/dy
						pFields[0]->DealiasedProd(pVelocity[1],grad_base_v1,grad_base_v1,m_UseContCoeff);
						// w' dV/dz
						pFields[0]->DealiasedProd(pVelocity[2],grad_base_v2,grad_base_v2,m_UseContCoeff);
						
						Vmath::Vadd(nPointsTot,grad0,1,grad1,1,pOutarray,1);
						Vmath::Vadd(nPointsTot,grad2,1,pOutarray,1,pOutarray,1);
						Vmath::Vadd(nPointsTot,grad_base_v0,1,pOutarray,1,pOutarray,1);
						Vmath::Vadd(nPointsTot,grad_base_v1,1,pOutarray,1,pOutarray,1);
						Vmath::Vadd(nPointsTot,grad_base_v2,1,pOutarray,1,pOutarray,1);
					}
					else 
					{

						//Evaluate U dv'/dx
						Vmath::Vmul (nPointsTot,grad0,1,m_base[0]->GetPhys(),1,pOutarray,1);
						//Evaluate U dv'/dx+ V dv'/dy
						Vmath::Vvtvp(nPointsTot,grad1,1,m_base[1]->GetPhys(),1,pOutarray,1,pOutarray,1);
						//Evaluate (U dv'/dx+ V dv'/dy)+u' dV/dx
						Vmath::Vvtvp(nPointsTot,grad_base_v0,1,pVelocity[0],1,pOutarray,1,pOutarray,1);
						//Evaluate (U du'/dx+ V du'/dy +u' dV/dx)+v' dV/dy
						Vmath::Vvtvp(nPointsTot,grad_base_v1,1,pVelocity[1],1,pOutarray,1,pOutarray,1);
						//Evaluate (U du'/dx+ V dv'/dy +u' dV/dx +v' dV/dy) + W du'/dz
						Vmath::Vvtvp(nPointsTot,grad2,1,m_base[2]->GetPhys(),1,pOutarray,1,pOutarray,1);
						//Evaluate (U du'/dx+ V dv'/dy +u' dV/dx +v' dV/dy + W dv'/dz)+ w' dV/dz
						Vmath::Vvtvp(nPointsTot,grad_base_v2,1,pVelocity[2],1,pOutarray,1,pOutarray,1);

						
						}
					break;
                
                //z-equation	
            case 2:
					if(m_session->DefinesSolverInfo("SingleMode")==false|| m_dealiasing)
					{

						//U dw'/dx
						pFields[0]->DealiasedProd(m_base[0]->GetPhys(),grad0,grad0,m_UseContCoeff);
						//V dw'/dy
						pFields[0]->DealiasedProd(m_base[1]->GetPhys(),grad1,grad1,m_UseContCoeff);
						//W dw'/dx
						pFields[0]->DealiasedProd(m_base[2]->GetPhys(),grad2,grad2,m_UseContCoeff);
						// u' dW/dx
						pFields[0]->DealiasedProd(pVelocity[0],grad_base_w0,grad_base_w0,m_UseContCoeff);
						// v' dW/dy
						pFields[0]->DealiasedProd(pVelocity[1],grad_base_w1,grad_base_w1,m_UseContCoeff);
						// w' dW/dz
						pFields[0]->DealiasedProd(pVelocity[2],grad_base_w2,grad_base_w2,m_UseContCoeff);
						
						Vmath::Vadd(nPointsTot,grad0,1,grad1,1,pOutarray,1);
						Vmath::Vadd(nPointsTot,grad2,1,pOutarray,1,pOutarray,1);
						Vmath::Vadd(nPointsTot,grad_base_w0,1,pOutarray,1,pOutarray,1);
						Vmath::Vadd(nPointsTot,grad_base_w1,1,pOutarray,1,pOutarray,1);
						Vmath::Vadd(nPointsTot,grad_base_w2,1,pOutarray,1,pOutarray,1);
						
						
					}
					else 
					{
						//Evaluate U dw'/dx
						Vmath::Vmul (nPointsTot,grad0,1,m_base[0]->GetPhys(),1,pOutarray,1);
						//Evaluate U dw'/dx+ V dw'/dx
						Vmath::Vvtvp(nPointsTot,grad1,1,m_base[1]->GetPhys(),1,pOutarray,1,pOutarray,1);
						//Evaluate (U dw'/dx+ V dw'/dx)+u' dW/dx
						Vmath::Vvtvp(nPointsTot,grad_base_w0,1,pVelocity[0],1,pOutarray,1,pOutarray,1);
						//Evaluate (U dw'/dx+ V dw'/dx +w' dW/dx)+v' dW/dy
						Vmath::Vvtvp(nPointsTot,grad_base_w1,1,pVelocity[1],1,pOutarray,1,pOutarray,1);
						//Evaluate (U dw'/dx+ V dw'/dx +u' dW/dx +v' dW/dy) + W dw'/dz
						Vmath::Vvtvp(nPointsTot,grad2,1,m_base[2]->GetPhys(),1,pOutarray,1,pOutarray,1);
						//Evaluate (U dw'/dx+ V dw'/dx +u' dW/dx +v' dW/dy + W dw'/dz)+ w' dW/dz
						Vmath::Vvtvp(nPointsTot,grad_base_w2,1,pVelocity[2],1,pOutarray,1,pOutarray,1);
					}					
                break;
					
					
            }
            break;
            
        default:
            ASSERTL0(false,"dimension unknown");
        }
    }
		
		void LinearisedAdvection::UpdateBase( const NekDouble m_slices,
											 Array<OneD, const NekDouble> &inarray,
											 Array<OneD, NekDouble> &outarray,
											 const NekDouble m_time,
											 const NekDouble m_period)
		{
			
			int npoints=m_base[0]->GetTotPoints();
			
			NekDouble BetaT=2*M_PI*fmod (m_time, m_period) / m_period;
			NekDouble phase;
			Array<OneD, NekDouble> auxiliary(npoints);
			
			Vmath::Vcopy(npoints,&inarray[0],1,&outarray[0],1);
			Vmath::Svtvp(npoints, cos(0.5*m_slices*BetaT),&inarray[npoints],1,&outarray[0],1,&outarray[0],1);
			
			for (int i = 2; i < m_slices; i += 2) 
			{
				phase = (i>>1) * BetaT;
				Vmath::Svtvp(npoints, cos(phase),&inarray[i*npoints],1,&outarray[0],1,&outarray[0],1);
				Vmath::Svtvp(npoints, -sin(phase), &inarray[(i+1)*npoints], 1, &outarray[0], 1,&outarray[0],1);
			}
						
		}
	
	void LinearisedAdvection::WriteFldBase(std::string &outname)
	{

		Array<OneD, Array<OneD, NekDouble> > fieldcoeffs(m_base.num_elements());
		Array<OneD, std::string>  variables(m_base.num_elements());

		for(int i = 0; i < m_base.num_elements(); ++i)
		{
			fieldcoeffs[i] = m_base[i]->UpdateCoeffs();
			variables[i] = m_boundaryConditions->GetVariable(i);
		}
		WriteFldBase(outname, m_base[0], fieldcoeffs, variables);
		
	}
	
	
	/**
	 * Writes the field data to a file with the given filename.
	 * @param   outname     Filename to write to.
	 * @param   field       ExpList on which data is based
	 * @param fieldcoeffs   An array of array of expansion coefficients
	 * @param  variables    An array of variable names
	 */
	void LinearisedAdvection::WriteFldBase(std::string &outname, MultiRegions::ExpListSharedPtr &field, Array<OneD, Array<OneD, NekDouble> > &fieldcoeffs, Array<OneD, std::string> &variables)
	{
		
		std::vector<SpatialDomains::FieldDefinitionsSharedPtr> FieldDef= field->GetFieldDefinitions();
		std::vector<std::vector<NekDouble> > FieldData(FieldDef.size());
		
		// copy Data into FieldData and set variable
		for(int j = 0; j < fieldcoeffs.num_elements(); ++j)
		{
			for(int i = 0; i < FieldDef.size(); ++i)
			{
				// Could do a search here to find correct variable
				FieldDef[i]->m_fields.push_back(variables[j]);
				//cout<<"v="<<variables[j]<<endl;                
				field->AppendFieldData(FieldDef[i], FieldData[i], fieldcoeffs[j]);
			}            
		}
		m_graph->Write(outname,FieldDef,FieldData);
	}		
	
	
} //end of namespace

