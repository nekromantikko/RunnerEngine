#include "model.h"
#include <iostream>
#include <fstream>
#include <sstream>

#include "resource.h"
#include "Platform/platform.h"

Model::~Model()
{
    platform_delete_vertex_array(vbuffer);
    std::cout << "Model <" << this << "> destroyed!" << std::endl;
}
/*
bool Model::load(std::string fname)
{
    std::ifstream file(fname.c_str());
    if (!file.is_open())
    {
        std::cout << "Error loading mdl-file!\n";
        Resource::throw_file_error("Cannot load model", fname);
        return false;
    }

    Mesh mesh;

    //read file to the end
    while (!file.eof())
    {
        std::string line;
        std::getline(file, line);
        std::string firstWord;
        std::stringstream sstream(line);
        sstream >> firstWord;

        //load vertex coordinate
        if (firstWord.compare("texture") == 0)
        {
            std::string fmat;
            sstream >> fmat;
            texture = Resource::get_texture(fmat);
        }
        //load mesh
        else if (firstWord.compare("mesh") == 0)
        {
            std::string fmesh;
            sstream >> fmesh;
            std::stringstream filu;
            filu << "res/models/" << fmesh << ".obj";
            std::string filustr = filu.str();
            mesh = platform_load_mesh(filustr.c_str());

        }
        else if (firstWord.compare("glow") == 0)
            sstream >> glow;
    }
    file.close();

    vbuffer = platform_create_vertex_buffer(&mesh, 1);

    return true;
}
*/
