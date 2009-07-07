#include <tinyxml/tinyxml.h>
#include <cstring>
#include <sstream>
#include <vector>
#include <list>

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

class TiXmlDocument;

namespace Utilities
{
  namespace Gmsh
  {
    
    struct Vertex
    {
      Vertex(int id, double x, double y, double z):
	id(id),
	x(x), 
	y(y), 
	z(z)
      {};
      int id;
      double x;
      double y;
      double z;
    };
    
    struct Edge
    {
      Edge(int id, vector<int> vert):
	id(id),
	vert(vert) 
      {};
      int id;
      vector<int> vert;
    };

    struct Face
    {
      Face(int id, vector<int> vert):
	id(id),
	vert(vert)
      {};
      int id;
      vector<int> vert;
    };

    struct ZeroDElement
    {
      ZeroDElement(int id, int type,  vector<int> tags, vector<int> vert):
	id(id),
	type(type),
	tags(tags),
	vert(vert)				
      {};
      int id;
      int type;
      vector<int> tags;
      vector<int> vert;
    };
    
    struct OneDElement
    {
      OneDElement(int id, int type,  vector<int> tags, vector<int> vert):
	id(id),               
	type(type),
	tags(tags),
	vert(vert)				
      {};
      int id;
      int type;
      vector<int> tags;
      vector<int> vert;
    };

    struct TwoDElement
    {
      TwoDElement(int id, int type, vector<int> tags, vector<int> vert, vector<int> edge):
	id(id),
	type(type),
	tags(tags),
	vert(vert),
	edge(edge)
      {};
      int id;
      int type;	
      vector<int> tags;
      vector<int> vert;
      vector<int> edge;
    };

    struct ThreeDElement
    {
      ThreeDElement(int id, int type, vector<int> tags, vector<int> vert, vector<int> face):
	id(id),
	type(type),
	vert(vert),
	face(face)				
      {};
      int id;
      int type;
      vector<int> vert;
      vector<int> face;
    };
    
    struct Composite
    {
      Composite(int id, int type, list<int>eid ):
	id(id),
	type(type),
	eid(eid)
      {};
      int id;
      int type;
      list<int> eid;
    };
    

    void ParseGmshFile(const char* inFile, const char* outfile);
    void WriteToXMLFile(const char* outfile, int expDim, const vector<Vertex> & vertices, const vector<Edge> & edges, 
			const vector<Face> & faces, const vector<OneDElement> & oneDElements,
			const vector<TwoDElement> & twoDElements, const vector<ThreeDElement> & threeDElements,
			const vector<Composite> & composites);
    void SortEdgeToVertex(vector<TwoDElement> & elements, vector<Edge> & edges);
    void SortZeroDElements(vector<ZeroDElement> & points,const vector<Vertex> & vertices);
    void SortOneDElements(vector<OneDElement> &segments, const vector<Edge>& edges);
    void SortTwoDComposites(const vector<TwoDElement> & elements, const vector<OneDElement> & edges, 
			    vector<Composite> & composites, int ncomposites);
    void SortOneDComposites(const vector<OneDElement> & elements, const vector<ZeroDElement> & points, 
			    vector<Composite> & composites, int ncomposites);
    int  GetNnodes(int GmshEntity);
    int GetEdge(vector<int> &vert, vector<Edge>& edges);  
     
   
  } //end of namespace Gmsh
     
} //end of namespace Utilities
     