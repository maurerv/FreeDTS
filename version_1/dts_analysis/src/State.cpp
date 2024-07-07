

/*
 Weria Pezeshkian (weria.pezeshkian@gmail.com)
 Copyright (c) Weria Pezeshkian
 This class distrubutes the tasks based on inputs provided and makes all the initials variables
 It also receives a blue print of the mesh and the generates the reall mesh, then pass it on to Simualtion object to start ..
 */
#include <fstream>
#include "State.h"
#include "Analysis.h"
State::State()
{
}
State::State(std::vector <std::string> argument)
{
    Restart re(this);
    m_Restart = re;
#if TEST_MODE == Enabled
    std::cout<<"----> We have reached the State Class -- "<<std::endl;
#endif
    //============ Initialization of all inputs and data structures for input
    m_Argument = argument;
    m_Healthy =true;
    m_Integrator = "MC";
    m_Initial_Step = 0;
    m_Final_Step = 0;
    m_Seed =36723;
    m_R_Vertex=0.05;   // Move Vertex  within a box with this size
    m_R_Box=0.04;   // box change within this range
    m_TotEnergy = 0;
    m_TopologyFile = "topology.top";
    m_InputFileName = "Input.dts";
    m_IndexFileName = "Index.inx";
    m_IndexFile = false;
    m_GeneralOutputFilename = "dts";
    m_FreezGroupName = "";
    m_BendingRigidity  = 1;
    m_GaussianRigidity = 0;
    m_MinFaceAngle = -0.5;
    m_MinVerticesDistanceSquare = 1.0;
    m_MaxLinkLengthSquare = 3.0;
    m_Display_periodic  = 1000;
    m_OutPutEnergy_periodic = 100;
    m_CNTCELL(0) = 1;m_CNTCELL(1) = 1;m_CNTCELL(2) = 1;
    m_TRJTSI.tsiPeriod = 0;
    m_TRJTSI.tsiPrecision = 1;
    m_TRJTSI.tsiFolder_name = "tsi";
    m_TRJTSI.tsiState = false;
    m_TRJBTS.btsPeriod = 0;
    m_TRJBTS.btsPrecision = 1;
    m_TRJBTS.btsFile_name = "dts.bts";
    m_TRJBTS.btsState = false;
    m_RESTART.restartState = false;
    m_RESTART.restartPeriod = 1000;
    m_MCMove.VertexMove = true;
    m_MCMove.LinkFlip = true;
    m_MCMove.InclusionMove = true;
    m_FrameTension.State = false;
    m_FrameTension.Type = "Position_Rescale";
    m_FrameTension.Tau = 0.0;
    m_FrameTension.updatePeriod = 5;
    m_VolumeConstraint.State = false;
    m_VolumeConstraint.EQSteps = 1000;
    m_VolumeConstraint.DeltaP = 0;
    m_VolumeConstraint.K = 0;
    m_VolumeConstraint.targetV = 1 ;
    m_pMesh = &m_Mesh;

    //====== Updating the input data from provided inputs
    ExploreArguments();     // Find input file name
    ReadInputFile(m_InputFileName);  // Reading from input file
    ExploreArguments();     // Update last state
    Inclusion_Interaction_Map inc_ForceField(m_InputFileName);
    m_inc_ForceField = inc_ForceField;
    m_pinc_ForceField = &m_inc_ForceField;
    CreateMashBluePrint  Cblueprint;
    Cblueprint.ReadInclusionType(m_InputFileName);

    Nfunction f;
    int trjtype = 0;
    std::string ext = m_TopologyFile.substr(m_TopologyFile.find_last_of(".") + 1);
    if(ext==BTSExt)
        trjtype = 1;
    
    Vec3D m_Box;
    Traj_XXX TSIFile(&m_Box,m_TopologyFile, "r");
    BTSFile btsFile(m_TopologyFile, false, "r");
    bool readok = true;
    MeshBluePrint blueprint;

    std::vector<InclusionType>  bINCtype = Cblueprint.GetAllInclusionType();
    int step=1;
    while (true)
    {
        if(trjtype==0)
        {
            std::string filename = m_TopologyFile + "/"+m_GeneralOutputFilename+f.Int_to_String(step)+".tsi";
            blueprint = TSIFile.ReadTSI(filename,bINCtype,&readok);
            if(readok!=true)
                break;
        }
        else if(trjtype==1)
        {
            blueprint = btsFile.ReadBTSFile(&readok);
            if(readok!=true)
                break;
        }
        m_TrjBluePrint.push_back(blueprint);
        step++;
    }

    Analysis Ana(this);
}
State::~State()
{
    
}
void State::ExploreArguments()
{
    Nfunction f;
    for (long i=1;i<m_Argument.size();i=i+2)
    {
        std::string Arg1 = m_Argument.at(i);
        if(Arg1=="-h")
        {
            HelpMessage();
            exit(0);
            break;
        }
        else if(Arg1=="-in")
        {
            m_InputFileName = m_Argument.at(i+1);
        }
        else if(Arg1=="-top")
        {
            m_TopologyFile = m_Argument.at(i+1);
        }
        else if(Arg1=="-defout")
        {
            m_GeneralOutputFilename = m_Argument.at(i+1);
        }
        else if(Arg1=="-ndx")
        {
            m_IndexFileName = m_Argument.at(i+1);
            m_IndexFile = true;
        }
        else if(Arg1=="-b")
        {
            m_Initial_Step = f.String_to_Int(m_Argument.at(i+1));
        }
        else if(Arg1=="-e")
        {
            m_Final_Step = f.String_to_Int(m_Argument.at(i+1));
        }
        else if(Arg1=="-seed")
        {
            m_Seed = f.String_to_Int(m_Argument.at(i+1));
        }
        else if(Arg1=="-angle")
        {
            m_MinFaceAngle = f.String_to_Double(m_Argument.at(i+1));
        }
        else if(Arg1=="-maxDist")
        {
            m_MaxLinkLengthSquare = f.String_to_Double(m_Argument.at(i+1));
        }
        else if(Arg1=="-minDist")
        {
            m_MinVerticesDistanceSquare = f.String_to_Double(m_Argument.at(i+1));
        }
        else if(Arg1=="-restart")
        {
            m_RESTART.restartState = true;
            m_RESTART.restartFilename = m_Argument.at(i+1);
        }
        else
        {
            std::cout << "Error: simulation will stop!!:"<<Arg1;
            std::cout<<"\n"<<"For more information and tips run "<< m_Argument.at(0) <<" -h"<<"\n";
            m_Healthy =false;
            exit(0);
            break;
        }
    }

}
void State::ReadInputFile(std::string file)
{
    Nfunction f;
    // enforcing correct file extension:  check simdef file for value of InExt
    std::string ext = file.substr(file.find_last_of(".") + 1);
    if(ext!=InExt)
    file = file + "." + InExt;
    if (f.FileExist(file)!=true)
    {
        std::cout<<"----> Error: the input file with the name "<<file<< " does not exist "<<std::endl;
        m_Healthy =false;
        exit(0);
    }

    std::ifstream input;
    input.open(file.c_str());
    std::string firstword,rest,str;

    while (true)
    {
        input>>firstword;
        if(input.eof())
            break;
        
        if(firstword == "Initial_Step")
        {
            input>>str>>m_Initial_Step;
            getline(input,rest);
        }
        else if(firstword == "Integrator")
        {
            input>>str>>m_Integrator;
            getline(input,rest);
        }
        else if(firstword == "MC_Moves")
        {
            input>>str>>(m_MCMove.VertexMove)>>(m_MCMove.LinkFlip)>>(m_MCMove.InclusionMove);
            getline(input,rest);
        }
        else if(firstword == "FreezingAGroup")
        {
            input>>str>>m_FreezGroupName;
            getline(input,rest);
        }
        else if(firstword == "Frame_Tension")
        {
            std::string state;
            (m_FrameTension.State)= false;
            input>>str>>state>>(m_FrameTension.Type)>>(m_FrameTension.Tau)>>(m_FrameTension.updatePeriod);
            if(state == "on" || state == "ON" || state == "On" )
                (m_FrameTension.State) = true;
            getline(input,rest);
            
            
        }
        else if(firstword == "Volume_Constraint")
        {
            std::string state;
            m_VolumeConstraint.State = false;
        input>>str>>state>>(m_VolumeConstraint.EQSteps)>>(m_VolumeConstraint.DeltaP)>>(m_VolumeConstraint.K)>>(m_VolumeConstraint.targetV);
            if(state == "on" || state == "ON" || state == "On" )
               m_VolumeConstraint.State = true;
            getline(input,rest);
            
            
            CmdVolumeCouplingSecondOrder C(m_VolumeConstraint.State, m_VolumeConstraint.EQSteps, m_VolumeConstraint.DeltaP,  m_VolumeConstraint.K, m_VolumeConstraint.targetV);
            m_VolumeCouplingSecondOrder = C;
        }
        else if(firstword == "Final_Step")
        {
            input>>str>>m_Final_Step;
            getline(input,rest);
        }
        else if(firstword == "MinfaceAngle")
        {
            input>>str>>m_MinFaceAngle;
            getline(input,rest);
        }
        else if(firstword == "OutPutEnergy_periodic")
        {
            input>>str>>m_OutPutEnergy_periodic;
            getline(input,rest);
        }
        else if(firstword == "Restart_periodic")
        {
            input>>str>>(m_RESTART.restartPeriod);
            getline(input,rest);
        }
        else if(firstword == "TopologyFile")
        {
            input>>str>>m_TopologyFile;
            getline(input,rest);
        }
        else if(firstword == "CouplingtoFixedGlobalCurvature")
        {
            std::string state;
            double k,c0;
            input>>str>>state>>k>>c0;
            if(state == "yes" || state == "on")
            {
                CouplingtoFixedGlobalCurvature  CoupleGCurvature(true,k,c0);
                m_CoupleGCurvature = CoupleGCurvature;
            }
            else
            {
                CouplingtoFixedGlobalCurvature  CoupleGCurvature(false,k,c0);
                m_CoupleGCurvature = CoupleGCurvature;
            }
            getline(input,rest);
        }
        else if(firstword == "HarmonicPotentialBetweenTwoGroups")
        {
            //HarmonicPotentialBetweenTwoGroups = on 10 0.1 2000 Group1 Group2 0 1 1
            std::string state,g1,g2;
            double k,dx;
            int nx,ny,nz,rate;
            input>>str>>state>>k>>dx>>rate>>g1>>g2>>nx>>ny>>nz;
            if(state == "yes" || state == "on")
            {
                SpringPotentialBetweenTwoGroups  tem(true,k, dx,rate,g1,g2,nx,ny,nz);
                m_SpringPotentialBetweenTwoGroups = tem;
            }
            else
            {
                SpringPotentialBetweenTwoGroups  tem(false,k, dx,rate,g1,g2,nx,ny,nz);
                m_SpringPotentialBetweenTwoGroups = tem;
            }
        }
        else if(firstword == "Seed")
        {
            input>>str>>m_Seed;
            getline(input,rest);
        }
        else if(firstword == "Kappa")
        {
            input>>str>>m_BendingRigidity>>m_GaussianRigidity;
            getline(input,rest);
        }
        else if(firstword == "Display_periodic")
        {
            input>>str>>m_Display_periodic;
            getline(input,rest);
        }
        else if(firstword == "Cell_Size")
        {
            input>>str>>m_CNTCELL(0)>>m_CNTCELL(1)>>m_CNTCELL(2);
            getline(input,rest);
        }
        else if(firstword == "GeneralOutputFilename")
        {
            input>>str>>m_GeneralOutputFilename;
            getline(input,rest);
        }
        else if(firstword == "Min_Max_LinkLenghtsSquare")
        {
            input>>str>>m_MinVerticesDistanceSquare>>m_MaxLinkLengthSquare;
            getline(input,rest);
        }
        else if(firstword == "CoupleToRigidWalls")
        {
            input>>str>>str;
            bool state = false;
            if(str == "on" || str == "yes" || str == "Yes")
            state = true;
            getline(input,rest);
            
            CoupleToWallPotential wall(state,rest);
            m_RigidWallCoupling = wall;
        }
        else if(firstword == "OutPutTRJ_TSI")
        {
            input>>str>>(m_TRJTSI.tsiPeriod)>>(m_TRJTSI.tsiPrecision)>>(m_TRJTSI.tsiFolder_name);
            getline(input,rest);
            if((m_TRJTSI.tsiPeriod)>0)
                m_TRJTSI.tsiState = true;
            else
                m_TRJTSI.tsiState = false;
        }
        else if(firstword == "OutPutTRJ_BTS")
        {
            input>>str>>(m_TRJBTS.btsPeriod)>>(m_TRJBTS.btsPrecision)>>(m_TRJBTS.btsFile_name);
            getline(input,rest);
            if((m_TRJBTS.btsPeriod)>0)
                m_TRJBTS.btsState = true;
            else
                m_TRJBTS.btsState = false;
        }
        else if(firstword == "INCLUSION")
        {
            break;
        }
        else
        {
            if(firstword.at(0)!=';')
            {
                std::cout<<"Error: bad keyword in the input file "<<firstword<<"\n";
                break;
            }
            getline(input,rest);
        }
    }
    input.close();
    
}
void State::HelpMessage()
{
    std::cout<<"----------------------copyright: Weria Pezeshkian------------------"<<"\n";
    std::cout<<" =================================================================  \n";
    std::cout<<"---------------------version "<<SoftWareVersion<<" ------------------"<<"\n";
    std::cout<<" =================================================================  \n";
    std::cout<<"------------simple example for exacuting the analysis -------------------"<<"\n";
    std::cout<< "ANA -f Input.dmc  -top data.bts "<<"\n";
    std::cout<<"-------------------------------------------------------------------------------"<<"\n";
    std::cout<<"  option    type        default            description "<<"\n";
    std::cout<<"-------------------------------------------------------------------------------"<<"\n";
    std::cout<<"  -in        string       Input.dmc        input file name "<<"\n";
    std::cout<<"  -top       string       Input.q           input file name "<<"\n";
    std::cout<<"  -b         int          1                 initial time step "<<"\n";
    std::cout<<"  -e         int          100               final time step  "<<"\n";
    std::cout<<"  -defout    string       output            an string for run out put files   "<<"\n";
    std::cout<<"  -ndx       string       Index.inx         Index file name  "<<"\n";
    std::cout<<"=========================================================================="<<"\n";
    std::cout<<"---------------Inclusion are made just inside inputfile as: --------------------------"<<"\n";
    std::cout<<"=========================================================================="<<"\n";
    std::cout<<"=========================================================================="<<"\n";
    std::cout<<"------------------ version "<<SoftWareVersion<<" ------------------"<<"\n";
    std::cout<<" =================================================================  \n";
}
void MESH::GenerateMesh(MeshBluePrint meshblueprint, double kappa, double kappag)
{
    m_Box = meshblueprint.simbox;
    m_pBox = &m_Box;
    m_InclusionType = meshblueprint.binctype;
    // Making vertices
    for (std::vector<Vertex_Map>::iterator it = (meshblueprint.bvertex).begin() ; it != (meshblueprint.bvertex).end(); ++it)
    {
        vertex v(it->id,it->x,it->y,it->z);
        v.UpdateBox(m_pBox);
        v.UpdateGroup(it->domain);
        v.UpdateKappa(kappa/2.0,kappag);
        m_Vertex.push_back(v);
    }
    // Making triangles
    for (std::vector<Triangle_Map>::iterator it = (meshblueprint.btriangle).begin() ; it != (meshblueprint.btriangle).end(); ++it)
    {
        bool pr=true;
        triangle T(it->id,&(m_Vertex.at(it->v1)),&(m_Vertex.at(it->v2)),&(m_Vertex.at(it->v3)));
        m_Triangle.push_back(T);
    }
    //make inclusions
    for (std::vector<Inclusion_Map>::iterator it = (meshblueprint.binclusion).begin() ; it != (meshblueprint.binclusion).end(); ++it)
    {
        inclusion Tinc(it->id);
        if(m_Vertex.size()<it->vid+1)
        {
        std::cout<<"----> Error: Inclusion vertex id is out of range "<<std::endl;
            exit(0);
        }
        Tinc.Updatevertex(&(m_Vertex.at(it->vid)));
        Tinc.UpdateInclusionTypeID(it->tid);
        Vec3D D(it->x,it->y,0);
        Tinc.UpdateLocalDirection(D);
        m_Inclusion.push_back(Tinc);
        (m_Vertex.at(it->vid)).UpdateOwnInclusion(true);
    }
    for (std::vector<inclusion>::iterator it = m_Inclusion.begin() ; it != m_Inclusion.end(); ++it)
        m_pInclusion.push_back(&(*it));
    for (std::vector<vertex>::iterator it = m_Vertex.begin() ; it != m_Vertex.end(); ++it)
        m_pAllV.push_back(&(*it));
    
    int li=-1;
    
    for (std::vector<triangle>::iterator it = m_Triangle.begin() ; it != m_Triangle.end(); ++it)
    {
        
        m_pAllT.push_back(&(*it));
        (it->GetV1())->AddtoTraingleList(&(*it));
        (it->GetV1())->AddtoNeighbourVertex((it->GetV2()));
        (it->GetV2())->AddtoTraingleList(&(*it));
        (it->GetV2())->AddtoNeighbourVertex((it->GetV3()));
        (it->GetV3())->AddtoTraingleList(&(*it));
        (it->GetV3())->AddtoNeighbourVertex((it->GetV1()));
        
        /// create links
        li++;
        int id1=li;
        li++;
        int id2=li;
        li++;
        int id3=li;
        
        links l1(id1,it->GetV1(),it->GetV2(),&(*it));
        l1.UpdateV3(it->GetV3());
        
        links l2(id2,it->GetV2(),it->GetV3(),&(*it));
        l2.UpdateV3(it->GetV1());
        
        links l3(id3,it->GetV3(),it->GetV1(),&(*it));
        l3.UpdateV3(it->GetV2());
        m_Links.push_back(l1);
        m_Links.push_back(l2);
        m_Links.push_back(l3);
        
    }
    li=-1;
    for (std::vector<triangle>::iterator it = m_Triangle.begin() ; it != m_Triangle.end(); ++it)
    {
        li++;
        int id1=li;
        li++;
        int id2=li;
        li++;
        int id3=li;
        links * l1=&(m_Links.at(id1));
        links * l2=&(m_Links.at(id2));
        links * l3=&(m_Links.at(id3));
        l1->UpdateNeighborLink1(l2);
        l1->UpdateNeighborLink2(l3);
        l2->UpdateNeighborLink1(l3);
        l2->UpdateNeighborLink2(l1);
        l3->UpdateNeighborLink1(l1);
        l3->UpdateNeighborLink2(l2);
        
        
        (it->GetV1())->AddtoLinkList(l1);
        (it->GetV2())->AddtoLinkList(l2);
        (it->GetV3())->AddtoLinkList(l3);
        
    }
    for (std::vector<links>::iterator it = m_Links.begin() ; it != m_Links.end(); ++it)
    {
        bool foundM=false;
        if((it)->GetMirrorFlag()==true)
        {
            m_pMHL.push_back(it->GetMirrorLink());
            m_pHL.push_back(&(*it));
            foundM = true;
        }
        else
        {
            vertex *v1=it->GetV1();
            vertex *v2=it->GetV2();
            
            std::vector<links*>  lList = v2->GetVLinkList();
            for (std::vector<links*>::iterator it2 = lList.begin() ; it2 != lList.end(); ++it2)
            {
                if(((*it2)->GetV2())->GetVID()==v1->GetVID())
                {
                    it->UpdateMirrorLink((*it2));
                    (*it2)->UpdateMirrorLink(&(*it));
                    it->UpdateMirrorFlag(true);
                    (*it2)->UpdateMirrorFlag(true);
                    foundM = true;
                    break;
                }
            }
        }
        if(foundM == false)
        {
            std::string A=" Warning : This system does not look like a closed system or the triangles orination are not consistent ";
            std::cout<<A<<"\n";
        }
        
    }
    int nomirror=0;
    for (std::vector<links>::iterator it = m_Links.begin() ; it != m_Links.end(); ++it)
    {
        if(it->GetMirrorFlag()==false)
        {
            nomirror++;
        }
        
    }
    if(nomirror!=0)
    {
        std::cout<<nomirror<<"----> Error: links without mirror \n";
    }
    for (std::vector<links>::iterator it = m_Links.begin() ; it != m_Links.end(); ++it)
    {
        
        m_pLinks.push_back(&(*it));
    }
    for (std::vector<inclusion*>::iterator it = m_pInclusion.begin() ; it != m_pInclusion.end(); ++it)
    {
        ((*it)->Getvertex())->UpdateInclusion((*it));
        int inc_typeid=(*it)->GetInclusionTypeID();
        if(m_InclusionType.size()-1<inc_typeid)
        {
            std::cout<<" Error: inclusion with typeid of "<<inc_typeid<<" has not been defined \n";
            exit(0);
        }
        (*it)->UpdateInclusionType(&(m_InclusionType.at(inc_typeid)));
    }
}
MeshBluePrint MESH::Convert_Mesh_2_BluePrint(MESH *mesh)
{
    MeshBluePrint BluePrint;
    std::vector<Vertex_Map> bvertex;       // a vector of all vertices (only the blueprint not the object) in the mesh
    std::vector<Triangle_Map> btriangle;   // a vector of all triangles (only the blueprint not the object) in the mesh
    std::vector<Inclusion_Map> binclusion; // a vector of all inclusions (only the blueprint not the object) in the mesh
    std::vector <InclusionType> binctype;  // a vector containing all inclsuion type and a default one
    Vec3D simbox;
    
    // vertex member of the blue print
    std::vector<vertex*> pV = mesh->m_pAllV;
    for (std::vector<vertex *>::iterator it = pV.begin() ; it != pV.end(); ++it)
    {
        Vertex_Map tvm;
        tvm.x = (*it)->GetVXPos();
        tvm.y = (*it)->GetVYPos();
        tvm.z = (*it)->GetVZPos();
        tvm.id = (*it)->GetVID();
        tvm.domain = (*it)->GetGroup();
        bvertex.push_back(tvm);
    }
    // triangle map member of the blue print
    std::vector<triangle*> pT = mesh->m_pAllT;
    for (std::vector<triangle *>::iterator it = pT.begin() ; it != pT.end(); ++it)
    {
        Triangle_Map ttm;
        ttm.v1 = ((*it)->GetV1())->GetVID();
        ttm.v2 = ((*it)->GetV2())->GetVID();
        ttm.v3 = ((*it)->GetV3())->GetVID();
        ttm.id = (*it)->GetTriID();
        btriangle.push_back(ttm);
    }
    
    // inclusion map member of the blue print
    std::vector<inclusion*> pInc = mesh->m_pInclusion;
    for (std::vector<inclusion *>::iterator it = pInc.begin() ; it != pInc.end(); ++it)
    {
        Inclusion_Map tim;
        tim.x = ((*it)->GetLDirection())(0);
        tim.y = ((*it)->GetLDirection())(1);
        tim.vid = ((*it)->Getvertex())->GetVID();
        tim.tid = ((*it)->GetInclusionType())->ITid;
        tim.id = ((*it)->GetID());
        binclusion.push_back(tim);

    }
    // inclusion type map member of the blue print
    BluePrint.binctype = mesh->m_InclusionType;
    // Add other map into the mesh map
    BluePrint.bvertex = bvertex;
    BluePrint.btriangle = btriangle;
    BluePrint.binclusion = binclusion;
    BluePrint.simbox = *(mesh->m_pBox);
    
    return BluePrint;
}
