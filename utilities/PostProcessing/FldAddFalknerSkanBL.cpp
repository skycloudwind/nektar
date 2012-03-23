/* ===============================================================================
 * Generation of an .fld file for the Blasius boundary layer. 
 * Requirements: 
 *                a) Session file with the mesh in the physical space and some 
 *                   data to define the BL properly
 * 
 *                b) Blasius similarity solution consistent with the dimensions
 *                   of the mesh file
=============================================================================== */

/* =====================================
 * Author: Gianmarco Mengaldo 
 * Generation: dd/mm/aa = 08/03/12
===================================== */ 

//! Loading cc libraries
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string.h>
#include <sstream>
#include <iomanip>

//! Loading Nektar++ libraries
#include <LibUtilities/Memory/NekMemoryManager.hpp>
#include <MultiRegions/ExpList.h>
#include <MultiRegions/ExpList0D.h>
#include <MultiRegions/ExpList1D.h>
#include <MultiRegions/ExpList2D.h>
#include <MultiRegions/ExpList3D.h>
#include <MultiRegions/ExpList2DHomogeneous1D.h>
#include <MultiRegions/ExpList3DHomogeneous1D.h>
#include <MultiRegions/ExpList1DHomogeneous2D.h>
#include <MultiRegions/ExpList3DHomogeneous2D.h>
#include <MultiRegions/ContField1D.h>
#include <MultiRegions/ContField2D.h>



//! Nektar++ namespace
using namespace Nektar;

//! Main
int main(int argc, char *argv[])
{
    //! Setting up the decimal precision to machine precision
    setprecision (16);
    
    //! Auxiliary counters for the x and y directions
    int  i, j, k, m;
    
    //! Auxiliary variables
    char    LocalString[1000];
    string  GlobalString[10000];
    bool    inspection = 1;

    string txtfile = argv[argc-1];
    --argc;
    
    //! Check for the command line
    //if(argc != 3)
    //{
    //  fprintf(stderr,"Usage: FldAddFSBL  meshfile txt_file\n");
    //  exit(1);
    //}

    //! Reading the session file
    LibUtilities::SessionReaderSharedPtr vSession = LibUtilities::SessionReader::CreateInstance(argc, argv);
    
    //! Loading the parameters to define the BL
    NekDouble Re;
    NekDouble L;
    NekDouble U_inf;
    NekDouble x;
    NekDouble x_0;
    NekDouble nu;
    string    BL_type;
    int       numLines; 

    
    BL_type = vSession->GetSolverInfo("BL_type");

    vSession->LoadParameter("Re",               Re,             1.0);
    vSession->LoadParameter("L",                L,              1.0);
    vSession->LoadParameter("U_inf",            U_inf,          1.0);
    vSession->LoadParameter("x",                x,              1.0);
    vSession->LoadParameter("x_0",              x_0,            1.0);
    vSession->LoadParameter("NumberLines_txt",  numLines,       1.0);


    
    if(x <= 0)
    {
        fprintf(stderr,"Error: x must be positive ===> CHECK the session file\n");
        exit(1);
    }
    
    if(x_0 < 0)
    {
        fprintf(stderr,"Error: x_0 must be positive or at least equal to 0 ===> CHECK the session file\n");
        exit(1);
    }

    
    
    std::cout<<"************************************************************\n";
    std::cout<<"PHYSICAL DATA FROM THE SESSION FILE:\n";
    std::cout << "Reynolds number                               = " << Re            << std::endl;
    std::cout << "Characteristic length [m]                     = " << L             << std::endl;
    std::cout << "U_infinity [m/s]                              = " << U_inf         << std::endl;
    std::cout << "Position x (parallel case only) [m]           = " << x             << std::endl;
    std::cout << "Position x_0 to start the BL [m]              = " << x_0           << std::endl;
    std::cout << "Number of lines of the .txt file              = " << numLines      << std::endl;
    std::cout << "BL type                                       = " << BL_type       << std::endl;
    std::cout<<"************************************************************\n";
    std::cout<<"------------------------------------------------------------\n";
    std::cout<<"MESH and EXPANSION DATA:\n";
    
    //! Computation of the kinematic viscosity
    nu = U_inf * L / Re;

    //! Read in mesh from input file and create an object of class MeshGraph2D
    SpatialDomains::MeshGraphSharedPtr graphShPt; 
    graphShPt = MemoryManager<SpatialDomains::MeshGraph2D>::AllocateSharedPtr(vSession);
    
    //!  Feed our spatial discretisation object
    MultiRegions::ContField2DSharedPtr Domain;
    Domain = MemoryManager<MultiRegions::ContField2D>::AllocateSharedPtr(vSession,graphShPt,vSession->GetVariable(0));
    
    //! Get the total number of elements
    int nElements;
    nElements = Domain->GetExpSize();
    std::cout << "Number of elements           = " << nElements << std::endl;
    
    //! Get the total number of quadrature points (depends on n. modes)
    int nQuadraturePts;
    nQuadraturePts = Domain->GetTotPoints();
    std::cout << "Number of quadrature points  = " << nQuadraturePts << std::endl;

    //! Coordinates of the quadrature points
    Array<OneD,NekDouble> x_QuadraturePts;
    Array<OneD,NekDouble> y_QuadraturePts;
    Array<OneD,NekDouble> z_QuadraturePts;
    x_QuadraturePts = Array<OneD,NekDouble>(nQuadraturePts);
    y_QuadraturePts = Array<OneD,NekDouble>(nQuadraturePts);
    z_QuadraturePts = Array<OneD,NekDouble>(nQuadraturePts);
    Domain->GetCoords(x_QuadraturePts,y_QuadraturePts,z_QuadraturePts);
    
    //! Reading the .txt file with eta, f(eta) and f'(eta) -----------------------------------------
    const char *txtfile_char;
    //string txtfile(argv[argc-1]);
    txtfile_char = txtfile.c_str();
    
    ifstream pFile(txtfile_char);
    numLines = numLines/3;
    NekDouble d;
    NekDouble GlobalArray[numLines][3];

    for (j=0; j<=2; j++)
    {
        for (i=0; i<=numLines-1; i++) 
        {
            pFile >> d;
            GlobalArray[i][j] = d;
        }
    }
    //! --------------------------------------------------------------------------------------------

    
    //! Saving eta, f(eta) and f'(eta) into separate arrays ----------------------------------------
    Array<OneD,NekDouble> eta;
    Array<OneD,NekDouble> f;
    Array<OneD,NekDouble> df;
    
    eta = Array<OneD,NekDouble>(numLines);
    f   = Array<OneD,NekDouble>(numLines);
    df  = Array<OneD,NekDouble>(numLines);
    
    for (i=0; i<numLines; i++) 
    {
        eta[i] = GlobalArray[i][0];
        f[i]   = GlobalArray[i][1];
        df[i]  = GlobalArray[i][2];
    }
    //! --------------------------------------------------------------------------------------------

    
    //! Inizialisation of the arrays for computations on the Quadrature points --------------------- 
    Array<OneD,NekDouble> eta_QuadraturePts;
    eta_QuadraturePts = Array<OneD,NekDouble>(nQuadraturePts);

    Array<OneD,NekDouble> f_QuadraturePts;
    f_QuadraturePts   = Array<OneD,NekDouble>(nQuadraturePts);
    
    Array<OneD,NekDouble> df_QuadraturePts;
    df_QuadraturePts  = Array<OneD,NekDouble>(nQuadraturePts);
    
    Array<OneD,NekDouble> u_QuadraturePts;
    u_QuadraturePts   = Array<OneD,NekDouble>(nQuadraturePts);

    Array<OneD,NekDouble> v_QuadraturePts;
    v_QuadraturePts   = Array<OneD,NekDouble>(nQuadraturePts);
    
    Array<OneD,NekDouble> p_QuadraturePts;
    p_QuadraturePts   = Array<OneD,NekDouble>(nQuadraturePts);
    //! --------------------------------------------------------------------------------------------

    
    
    //! PARALLEL CASE ------------------------------------------------------------------------------
    if(BL_type == "Parallel")
    {
        for(i=0; i<nQuadraturePts; i++)
        {
            eta_QuadraturePts[i] = y_QuadraturePts[i] * sqrt(U_inf / (2 * x * nu));
            
            for(j=0; j<numLines; j++)
            {
                if(eta_QuadraturePts[i] >= eta[j] & eta_QuadraturePts[i] <= eta[j+1])
                {
                    f_QuadraturePts[i]  = (eta_QuadraturePts[i] - eta[j]) * (f[j+1] - f[j]) / (eta[j+1] - eta[j]) + f[j]; 
                    df_QuadraturePts[i] = (eta_QuadraturePts[i] - eta[j]) * (df[j+1] - df[j]) / (eta[j+1] - eta[j]) + df[j];
                }
                
                else if(eta_QuadraturePts[i] == 1000000)
                {
                    f_QuadraturePts[i] = f[numLines-1];
                    df_QuadraturePts[i] = df[numLines-1];
                }
                
                else if(eta_QuadraturePts[i] > eta[numLines-1])
                {
                    f_QuadraturePts[i] = f[numLines-1] + df[numLines-1] * (eta_QuadraturePts[i] - eta[numLines-1]);
                    df_QuadraturePts[i] = df[numLines-1];
                }
            }
            
            u_QuadraturePts[i] = U_inf * df_QuadraturePts[i];
            v_QuadraturePts[i] = nu * sqrt(U_inf / (2.0 * nu * x)) * (y_QuadraturePts[i] * sqrt(U_inf / (2.0 * nu * x)) * df_QuadraturePts[i] - f_QuadraturePts[i]);
            p_QuadraturePts[i] = 0.0;
        }
    }
    //! --------------------------------------------------------------------------------------------

    
    
    //! NON-PARALLEL CASE --------------------------------------------------------------------------
    if(BL_type == "nonParallel")
    {
        for(i=0; i<nQuadraturePts; i++)
        {
            eta_QuadraturePts[i] = y_QuadraturePts[i] * sqrt(U_inf / (2 * (x_QuadraturePts[i] + x_0) * nu));
        
            if((x_QuadraturePts[i] + x_0) == 0)
            {
                eta_QuadraturePts[i] = 1000000;        
            }
        
            for(j=0; j<numLines; j++)
            {
                if(eta_QuadraturePts[i] >= eta[j] & eta_QuadraturePts[i] <= eta[j+1])
                {
                    f_QuadraturePts[i]  = (eta_QuadraturePts[i] - eta[j]) * (f[j+1] - f[j]) / (eta[j+1] - eta[j]) + f[j]; 
                    df_QuadraturePts[i] = (eta_QuadraturePts[i] - eta[j]) * (df[j+1] - df[j]) / (eta[j+1] - eta[j]) + df[j];
                }
            
                else if(eta_QuadraturePts[i] == 1000000)
                {
                    f_QuadraturePts[i] = f[numLines-1];
                    df_QuadraturePts[i] = df[numLines-1];
                }
            
                else if(eta_QuadraturePts[i] > eta[numLines-1])
                {
                    f_QuadraturePts[i] = f[numLines-1] + df[numLines-1] * (eta_QuadraturePts[i] - eta[numLines-1]);
                    df_QuadraturePts[i] = df[numLines-1];
                }
            }
        
            u_QuadraturePts[i] = U_inf * df_QuadraturePts[i];
            v_QuadraturePts[i] = nu * sqrt(U_inf / (2.0 * nu * (x_QuadraturePts[i] + x_0))) * 
                                 (y_QuadraturePts[i] * sqrt(U_inf / (2.0 * nu * (x_QuadraturePts[i] + x_0))) * 
                                 df_QuadraturePts[i] - f_QuadraturePts[i]);
        
            //! INFLOW SECTION: X = 0; Y > 0.
            if((x_QuadraturePts[i] + x_0) == 0)
            {
                u_QuadraturePts[i] = U_inf;
                v_QuadraturePts[i] = 0.0;
            }
        
            //! SINGULARITY POINT: X = 0; Y = 0.
            if((x_QuadraturePts[i] + x_0) == 0 & y_QuadraturePts[i] == 0)
            {
                u_QuadraturePts[i] = 0.0;
                v_QuadraturePts[i] = 0.0;
            }
        
            p_QuadraturePts[i] = 0.0;
        }
    }
    //! --------------------------------------------------------------------------------------------

    
        
    //! Inspection of the interpolation ------------------------------------------------------------
    FILE *etaOriginal; 
    etaOriginal = fopen("eta.txt","w+"); 
    for(i=0; i<numLines; i++)
    {
        fprintf(etaOriginal,"%f %f %f \n", eta[i], f[i], df[i]);
    }
    fclose(etaOriginal);
    
        
    FILE *yQ; 
    yQ = fopen("yQ.txt","w+"); 
    for(i=0; i<nQuadraturePts; i++)
    {
        fprintf(yQ,"%f %f %f %f %f %f %f\n", x_QuadraturePts[i], y_QuadraturePts[i], u_QuadraturePts[i],
                v_QuadraturePts[i], eta_QuadraturePts[i], f_QuadraturePts[i], df_QuadraturePts[i]);
    }
    fclose(yQ);
    //! --------------------------------------------------------------------------------------------

    
    
    //! Definition of the 2D expansion using the mesh data specified on the session file -----------
    MultiRegions::ExpList2DSharedPtr Exp2D_uk;
    Exp2D_uk = MemoryManager<MultiRegions::ExpList2D>::AllocateSharedPtr(vSession,graphShPt);
    
    MultiRegions::ExpList2DSharedPtr Exp2D_vk;
    Exp2D_vk = MemoryManager<MultiRegions::ExpList2D>::AllocateSharedPtr(vSession,graphShPt);
    
    MultiRegions::ExpList2DSharedPtr Exp2D_pk;
    Exp2D_pk = MemoryManager<MultiRegions::ExpList2D>::AllocateSharedPtr(vSession,graphShPt);
    //! --------------------------------------------------------------------------------------------
    
    
    
    //! Filling the 2D expansion using a recursive algorithm based on the mesh ordering ------------
    m = 0;
    LibUtilities::BasisSharedPtr Basis;
    Basis = Domain->GetExp(0)->GetBasis(0);
      
    //! Copying the ukGlobal vector (with the same pattern of m_phys) in m_phys 
    Vmath::Vcopy(nQuadraturePts, u_QuadraturePts , 1, Exp2D_uk->UpdatePhys(), 1);
    Vmath::Vcopy(nQuadraturePts, v_QuadraturePts,  1, Exp2D_vk->UpdatePhys(), 1);
    Vmath::Vcopy(nQuadraturePts, p_QuadraturePts , 1, Exp2D_pk->UpdatePhys(), 1);

    //! Initialisation of the ExpList Exp
    Array<OneD, MultiRegions::ExpListSharedPtr> Exp(3);
    Exp[0] = Exp2D_uk;
    Exp[1] = Exp2D_vk;
    Exp[2] = Exp2D_pk;

    //! Expansion coefficient extraction (necessary to write the .fld file)    
    Exp[0]->FwdTrans(Exp2D_uk->GetPhys(),Exp[0]->UpdateCoeffs());
    Exp[1]->FwdTrans(Exp2D_vk->GetPhys(),Exp[1]->UpdateCoeffs());
    Exp[2]->FwdTrans(Exp2D_pk->GetPhys(),Exp[2]->UpdateCoeffs());
    //! --------------------------------------------------------------------------------------------


    
    //! Generation .FLD file with one field only (at the moment) -----------------------------------
    //! Definition of the name of the .fld file
    string blasius = "blasius.fld";
    
    //! Definition of the number of the fields
    int nFields = 3;    
    
    //! Definition of the Field
    std::vector<SpatialDomains::FieldDefinitionsSharedPtr> FieldDef = Exp[0]->GetFieldDefinitions();
    std::vector<std::vector<NekDouble> > FieldData(FieldDef.size());
        
    for(j = 0; j < nFields; ++j)
    {
		for(i = 0; i < FieldDef.size(); ++i)
		{
            if(j == 0)
            {
                FieldDef[i]->m_fields.push_back("u");
            }
            else if(j == 1)
            {
                FieldDef[i]->m_fields.push_back("v");
            }
            else if(j == 2)
            {
                FieldDef[i]->m_fields.push_back("p");
            }
            Exp[j]->AppendFieldData(FieldDef[i], FieldData[i]);
        }
    }    
    graphShPt->Write(blasius, FieldDef, FieldData);
    std::cout<<"------------------------------------------------------------\n";
    //! --------------------------------------------------------------------------------------------

    return 0;
}

