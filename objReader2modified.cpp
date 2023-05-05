#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <strstream>
using namespace std;

/*
 * Code to read in a simple Wavefront OBJ file.  The code will read in a file with only 
 * vertices and faces.  (NO normals).  The user needs to pass a string containing the 
 * filename and a variable to hold the number of triangles (an integer).  It returns a
 * pointer to an array containing the triangle information and the number of triangles 
 * should be stored in the second argument.
 *
 * Author:  Jerry Heuring
 * Date:    October 23, 2014
 *
 * Revisions:
 *
 * Bugs:
 *    Bad reads of data are not caught in some cases.  I have had an instance where the
 *    normal data was included and the routine crashed trying to use the vector of information.
 *
 */
float *readOBJFile (string filename, int &nbrTriangles, float * &normalArray) {
	/*
	 * I'm going to use vectors to hold the items I read in.
	 * this is not terribly efficient but I'm striving for simplicity here
	 * rather than efficiency.
	 */
	vector<float *> vertices;
	vector<int *> triangles;
	vector<float *> normals;
	vector<int *> triangleNormals;

	string currentLine, lineType;
	istrstream *lineRdr;
	float *vertex, *normal;
	int *triangle;
	int *triangleNormal;
	char slash;
	float value;

	nbrTriangles = 0;
	/*
	 * Open the fail -- routine returns null if it fails
	 */
	ifstream infile;
	infile.open(filename.c_str());
	if (infile.fail()) {
		cerr << "Failed to open \"" << filename << "\" for input\n";
		return NULL;
	}
	/*
	 * Put an dummy vertex in the zero spot in the vector.  Then I don't need
	 * to adjust the indices in the OBJ file.
	 */
	vertex = new float[4];
	vertex[0] = vertex[1] = vertex[2] = 0.0f;
	vertex[3] = 1.0f;
	vertices.push_back(vertex); // put an empty entry in 0 so that we don't need to adjust 
	normal = new float[3]; normal[0] = 1.0; normal[1] =normal[2] = 0.0;
	normals.push_back(normal);
	while (!infile.fail() && !infile.eof()) { // read data in...
		getline(infile, currentLine);
		if (infile.fail() || infile.eof()) break;
		lineRdr = new istrstream(currentLine.c_str());
		*lineRdr >> lineType;
		if (lineType == "v") { // process the vertex
			vertex = new float[4];
			*lineRdr >> vertex[0] >> vertex[1] >> vertex[2];  // should check for errors.
			vertex[3] = 1.0f;
			vertices.push_back(vertex);
		} else if (lineType == "vn") {
			normal = new float[3];
			*lineRdr >> normal[0] >> normal[1] >> normal[2];
			normals.push_back(normal);
		} else if (lineType == "f" ) { // process the face
			triangle = new int [3];

			*lineRdr >> triangle[0];
			if (lineRdr->peek() == '/') { // assume we have normals
			triangleNormal = new int [3];
			
			*lineRdr >> slash;
			if (lineRdr->peek() == '/') { // assume we have no textures
				*lineRdr >> slash >> triangleNormal[0] >> triangle[1] >> slash >> slash >> triangleNormal[1] >> triangle[2] >> slash >> slash >> triangleNormal[2];
			} else { // assume we have textures...
				int tex0, tex1, tex2;
				*lineRdr >> tex0 >> slash >> triangleNormal[0] >> triangle[1] >> slash >> tex1 >> slash >> triangleNormal[1] >> triangle[2] >> slash >> tex2 >> slash >> triangleNormal[2];
			}
				triangles.push_back(triangle);
				triangleNormals.push_back(triangleNormal);
			} else {
				*lineRdr >> triangle[1] >> triangle[2];  // should check for errors.
				triangles.push_back(triangle);
			}
			
		}
		delete lineRdr;  // ought to reinitialize it rather than create a new one.  
	}
	/*
	 * Build an array to hold the coordinates.  Remember everything
	 * is a triangle.
	 */
	float * triangleVertices = new float [triangles.size()*3 * 4];
	nbrTriangles = triangles.size();
	normalArray = new float[triangles.size() * 3 * 3];

	float *normalVector;
	for (int i = 0; i < triangles.size(); i++) {
		triangle = triangles[i];
		if (i < triangleNormals.size()) {
			triangleNormal = triangleNormals[i];
		} else {
			triangleNormal = NULL;
		}
		for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 4; k++) {
				if (triangleNormal != NULL && k != 3) {
					normalVector = normals[triangleNormal[j]];
					value = normalVector[k];
					normalArray[((i * 3) + j) * 3 + k] = value;
				}
				triangleVertices[i * 12 + j * 4 + k] = (vertices[triangle[j]])[k];
			}
		}
		delete [] triangle;
//		delete [] normal;
	}
	/*
	 * finish the clean up....
	 */
	for (int i = 0; i < vertices.size(); i++) {
		delete [] vertices[i];
	}
	/*
	 * Return the pointer to the array.
	 */
	return triangleVertices;

}

float* readOBJFile(string filename, int& nbrTriangles, float*& normalArray, float *& textureCoordArray) {
	/*
	 * I'm going to use vectors to hold the items I read in.
	 * this is not terribly efficient but I'm striving for simplicity here
	 * rather than efficiency.
	 */
	vector<float*> vertices;
	vector<int*> triangles;
	vector<float*> normals;
	vector<int*> triangleNormals;
	vector<float*> textureCoords;
	vector<int*> textureIndices;

	string currentLine, lineType;
	istrstream* lineRdr;
	float* vertex, * normal, *tc;
	int* triangle;
	int* triangleNormal;
	int* tex;
	char slash;
	float value;

	nbrTriangles = 0;
	/*
	 * Open the fail -- routine returns null if it fails
	 */
	ifstream infile;
	infile.open(filename.c_str());
	if (infile.fail()) {
		cerr << "Failed to open \"" << filename << "\" for input\n";
		return NULL;
	}
	/*
	 * Put an dummy vertex in the zero spot in the vector.  Then I don't need
	 * to adjust the indices in the OBJ file.
	 */
	vertex = new float[4];
	vertex[0] = vertex[1] = vertex[2] = 0.0f;
	vertex[3] = 1.0f;
	vertices.push_back(vertex); // put an empty entry in 0 so that we don't need to adjust 
	normal = new float[3]; normal[0] = 1.0; normal[1] = normal[2] = 0.0;
	normals.push_back(normal);
	float* textureCoord = new float[2];
	textureCoord[0] = textureCoord[1] = 0.0;
	textureCoords.push_back(textureCoord);

	while (!infile.fail() && !infile.eof()) { // read data in...
		getline(infile, currentLine);
		if (infile.fail() || infile.eof()) break;
		lineRdr = new istrstream(currentLine.c_str());
		*lineRdr >> lineType;
		if (lineType == "v") { // process the vertex
			vertex = new float[4];
			*lineRdr >> vertex[0] >> vertex[1] >> vertex[2];  // should check for errors.
			vertex[3] = 1.0f;
			vertices.push_back(vertex);
		}
		else if (lineType == "vn") {
			normal = new float[3];
			*lineRdr >> normal[0] >> normal[1] >> normal[2];
			normals.push_back(normal);
		}
		else if (lineType == "vt") {
			tc = new float[2];
			*lineRdr >> tc[0] >> tc[1]; // get two texture coordinates 
			textureCoords.push_back(tc);
		} else if (lineType == "f") { // process the face
			triangle = new int[3];

			*lineRdr >> triangle[0];
			if (lineRdr->peek() == '/') { // assume we have normals
				triangleNormal = new int[3];

				*lineRdr >> slash;
				if (lineRdr->peek() == '/') { // assume we have no textures
					*lineRdr >> slash >> triangleNormal[0] >> triangle[1] >> slash >> slash >> triangleNormal[1] >> triangle[2] >> slash >> slash >> triangleNormal[2];
				}
				else { // assume we have textures...
					tex = new int[3];
					*lineRdr >> tex[0] >> slash >> triangleNormal[0] >> triangle[1] >> slash >> tex[1] >> slash >> triangleNormal[1] >> triangle[2] >> slash >> tex[2] >> slash >> triangleNormal[2];
				}
				triangles.push_back(triangle);
				triangleNormals.push_back(triangleNormal);
				textureIndices.push_back(tex);
			}
			else {
				*lineRdr >> triangle[1] >> triangle[2];  // should check for errors.
				triangles.push_back(triangle);
			}

		}
		delete lineRdr;  // ought to reinitialize it rather than create a new one.  
	}
	/*
	 * Build an array to hold the coordinates.  Remember everything
	 * is a triangle.
	 */
	float* triangleVertices = new float[triangles.size() * 3 * 4];
	nbrTriangles = triangles.size();
	normalArray = new float[triangles.size() * 3 * 3];
	textureCoordArray = new float[triangles.size() * 3 * 2];

	float* normalVector;
	for (int i = 0; i < triangles.size(); i++) {
		triangle = triangles[i];
		if (i < triangleNormals.size()) {
			triangleNormal = triangleNormals[i];
		}
		else {
			triangleNormal = NULL;
		}
		for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 4; k++) {
				if (triangleNormal != NULL && k != 3) {
					normalVector = normals[triangleNormal[j]];
					value = normalVector[k];
					normalArray[((i * 3) + j) * 3 + k] = value;
				}
				triangleVertices[i * 12 + j * 4 + k] = (vertices[triangle[j]])[k];
			}
			int * textureInfo = textureIndices[i];  // processing i'th texture pair.  
				for (int item = 0; item < 2; item++) { // assuming pairs of coordinates
					textureCoordArray[i * 6 + j * 2 + item] = (textureCoords[textureInfo[j]])[item];
				}
		}
		delete[] triangle;
		//		delete [] normal;
	}
	/*
	 * finish the clean up....
	 */
	for (int i = 0; i < vertices.size(); i++) {
		delete[] vertices[i];
	}
	/*
	 * Return the pointer to the array.
	 */
	return triangleVertices;

}