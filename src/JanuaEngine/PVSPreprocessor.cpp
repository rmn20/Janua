#include "StdAfx.h"
#include "PVSPreprocessor.h"


PVSPreprocessor::PVSPreprocessor()
{

}


bool PVSPreprocessor::addMeshesFromXmlFile(const string inputPath)
{
	//Load XML file
	xml_document doc;
	xml_parse_result result = doc.load_file(inputPath.c_str());
	if(!result)
	{
		errorMessages.push_back("Cannot load XML file: " + inputPath);
		errorMessages.push_back(result.description());
		return false;
	}
	xml_node root = doc.child("janua_scene");
	if(root.empty())
	{
		errorMessages.push_back("The root node [janua_scene] of the XML is empty.");
		return false;
	}

	//Load meshes
	xml_node meshesNode = root.child("meshes");
	if(meshesNode.empty())
	{
		errorMessages.push_back("The \"meshes\" node of the XML is empty.");
		return false;
	}
    int i = 0;
	unordered_map<unsigned int, unsigned int> idMap;
	for (xml_node meshNode = meshesNode.first_child(); meshNode; meshNode = meshNode.next_sibling())
    {
		MeshData meshData;

		//id
		string idStr = meshNode.attribute("id").as_string();
		meshData.id = meshNode.attribute("id").as_uint();

		//Check unique
		unordered_map<unsigned int, unsigned int>::iterator  it = idMap.find(meshData.id);
		if (it != idMap.end())
		{
			errorMessages.push_back("The ID: " + idStr + " is duplicated.");
			continue;
		}
		idMap[meshData.id] = meshData.id;

		//type
		string type = meshNode.attribute("type").as_string();
		if(type == "OCCLUDER")
		{
			meshData.type = OCCLUDER;
		}
		else if(type == "OCCLUDEE")
		{
			meshData.type = OCCLUDEE;
		}
		else
		{
			errorMessages.push_back("Bad mesh type for ID: " + idStr);
			continue;
		}

		//triCount
		string triCountStr = meshNode.attribute("triCount").as_string();
		meshData.trianglesCount = meshNode.attribute("triCount").as_uint();
		unsigned int vertexValuesCount = meshData.trianglesCount * 3 * 3;
		meshData.vertexData = new float[vertexValuesCount];

		//vertex values
		string vertexValues = meshNode.child_value();
		int start = 0, end = 0;
		string floatStr;
		float coordValue;
		int currentVertIdx = 0;
		while ((end = vertexValues.find(' ', start)) != string::npos) 
		{
			floatStr = vertexValues.substr(start, end - start);
			coordValue = (float)atof(floatStr.c_str());
			meshData.vertexData[currentVertIdx++] = coordValue;
			start = end + 1;

			if(currentVertIdx >= (int)vertexValuesCount)
			{
				errorMessages.push_back("The mesh with ID: " + idStr + " has more vertices than the triCount specified [" + triCountStr + "]");
				currentVertIdx = -1;
				break;
			}
		}
		if(currentVertIdx < 0)
		{
			continue;
		}
		floatStr = vertexValues.substr(start);
		coordValue = (float)atof(floatStr.c_str());
		meshData.vertexData[currentVertIdx++] = coordValue;

		//check amounts
		if(currentVertIdx != vertexValuesCount)
		{
			errorMessages.push_back("The triCount [" + triCountStr + "] for the mesh with ID: " + idStr + " does not match with amount of coordinates [" + toString((int)vertexValuesCount) + "]");
			continue;
		}

		meshes.push_back(meshData);
	}



	return true;
}

void PVSPreprocessor::loadObjVertex(char* line, vector<float> &vertices, size_t attribs) {
	char* token = strtok(line, " ");
	size_t tokens = 0;
	
    while(token) {
        if(tokens > 0 && tokens-1 < attribs) vertices.push_back((float) atof(token));
        token = strtok(NULL, " ");
		tokens++;
    }
}

void PVSPreprocessor::loadObjFace(char* line, vector<float> &vertices, vector<float> &meshVerts) {
	char* token = strtok(line, " ");
	size_t tokens = 0;
	size_t vert = 0;
	
    while(token) {
		
        if(tokens > 0) {
			int v = 0, uv = 0, n = 0;
			
			char* tmp = token;
			size_t attrib = 0;
			
			while(tmp) {
				if(attrib == 0) v = atoi(tmp);
				else if(attrib == 1) uv = atoi(tmp);
				else if(attrib == 2) n = atoi(tmp);
				
				attrib++;
				tmp = strchr(tmp, '/');
				if(tmp) tmp++;
			}
			if(vert >= 3) {
				//polygon triangulation!
				for(int i=0; i<3; i++) meshVerts.push_back(meshVerts[meshVerts.size() - 3*vert]);
				for(int i=0; i<3; i++) meshVerts.push_back(meshVerts[meshVerts.size() - 6]);

				vert + 2;
			}
			
			if(v < 0) {
				for(int i=0; i<3; i++) meshVerts.push_back(vertices[vertices.size() + v * 3 + i]);
			} if(v > 0) {
				for(int i=0; i<3; i++) meshVerts.push_back(vertices[(v-1) * 3 + i]);
			} else {
				errorMessages.push_back("Undefined behavior in wavefront obj loader, vertex ID can't be zero!");
			}
			
			vert++;
		}
		
        token = strtok(NULL, " ");
		tokens++;
    }
}

bool PVSPreprocessor::addMeshesFromObjFile(const string inputPath)
{
	FILE* f = fopen(inputPath.c_str(), "rt");
	if(!f) {
		errorMessages.push_back("Can't open file");
		
		return false;
	}
	
	char line[1024] = {'\0'};
	
	if(!line) {
		errorMessages.push_back("Can't allocate line");
		fclose(f);
		
		return false;
	}
	
	vector<float> vertices;
	
	MeshData meshData;
	meshData.id = 0;
	meshData.type = OCCLUDER;
	
	vector<float> meshVerts;
	
	Vector3f mins(FLT_MAX);
	Vector3f maxs(FLT_MIN);
	
	while(fgets(line, 1024, f)) {
		size_t size = strlen(line) - 1; //skip new line character
		if(!size) continue;
		
		if(strstr(line, "o ") == line || strstr(line, "g ") == line) {
			
			if(meshVerts.size() > 0) {
				meshData.trianglesCount = meshVerts.size() / 9;
	
				for(int i=0; i<meshVerts.size(); i+=3) {
					mins.x = min(meshVerts[i + 0], mins.x);
					mins.y = min(meshVerts[i + 1], mins.y);
					mins.z = min(meshVerts[i + 2], mins.z);

					maxs.x = max(meshVerts[i + 0], maxs.x);
					maxs.y = max(meshVerts[i + 1], maxs.y);
					maxs.z = max(meshVerts[i + 2], maxs.z);
				}
				
				meshData.vertexData = new float[meshVerts.size()];
				memcpy(meshData.vertexData, meshVerts.data(), meshVerts.size() * sizeof(float));
				meshVerts.clear();
				
				meshes.push_back(meshData);
				meshData.id++;
				
				meshData.trianglesCount = 0;
				meshData.vertexData = NULL;
			}
			
			
		} else if(strstr(line, "v ") == line) {
			loadObjVertex(line, vertices, 3);
		} else if(strstr(line, "f ") == line) {
			loadObjFace(line, vertices, meshVerts);
		}
		
	}
	
	if(meshVerts.size() > 0) {
		meshData.trianglesCount = meshVerts.size() / 9;
	
		for(int i=0; i<meshVerts.size(); i+=3) {
			mins.x = min(meshVerts[i + 0], mins.x);
			mins.y = min(meshVerts[i + 1], mins.y);
			mins.z = min(meshVerts[i + 2], mins.z);

			maxs.x = max(meshVerts[i + 0], maxs.x);
			maxs.y = max(meshVerts[i + 1], maxs.y);
			maxs.z = max(meshVerts[i + 2], maxs.z);
		}
		
		meshData.vertexData = new float[meshVerts.size()];
		memcpy(meshData.vertexData, meshVerts.data(), meshVerts.size() * sizeof(float));
		meshVerts.clear();
		
		meshes.push_back(meshData);
		meshData.id++;
	}
	
	cout << "Min: " << mins.x << " " << mins.y << " " << mins.z << "\n";
	cout << "Max: " << maxs.x << " " << maxs.y << " " << maxs.z << "\n";
	
	fclose(f);

	return true;
}

string PVSPreprocessor::toString(const int n)
{
	stringstream ss;
	ss << n;
	return ss.str();
}


void PVSPreprocessor::buildPVS()
{
	//Create scene
	SceneOptions ocSceneOptions;
	ocSceneOptions.setMaxCellSize((int)maxCellSize.x, (int)maxCellSize.y, (int)maxCellSize.z);
	ocSceneOptions.setSceneTileSize((int)tileSize.x, (int)tileSize.y, (int)tileSize.z);
	ocSceneOptions.setVoxelSize(voxelSize.x,voxelSize.y, voxelSize.z);
	Scene ocScene = Scene(ocSceneOptions, sceneName);

	//Add meshes to scene
	cout << "Adding meshes\n";
	vector<Model*> ocModels;
	Matrix4x4 identityMat;
	for(unsigned int i = 0; i < meshes.size(); i++)
	{
		MeshData meshData = meshes[i];

		//Sequential index data
		int indexCount = meshData.trianglesCount * 3;
		int* indexData = new int[indexCount];
		for(int i = 0; i < indexCount; i++)
		{
			indexData[i] = i;
		}

		//Create model
		Model* ocModel = new Model(meshData.vertexData, indexCount, indexData, meshData.trianglesCount);
		
		//Add model to scene
		ocModels.push_back(ocModel);
		ocScene.addModelInstance((*ocModel), meshData.id, identityMat, meshData.type);

		delete[] indexData;
	}

	//Generate voxels from the scene
	cout << "Generating voxels\n";
 	PVSGenerator gen = PVSGenerator(ocScene);
	cout << "Generating database\n";
	shared_ptr<PVSDatabase> ocDatabase = gen.generatePVSDatabase();

	//Export to file
	cout << "Exporting\n";
	PVSDatabaseExporter dbExporter(*ocDatabase);
	int allocatedSize = dbExporter.getBufferSize();
	char *buffer = new char[allocatedSize];
	dbExporter.saveToBuffer(buffer);
	FILE *file = fopen(outputPath.c_str(), "wb");
	fwrite( buffer, 1, allocatedSize, file);
	fclose(file);
	delete [] buffer;

}


void PVSPreprocessor::dispose()
{
	for(unsigned int i = 0; i < meshes.size(); i++)
	{
		delete[] meshes[i].vertexData;
	}
}

string PVSPreprocessor::getErrors()
{
	string errors = "Errors (" + toString((int)errorMessages.size()) + "): \n";
	for(unsigned int i = 0; i < errorMessages.size(); i++)
	{
		errors += " - " + errorMessages[i] + "\n";
	}
	return errors;
}

vector<string> PVSPreprocessor::split(const string &text, char sep) 
{
	vector<string> tokens;
	int start = 0, end = 0;
	while ((end = text.find(sep, start)) != string::npos) 
	{
		tokens.push_back(text.substr(start, end - start));
		start = end + 1;
	}
	tokens.push_back(text.substr(start));
	return tokens;
}