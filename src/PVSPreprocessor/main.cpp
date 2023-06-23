/**
* Tool to compute the PVS from an input file with the scene description
*/

#include <iostream>
#include <vector>
#include <string>
#include "..\JanuaEngine\PVSPreprocessor.h"
#include "..\Core\Vector3f.h"

/**
* -inputPath "C:\folder\myFile.xml" -outputPath "C:\folder\output.lrb" -voxelSize "10.5;10.5;10.5" -sceneName "myScene" -maxCellSize "100;100;100"
*/
int main(int argc, char* argv[])
{
	try {
		//Para debuggear en VS: -inputPath $(SolutionDir)\tgcviewer-cpp\Media\Examples\TgcModels\NormalMapRoom2\NormalMapRoom2-TgcScene.xml -outputPath %cd%\pvs.lrb -voxelSize 10.5;10.5;10.5 -sceneName myScene -maxCellSize 100;100;100

		std::cout << "PVS Pre-processor \n";

		//Check arguments count
		if(argc != 13)
		{
			std::cout << "USAGE: -inputPath C:\\folder\\myFile.xml -outputPath C:\\folder\\output.lrb -voxelSize 10.5;10.5;10.5 -sceneName myScene -maxCellSize 100;100;100 -tileSize 10;10;10";
			exit(0);
		}

		PVSPreprocessor preprocessor;
		string inputPath;
		string arg;

		//Parse arguments
		for (int i = 1; i < argc; i++) 
		{
			arg = argv[i];

			if (arg == "-inputPath")
			{
				inputPath = argv[i + 1];
			}
			else if(arg == "-outputPath")
			{
				preprocessor.outputPath = argv[i + 1];
			}
			else if(arg == "-voxelSize")
			{
				vector<string> tokens = PVSPreprocessor::split(argv[i + 1], ';');
				if(tokens.size() != 3)
				{
					std::cerr << "The voxelSize is incorrect";
					exit(1);
				}
				preprocessor.voxelSize.x = (float)atof(tokens[0].c_str());
				preprocessor.voxelSize.y = (float)atof(tokens[1].c_str());
				preprocessor.voxelSize.z = (float)atof(tokens[2].c_str());
			}
			else if(arg == "-sceneName")
			{
				preprocessor.sceneName = argv[i + 1];
			}
			else if(arg == "-maxCellSize")
			{
				vector<string> tokens = PVSPreprocessor::split(argv[i + 1], ';');
				if(tokens.size() != 3)
				{
					std::cerr << "The maxCellSize is incorrect";
					exit(1);
				}
				preprocessor.maxCellSize.x = (float)atof(tokens[0].c_str());
				preprocessor.maxCellSize.y = (float)atof(tokens[1].c_str());
				preprocessor.maxCellSize.z = (float)atof(tokens[2].c_str());
			}
			else if(arg == "-tileSize")
			{
				vector<string> tokens = PVSPreprocessor::split(argv[i + 1], ';');
				if(tokens.size() != 3)
				{
					std::cerr << "The tileSize is incorrect";
					exit(1);
				}
				preprocessor.tileSize.x = (float)atof(tokens[0].c_str());
				preprocessor.tileSize.y = (float)atof(tokens[1].c_str());
				preprocessor.tileSize.z = (float)atof(tokens[2].c_str());
			}
		}

		//Import models data
		std::cout << "Importing file: " + inputPath + "\n";
		
		std::string fileFormat = "";
		if(inputPath.find_last_of('.') != -1) fileFormat = inputPath.substr(inputPath.find_last_of('.'), inputPath.size() - inputPath.find_last_of('.'));
		std::cout << fileFormat << "\n";
		
		if(fileFormat == ".obj") 
		{
			if(!preprocessor.addMeshesFromObjFile(inputPath))
			{
				std::cerr << preprocessor.getErrors();
				exit(1);
			}

		} 
		else 
		{
			if(!preprocessor.addMeshesFromXmlFile(inputPath))
			{
				std::cerr << preprocessor.getErrors();
				exit(1);
			}
		}

		//Build PVS
		std::cout << "Building PVS... \n";
		preprocessor.buildPVS();
		preprocessor.dispose();

		std::cerr << "PVS completed. Results outputed in: " + preprocessor.outputPath + "\n";

		return 0;
	} catch(logic_error* err) {
		std::cerr << "Error happened ... " << err->what() << std::endl;
		return 1; 
	}
}