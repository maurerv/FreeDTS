

/*
 Weria Pezeshkian (weria.pezeshkian@gmail.com)
 Copyright (c) Weria Pezeshkian
 */
#include <fstream>
#include "Restart.h"
#include "State.h"
#include "Nfunction.h"
#include "CreateMashBluePrint.h"
Restart::Restart()
{
}
Restart::Restart(State *pState)
{
    m_pState = pState;
    m_CopyFileName = "copyrestart-1.res";
}
Restart::~Restart()
{
    
}
//=== Writing a restart file: this file is just the active [State] Object with an updated initial step
void Restart::WrireRestart(int step, std::string filename, MESH * pmesh, double r, double rb)
{
    MeshBluePrint blueprint = pmesh->Convert_Mesh_2_BluePrint(pmesh);
    std::string restartfilename;
    std::string ext = filename.substr(filename.find_last_of(".") + 1);
    if(ext!=RestartExt)
     restartfilename = filename+"."+RestartExt;
#if TEST_MODE == Enabled
    std::cout<<"----> Note: restart file name for writing "<<restartfilename<<std::endl;
#endif
    {// first make a copy of an existing restart file incase we fail to update restart file
        std::string f1=restartfilename;
        std::string f2=m_CopyFileName;
        std::ifstream file(f1.c_str(),std::ios::binary);
        if(file)
        CopyBinaryFile(f1,f2);
        file.close();
    }
    //=== Since we have a copy of the restart file, we copy the active State object into the the file
    std::fstream Rfile;
    Rfile.open(restartfilename.c_str(),std::ios::out | std::ios::binary);
    (Rfile).write((char *) &step, sizeof(int));
     m_pState->m_Initial_Step = step+1;
    
    (Rfile).write((char *) &r, sizeof(double));
    (Rfile).write((char *) &rb, sizeof(double));
    Vec3D box = blueprint.simbox;
    (Rfile).write((char *) &(box(0)), sizeof(double));
    (Rfile).write((char *) &(box(1)), sizeof(double));
    (Rfile).write((char *) &(box(2)), sizeof(double));

    int size = (blueprint.bvertex).size();
    (Rfile).write((char *) &size, sizeof(int));
    for (std::vector<Vertex_Map>::iterator it = (blueprint.bvertex).begin() ; it != (blueprint.bvertex).end(); ++it)
        (Rfile).write((char *) &(*it), sizeof(Vertex_Map));
    size = (blueprint.btriangle).size();
    (Rfile).write((char *) &size, sizeof(int));
    for (std::vector<Triangle_Map>::iterator it = (blueprint.btriangle).begin() ; it != (blueprint.btriangle).end(); ++it)
        (Rfile).write((char *) &(*it), sizeof(Triangle_Map));
    size = (blueprint.binclusion).size();
    (Rfile).write((char *) &size, sizeof(int));
    for (std::vector<Inclusion_Map>::iterator it = (blueprint.binclusion).begin() ; it != (blueprint.binclusion).end(); ++it)
        (Rfile).write((char *) &(*it), sizeof(Inclusion_Map));
    size = (blueprint.binctype).size();
    (Rfile).write((char *) &size, sizeof(int));
    for (std::vector<InclusionType>::iterator it = (blueprint.binctype).begin() ; it != (blueprint.binctype).end(); ++it)
        (Rfile).write((char *) &(*it), sizeof(InclusionType));
    Rfile.close();
    
    remove(m_CopyFileName.c_str());

}
//=== Read a restart file and load to the  active [State] Object
MeshBluePrint Restart::ReadRestart(std::string filename , bool *readok)
{
    *readok = false;
    MeshBluePrint blueprint;
    std::string restartfilename;
    std::ifstream f(m_CopyFileName.c_str());
    if(f.good())
        restartfilename = m_CopyFileName;
    else
        restartfilename = filename;
    std::string ext = filename.substr(filename.find_last_of(".") + 1);
    if(ext!=RestartExt)
        restartfilename = filename+"."+RestartExt;

#if TEST_MODE == Enabled
    std::cout<<"----> Note: restart file name for reading "<<restartfilename<<std::endl;
#endif
    // We should make a check and see if the restart file is writtn correctly or not
    //=== just read the restart file and copy it into the current active [State] object
    std::fstream Rfile;
    Rfile.open(restartfilename.c_str(), std::ios::in |std::ios::binary);
if(Rfile.is_open())
{
    int step = 44 ;
    Rfile.read((char *) &step, sizeof(int));
    m_pState->m_Initial_Step = step+1;
#if TEST_MODE == Enabled
    std::cout<<"----> Note: The saved restart was at step "<<step<<std::endl;
#endif
    double r,rb,lx,ly,lz;
    Rfile.read((char *) &r, sizeof(double));
    Rfile.read((char *) &rb, sizeof(double));
    Rfile.read((char *) &lx, sizeof(double));
    Rfile.read((char *) &ly, sizeof(double));
    Rfile.read((char *) &lz, sizeof(double));

    m_pState->m_R_Vertex = r;
    m_pState->m_R_Box = rb;
    Vec3D box(lx,ly,lz);
    blueprint.simbox= box;
    std::vector<Vertex_Map> bvertex;       // a vector of all vertices (only the blueprint not the object) in the mesh
    std::vector<Triangle_Map> btriangle;   // a vector of all triangles (only the blueprint not the object) in the mesh
    std::vector<Inclusion_Map> binclusion; // a vector of all inclusions (only the blueprint not the object) in the mesh
    std::vector <InclusionType> binctype;  // a vector containing all inclsuion type and a default one
    int size;
    Rfile.read((char *) &size, sizeof(int));
    for (int i=0;i<size;i++)
    {
        Vertex_Map vmap;
        (Rfile).read((char *) &vmap, sizeof(Vertex_Map));
        bvertex.push_back(vmap);
    }
    Rfile.read((char *) &size, sizeof(int));
    for (int i=0;i<size;i++)
    {
        Triangle_Map tmap;
        (Rfile).read((char *) &tmap, sizeof(Triangle_Map));
        btriangle.push_back(tmap);
    }
    Rfile.read((char *) &size, sizeof(int));
    for (int i=0;i<size;i++)
    {
        Inclusion_Map incmap;
        (Rfile).read((char *) &incmap, sizeof(Inclusion_Map));
        binclusion.push_back(incmap);
    }
    Rfile.read((char *) &size, sizeof(int));
    for (int i=0;i<size;i++)
    {
        InclusionType inctype;
        (Rfile).read((char *) &inctype, sizeof(InclusionType));
        binctype.push_back(inctype);
    }
    
    blueprint.binctype = binctype;
    blueprint.bvertex = bvertex;
    blueprint.btriangle = btriangle;
    blueprint.binclusion = binclusion;

    Rfile.close();
    *readok = true;

}
    return blueprint;
}
void Restart::CopyBinaryFile(std::string file1, std::string file2)
{
//==============================================================================
//=========== This function copies file1 into file2 while both are in binary mode
    std::ifstream in(file1.c_str(),std::ios::binary);
    std::ofstream out(file2.c_str(),std::ios::binary);
    
    if(in.is_open() && out.is_open())
    {
        while(!in.eof())
        {
            out.put(in.get());
        }
    }
    
    //Close both files
    in.close();
    out.close();
}
