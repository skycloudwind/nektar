#include <cstdio>
#include <cstdlib>

#include <MultiRegions/ContField3DHomogeneous1D.h>

using namespace Nektar;

int NoCaseStringCompare(const string & s1, const string& s2);

int main(int argc, char *argv[])
{
    MultiRegions::ContField3DHomogeneous1DSharedPtr Exp, Fce;
    int     i, nq,  coordim;
    Array<OneD,NekDouble>  fce;
    Array<OneD,NekDouble>  xc0,xc1,xc2;
    NekDouble  lambda;
    // default solution type multilevel statis condensation
    MultiRegions::GlobalSysSolnType SolnType = MultiRegions::eDirectMultiLevelStaticCond;

    if( (argc != 2) && (argc != 3))
    {
        fprintf(stderr,"Usage: Helmholtz3DHomo1D meshfile [SysSolnType]   \n");
        exit(1);
    }

    //----------------------------------------------
    // Load the solver type so we can test full solve, static
    // condensation and the default multi-level statis condensation.
    if( argc == 3 )
    {
        if(!NoCaseStringCompare(argv[2],"MultiLevelStaticCond"))
        {
            SolnType = MultiRegions::eDirectMultiLevelStaticCond;
            cout << "Solution Type: MultiLevel Static Condensation" << endl;
        }
        else if(!NoCaseStringCompare(argv[2],"StaticCond"))
        {
            SolnType = MultiRegions::eDirectStaticCond;
            cout << "Solution Type: Static Condensation" << endl;
        }
        else if(!NoCaseStringCompare(argv[2],"FullMatrix"))
        {
            SolnType = MultiRegions::eDirectFullMatrix;
            cout << "Solution Type: Full Matrix" << endl;
        }
        else
        {
            cerr << "SolnType not recognised" <<endl;
            exit(1);
        }

    }
    //----------------------------------------------

    //----------------------------------------------
    // Read in mesh from input file
    string meshfile(argv[1]);
    SpatialDomains::MeshGraph2D graph2D;
    graph2D.ReadGeometry(meshfile);
    graph2D.ReadExpansions(meshfile);
    //----------------------------------------------

    //----------------------------------------------
    // read the problem parameters from input file
    SpatialDomains::BoundaryConditions bcs(&graph2D);
    bcs.Read(meshfile);
    //----------------------------------------------

    //----------------------------------------------
    // Define Expansion
    int bc_val = 0;
    int nplanes = 8;
    NekDouble lz     = bcs.GetParameter("Lz");
    const LibUtilities::PointsKey Pkey(nplanes,LibUtilities::eFourierEvenlySpaced);
    const LibUtilities::BasisKey Bkey(LibUtilities::eFourier,nplanes,Pkey);
    Exp = MemoryManager<MultiRegions::ContField3DHomogeneous1D>
        ::AllocateSharedPtr(Bkey,lz,graph2D,bcs,bc_val,SolnType);
    //----------------------------------------------

    //----------------------------------------------
    // Print summary of solution details
    lambda = bcs.GetParameter("Lambda");
    const SpatialDomains::ExpansionVector &expansions = graph2D.GetExpansions();
    LibUtilities::BasisKey bkey0 = expansions[0]->m_basisKeyVector[0];
    cout << "Solving 3D Helmholtz (Homogeneous in z-direction):"  << endl;
    cout << "         Lambda          : " << lambda << endl;
    cout << "         Lz              : " << lz << endl;
    cout << "         No. modes       : " << bkey0.GetNumModes() << endl;
    cout << "         No. hom. modes  : " << Bkey.GetNumModes() << endl;
    cout << endl;
    //----------------------------------------------

    //----------------------------------------------
    // Set up coordinates of mesh for Forcing function evaluation
    coordim = Exp->GetCoordim(0);
    nq      = Exp->GetTotPoints();

    xc0 = Array<OneD,NekDouble>(nq,0.0);
    xc1 = Array<OneD,NekDouble>(nq,0.0);
    xc2 = Array<OneD,NekDouble>(nq,0.0);

    Exp->GetCoords(xc0,xc1,xc2);
    //----------------------------------------------

    //----------------------------------------------
    // Define forcing function for first variable defined in file
    fce = Array<OneD,NekDouble>(nq);
    SpatialDomains::ConstForcingFunctionShPtr ffunc
        = bcs.GetForcingFunction(bcs.GetVariable(0));
    for(i = 0; i < nq; ++i)
    {
        fce[i] = ffunc->Evaluate(xc0[i],xc1[i],xc2[i]);
    }
    //----------------------------------------------

    //----------------------------------------------
    // Setup expansion containing the  forcing function
    Fce = MemoryManager<MultiRegions::ContField3DHomogeneous1D>::AllocateSharedPtr(*Exp);
    Fce->SetPhys(fce);
    //----------------------------------------------

    //----------------------------------------------
    // Helmholtz solution taking physical forcing
    //Exp->HelmSolve(Fce->GetPhys(), Exp->UpdateCoeffs(), lambda);
    Exp->HelmSolve(Fce->GetPhys(), Exp->UpdateContCoeffs(), lambda, true);
     //----------------------------------------------

    //----------------------------------------------
    // Backward Transform Solution to get solved values at
    //Exp->BwdTrans(Exp->GetCoeffs(), Exp->UpdatePhys());
    Exp->BwdTrans(Exp->GetContCoeffs(), Exp->UpdatePhys(),true);
    //----------------------------------------------

    //----------------------------------------------
    // See if there is an exact solution, if so
    // evaluate and plot errors
    SpatialDomains::ConstExactSolutionShPtr ex_sol
        = bcs.GetExactSolution(bcs.GetVariable(0));

    if(ex_sol)
    {
        //----------------------------------------------
        // evaluate exact solution
        for(i = 0; i < nq; ++i)
        {
            fce[i] = ex_sol->Evaluate(xc0[i],xc1[i],xc2[i]);
        }
        //----------------------------------------------

        //--------------------------------------------
        // Calculate L_inf error
        Fce->SetPhys(fce);
        Fce->SetPhysState(true);

        cout << "L infinity error: " << Exp->Linf(Fce->GetPhys()) << endl;
        cout << "L 2 error:        " << Exp->L2  (Fce->GetPhys()) << endl;
        //--------------------------------------------
    }
    //----------------------------------------------

    //-----------------------------------------------
    // Write solution to file
    string   out(strtok(argv[1],"."));
    string   endfile(".fld");
    out += endfile;
    std::vector<SpatialDomains::FieldDefinitionsSharedPtr> FieldDef;

    Exp->GetFieldDefinitions(FieldDef);

    std::vector<std::vector<NekDouble> > FieldData(FieldDef.size());

    Exp->GlobalToLocal();
    for(i = 0; i < FieldDef.size(); ++i)
    {
        FieldDef[i]->m_fields.push_back("u");
        Exp->AppendFieldData(FieldDef[i], FieldData[i]);
    }

    graph2D.Write(out, FieldDef, FieldData);
    //-----------------------------------------------

    return 0;
}


/**
 * Performs a case-insensitive string comparison (from web).
 * @param   s1          First string to compare.
 * @param   s2          Second string to compare.
 * @returns             0 if the strings match.
 */
int NoCaseStringCompare(const string & s1, const string& s2)
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

    size_t size1=s1.size();
    size_t size2=s2.size();// cache lengths

    //return -1,0 or 1 according to strings' lengths
    if (size1==size2)
    {
        return 0;
    }

    return (size1 < size2) ? -1 : 1;
}