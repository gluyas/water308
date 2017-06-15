//---------------------------------------------------------------------------
//
// Copyright (c) 2016 Taehyun Rhee, Joshua Scott, Ben Allen
//
// This software is provided 'as-is' for assignment of COMP308 in ECS,
// Victoria University of Wellington, without any express or implied warranty. 
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// The contents of this file may not be copied or duplicated in any form
// without the prior permission of its owner.
//
//----------------------------------------------------------------------------

#include <cmath>
#include <iostream> // input/output streams
#include <fstream>  // file streams
#include <sstream>  // string streams
#include <string>
#include <stdexcept>
#include <vector>

#include "cgra_math.hpp"
#include "geometry.hpp"
#include "opengl.hpp"

using namespace std;
using namespace cgra;


Geometry::Geometry(string filename ) {
	m_filename = filename;
	readOBJ(filename);
	if (m_triangles.size() > 0) {
		createDisplayListPoly();
		createDisplayListWire();
	}
}


Geometry::~Geometry() { }


void Geometry::readOBJ(string filename) {

	// Make sure our geometry information is cleared
	m_points.clear();
	m_uvs.clear();
	m_normals.clear();
	m_triangles.clear();

    bool hasUV = false;
    bool hasNormal = false;

	// Load dummy points because OBJ indexing starts at 1 not 0
	m_points.push_back(vec3(0,0,0));
	m_uvs.push_back(vec2(0,0));
	m_normals.push_back(vec3(0,0,1));

	ifstream objFile(filename);

	if(!objFile.is_open()) {
		cerr << "Error reading " << filename << endl;
		throw runtime_error("Error :: could not open file.");
	}

	cout << "Reading file " << filename << endl;
	
	// good() means that failbit, badbit and eofbit are all not set
	while(objFile.good()) {

		// Pull out line from file
		string line;
		std::getline(objFile, line);
		istringstream objLine(line);

		// Pull out mode from line
		string mode;
		objLine >> mode;

		// Reading like this means whitespace at the start of the line is fine
		// attempting to read from an empty string/line will set the failbit
		if (!objLine.fail()) {

			if (mode == "v") {
				vec3 v;
				objLine >> v.x >> v.y >> v.z;
				m_points.push_back(v);

			} else if(mode == "vn") {
				vec3 vn;
				objLine >> vn.x >> vn.y >> vn.z;
				m_normals.push_back(vn);
                hasNormal = true;

			} else if(mode == "vt") {
				vec2 vt;
				objLine >> vt.x >> vt.y;
                vt *= 5;
				m_uvs.push_back(vt);
                hasUV = true;

			} else if(mode == "f") {

				vector<vertex> verts;
				while (objLine.good()) {
                    vertex v;

                    //-------------------------------------------------------------
                    // [Assignment 1] :
                    // Modify the following to parse the bunny.obj. It has no uv
                    // coordinates so each vertex for each face is in the format
                    // v//vn instead of the usual v/vt/vn.
                    //
                    // Modify the following to parse the dragon.obj. It has no
                    // normals or uv coordinates so the format for each vertex is
                    // v instead of v/vt/vn or v//vn.
                    //
                    // Hint : Check if there is more than one uv or normal in
                    // the uv or normal vector and then parse appropriately.
                    //-------------------------------------------------------------

                    if (!hasUV) {
                        if (!hasNormal) {
                            objLine >> v.p;
                            verts.push_back(v);
                        } else {
                            objLine >> v.p;        // Scan in position index
                            objLine.ignore(2);    // Ignore the '//' characters
                            objLine >> v.n;        // Scan in normal index

                            verts.push_back(v);
                        }
                    } else {
                        // Assignment code (assumes you have all of v/vt/vn for each vertex)
                        objLine >> v.p;        // Scan in position index
                        objLine.ignore(1);    // Ignore the '/' character
                        objLine >> v.t;        // Scan in uv (texture coord) index
                        objLine.ignore(1);    // Ignore the '/' character
                        objLine >> v.n;        // Scan in normal index

                        verts.push_back(v);
                    }
				}

				// IFF we have 3 verticies, construct a triangle
				if(verts.size() == 3){
					triangle tri;
					tri.v[0] = verts[0];
					tri.v[1] = verts[1];
					tri.v[2] = verts[2];
					m_triangles.push_back(tri);

				}
			}
		}
	}
    //hasNormal = false;
    if (!hasNormal) { // make normals
//        m_normals.clear(); // debug code to test normal algorithm on any model
//        m_normals.push_back(vec3(0,0,1));
//        for (int i = 0; i < m_triangles.size(); i++) {
//            m_triangles[i].v[0].n = 0;
//            m_triangles[i].v[1].n = 0;
//            m_triangles[i].v[2].n = 0;
//        }
        cout << "Generating normals... ";
        createNormals();
        cout << "Done!" << endl;
    }

	cout << "Reading OBJ file is DONE." << endl;
	cout << m_points.size()-1 << " points" << endl;
	cout << m_uvs.size()-1 << " uv coords" << endl;
	cout << m_normals.size()-1 << " normals" << endl;
	cout << m_triangles.size() << " faces" << endl;
}


//-------------------------------------------------------------
// [Assignment 1] :
// Fill the following function to populate the normals for 
// the model currently loaded. Compute per face normals
// first and get that working before moving onto calculating
// per vertex normals.
//-------------------------------------------------------------
void Geometry::createNormals() {
    m_normals.resize(m_points.size()); // make normals vector same size as points
    //for (int i = 0; i < m_points.size(); i++) m_normals.push_back(vec3(0,0,0));

    // first take the sum of all surface normals of each vertex
    // assumes that there is only one instance of any vertex in the model
    for (int i = 0; i < m_triangles.size(); ++i) {
        triangle &t = m_triangles[i];

        // v1 = A -> B
        vec3 v1 = m_points[t.v[1].p];
        v1 -= m_points[t.v[0].p];

        // v2 = A -> C
        vec3 v2 = m_points[t.v[2].p];
        v2 -= m_points[t.v[0].p];

        vec3 normal = normalize(cross(v1, v2));

        for (int j = 0; j < 3; j++) {
            vertex &v = t.v[j];
            v.n = v.p;
            m_normals[v.p] += normal;
        }
    }
    // finally, normalise all to get the mean unit vector
    // although this appears to be unnecessary as gl seems to handle non-unit normals
    for (int i = 0; i < m_normals.size(); i++) {
        m_normals[i] = normalize(m_normals[i]);
        //normals << m_normals[i] << endl;
    }
}

double sin2(double x) {
    double res = sin(x);
    return res*res;
}

double cos2(double x) {
    double res = cos(x);
    return res*res;
}


//-------------------------------------------------------------
// [Assignment 1] :
// Fill the following function to create display list
// of the obj file to show it as wireframe model
//-------------------------------------------------------------
void Geometry::createDisplayListPoly() {
	// Delete old list if there is one
	if (m_displayListPoly) glDeleteLists(m_displayListPoly, 1);

	// Create a new list
	cout << "Creating Poly Geometry" << endl;
	m_displayListPoly = glGenLists(1);
	glNewList(m_displayListPoly, GL_COMPILE);

	// YOUR CODE GOES HERE
	// ...

    glBegin(GL_TRIANGLES);
    int triangles = m_triangles.size();
    for (int i = 0; i < triangles; ++i) {
        triangle t = m_triangles[i];

        glNormal3fv((const GLfloat *) &m_normals[t.v[0].n]);
        glTexCoord2fv((const GLfloat *) &m_uvs[t.v[0].t]);
        glVertex3fv((const GLfloat *) &m_points[t.v[0].p]);

        glNormal3fv((const GLfloat *) &m_normals[t.v[1].n]);
        glTexCoord2fv((const GLfloat *) &m_uvs[t.v[1].t]);
        glVertex3fv((const GLfloat *) &m_points[t.v[1].p]);

        glNormal3fv((const GLfloat *) &m_normals[t.v[2].n]);
        glTexCoord2fv((const GLfloat *) &m_uvs[t.v[2].t]);
        glVertex3fv((const GLfloat *) &m_points[t.v[2].p]);
    }
    glEnd();
	
	glEndList();
	cout << "Finished creating Poly Geometry" << endl;
}


//-------------------------------------------------------------
// [Assignment 1] :
// Fill the following function to create display list
// of the obj file to show it as polygon model
//-------------------------------------------------------------
void Geometry::createDisplayListWire() {
	// Delete old list if there is one
	if (m_displayListWire) glDeleteLists(m_displayListWire, 1);

	// Create a new list
	cout << "Creating Wire Geometry" << endl;
	m_displayListWire = glGenLists(1);
	glNewList(m_displayListWire, GL_COMPILE);

	// YOUR CODE GOES HERE
	// ...
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glBegin(GL_TRIANGLES);
    int triangles = m_triangles.size();
    for (int i = 0; i < triangles; ++i) {
        triangle t = m_triangles[i];

        glNormal3fv((const GLfloat *) &m_normals[t.v[0].n]);
        glVertex3fv((const GLfloat *) &m_points[t.v[0].p]);

        glNormal3fv((const GLfloat *) &m_normals[t.v[1].n]);
        glVertex3fv((const GLfloat *) &m_points[t.v[1].p]);

        glNormal3fv((const GLfloat *) &m_normals[t.v[2].n]);
        glVertex3fv((const GLfloat *) &m_points[t.v[2].p]);
    }
    glEnd();


	glEndList();
	cout << "Finished creating Wire Geometry" << endl;
}


void Geometry::renderGeometry() {
	glPushMatrix();
	{
		glTranslatef(m_position.x, m_position.y, m_position.z);

		if (m_wireFrameOn) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glShadeModel(GL_SMOOTH);
			glCallList(m_displayListWire);
		} else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glShadeModel(GL_SMOOTH);
			glCallList(m_displayListPoly);
		}
	}
	glPopMatrix();
}


void Geometry::toggleWireFrame() {
	m_wireFrameOn = !m_wireFrameOn;
}