#include <openvdb/openvdb.h>

#include "MorphOperations.h"
#include "MeshOperations.h"
#include "Container.hpp"

#include "CommonOperations.h"
#include <openvdb/tools/VolumeToMesh.h>

int main()
{
    openvdb::initialize();
    
    DIR *dir;
    struct dirent *ent;
    std::string file_name = "", table = "", morph_path = "";
    std::string curr_obj = "cylinder.stl.obj",
    curr_name = "cylinder.vdb",
    curr_name_wo_ext = "cylinder",
    curr_path = "morphs/cylinder/",
    obj_path = "original_objs/",
    vdb_path = "vdbs/";
    
    
    
    CommonOperations::makeDirs(curr_path.c_str());
    CommonOperations::makeDirs(obj_path.c_str());
    CommonOperations::makeDirs(vdb_path.c_str());
    
    openvdb::FloatGrid::Ptr source_grid;
    openvdb::FloatGrid::Ptr target_grid;
    std::ofstream file;
    std::string ignore_arr[40] = {
        curr_obj,
        //        "beast.stl.obj",
        "cylinder.stl.obj",
        "hand.stl.obj",
        "winding-wheel.stl.obj",
        ".DS_Store"
    };
    
    bool ignore = false;
    int source_nb = 3, target_nb = 30;
    double voxel_size = 0.01, dt = 0.25;
    double energy1 = 0, energy2 = 0, mean = 0;
    
    if ((dir = opendir ("original_objs")) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            ignore = false;
            for(int j = 0; j < sizeof(ignore_arr)/sizeof(ignore_arr[0]); j++)
                if(ignore_arr[j] == ent->d_name) ignore = true;
            if(ignore) continue;
            
            if(ent->d_type == DT_REG) {
                
                source_grid = nullptr;
                target_grid = nullptr;
                
                file_name = CommonOperations::getFileNameWithoutExtension(std::string(ent->d_name), ".stl");
                std::cout << file_name << std::endl;
                
                MeshOperations::doAllMeshOperations(obj_path + curr_obj, "srt1.obj");
                MeshOperations::doAllMeshOperations(obj_path + ent->d_name, "srt2.obj");
                
                MeshOperations::convertMeshToVolume("srt1.obj", curr_name, vdb_path, source_nb, voxel_size);
                MeshOperations::convertMeshToVolume("srt2.obj", file_name + ".vdb", vdb_path, target_nb, voxel_size);
                
                source_grid = openvdb::gridPtrCast<openvdb::FloatGrid>(GridOperations::readFile(vdb_path + curr_name));
                target_grid = openvdb::gridPtrCast<openvdb::FloatGrid>(GridOperations::readFile(vdb_path + file_name + ".vdb"));
                
                morph_path = curr_path + curr_name_wo_ext + "-" + file_name;
                
                table += "<tr><td colspan='3'><img src='" + file_name + ".png' /></td><td colspan='3'></td></tr>";
                table += "<tr><td colspan=6>" + curr_name_wo_ext + "-" + file_name + "</td></tr>";
                energy1 = MorphOperations::morphModels(source_grid, target_grid, dt, morph_path, table);
                table += "<td></td></tr>";
                
                source_grid = nullptr;
                target_grid = nullptr;
                
                MeshOperations::convertMeshToVolume("srt1.obj", curr_name, vdb_path, target_nb, voxel_size);
                MeshOperations::convertMeshToVolume("srt2.obj", file_name + ".vdb", vdb_path, source_nb, voxel_size);
                
                source_grid = openvdb::gridPtrCast<openvdb::FloatGrid>(GridOperations::readFile(vdb_path + file_name + ".vdb"));
                target_grid = openvdb::gridPtrCast<openvdb::FloatGrid>(GridOperations::readFile(vdb_path + curr_name));
                
                morph_path = curr_path + file_name + "-" + curr_name_wo_ext;
                
                table += "<tr><td colspan=6>" + file_name + "-" + curr_name_wo_ext + "</td></tr>";
                energy2 = MorphOperations::morphModels(source_grid, target_grid, dt, morph_path, table);
                mean = (energy1 + energy2) / 2;
                table += "<td>" + CommonOperations::intNumberFormatCommas(std::to_string(mean)) + "</td></tr>";
                
                std::cout << table << std::endl;
                file.open("table.txt");
                file << table;
                file.close();
            }
        }
        closedir (dir);
    } else {
        /* could not open directory */
        perror ("");
        return EXIT_FAILURE;
    }
    return 0;
}