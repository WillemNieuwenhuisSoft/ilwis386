/*
* The author of this software is Steven Fortune.  Copyright (c) 1994 by AT&T
* Bell Laboratories.
* Permission to use, copy, modify, and distribute this software for any
* purpose without fee is hereby granted, provided that this entire notice
* is included in all copies of any software which is or includes a copy
* or modification of this software and in all copies of the supporting
* documentation for such software.
* THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
* WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR AT&T MAKE ANY
* REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
* OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
*/

/* 
* This code was originally written by Stephan Fortune in C code.  I, Shane O'Sullivan, 
* have since modified it, encapsulating it in a C++ class and, fixing memory leaks and 
* adding accessors to the Voronoi Edges.
* Permission to use, copy, modify, and distribute this software for any
* purpose without fee is hereby granted, provided that this entire notice
* is included in all copies of any software which is or includes a copy
* or modification of this software and in all copies of the supporting
* documentation for such software.
* THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
* WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR AT&T MAKE ANY
* REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
* OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
*/

#ifndef VORONOI_DIAGRAM_GENERATOR
#define VORONOI_DIAGRAM_GENERATOR

#include <math.h>
#include <stdlib.h>
#include <string.h>


#ifndef NULL
#define NULL 0
#endif
#define DELETED -2

#define le 0
#define re 1



struct	Freenode	
{
	struct	Freenode *nextfree;
};

struct FreeNodeArrayList
{
	struct	Freenode* memory;
	struct	FreeNodeArrayList* next;

};

struct	Freelist	
{
	struct	Freenode	*head;
	int		nodesize;
};

struct VoronoiPoint	
{
	float x,y;
};

// structure used both for sites and for vertices 
struct Site	
{
	struct	VoronoiPoint	coord;
	int		sitenbr;
	int		refcnt;
};



struct Edge	
{
	float   a,b,c;
	struct	Site 	*ep[2];
	struct	Site	*reg[2];
	int		edgenbr;

};

struct GraphEdge
{
	float x1,y1,x2,y2;
	struct GraphEdge* next;
};




struct Halfedge 
{
	struct	Halfedge	*ELleft, *ELright;
	struct	Edge	*ELedge;
	int		ELrefcnt;
	char	ELpm;
	struct	Site	*vertex;
	float	ystar;
	struct	Halfedge *PQnext;
};




class VoronoiDiagramGenerator
{
public:
	VoronoiDiagramGenerator();
	~VoronoiDiagramGenerator();

	bool generateVoronoi(vector<Coord> coords, const CoordBounds& cbIn, double minDist = 0);

	void resetIterator()
	{
		iteratorEdges = allEdges;
	}

	bool getNext(Coord& c1, Coord& c2)
	{
		if(iteratorEdges == 0)
			return false;
		
		c1.x = iteratorEdges->x1;
		c2.x = iteratorEdges->x2;
		c1.y = iteratorEdges->y1;
		c2.y = iteratorEdges->y2;

		iteratorEdges = iteratorEdges->next;

		return true;
	}


private:
	void cleanup();
	void cleanupEdges();
	char *getfree(struct Freelist *fl);	
	struct	Halfedge *PQfind();
	int PQempty();


	
	struct	Halfedge **ELhash;
	struct	Halfedge *HEcreate(), *ELleft(), *ELright(), *ELleftbnd();
	struct	Halfedge *HEcreate(struct Edge *e,int pm);


	struct VoronoiPoint PQ_min();
	struct Halfedge *PQextractmin();	
	void freeinit(struct Freelist *fl,int size);
	void makefree(struct Freenode *curr,struct Freelist *fl);
	void geominit();
	void plotinit();
	bool voronoi(int triangulate);
	void ref(struct Site *v);
	void deref(struct Site *v);
	void endVoronoiPoint(struct Edge *e,int lr,struct Site * s);

	void ELdelete(struct Halfedge *he);
	struct Halfedge *ELleftbnd(struct VoronoiPoint *p);
	struct Halfedge *ELright(struct Halfedge *he);
	void makevertex(struct Site *v);
	void out_triple(struct Site *s1, struct Site *s2,struct Site * s3);

	void PQinsert(struct Halfedge *he,struct Site * v, float offset);
	void PQdelete(struct Halfedge *he);
	bool ELinitialize();
	void ELinsert(struct	Halfedge *lb, struct Halfedge *newHe);
	struct Halfedge * VoronoiDiagramGenerator::ELgethash(int b);
	struct Halfedge *ELleft(struct Halfedge *he);
	struct Site *leftreg(struct Halfedge *he);
	void out_site(struct Site *s);
	bool PQinitialize();
	int PQbucket(struct Halfedge *he);
	void clip_line(struct Edge *e);
	char *myalloc(unsigned n);
	int right_of(struct Halfedge *el,struct VoronoiPoint *p);

	struct Site *rightreg(struct Halfedge *he);
	struct Edge *bisect(struct	Site *s1,struct	Site *s2);
	float dist(struct Site *s,struct Site *t);
	struct Site *intersect(struct Halfedge *el1, struct Halfedge *el2, struct VoronoiPoint *p=0);

	void out_bisector(struct Edge *e);
	void out_ep(struct Edge *e);
	void out_vertex(struct Site *v);
	struct Site *nextone();

	void pushGraphEdge(float x1, float y1, float x2, float y2);

	void VoronoiDiagramGenerator::openpl();
	void VoronoiDiagramGenerator::line(float x1, float y1, float x2, float y2);
	void VoronoiDiagramGenerator::circle(float x, float y, float radius);
	void VoronoiDiagramGenerator::range(float minX, float minY, float maxX, float maxY);


	struct  Freelist	hfl;
	struct	Halfedge *ELleftend, *ELrightend;
	int 	ELhashsize;

	int		triangulate, sorted, plot, debug;
	float	xmin, xmax, ymin, ymax, deltax, deltay;

	struct	Site	*sites;
	int		nsites;
	int		siteidx;
	int		sqrt_nsites;
	int		nvertices;
	struct 	Freelist sfl;
	struct	Site	*bottomsite;

	int		nedges;
	struct	Freelist efl;
	int		PQhashsize;
	struct	Halfedge *PQhash;
	int		PQcount;
	int		PQmin;

	int		ntry, totalsearch;
	float	pxmin, pxmax, pymin, pymax, cradius;
	int		total_alloc;

	float borderMinX, borderMaxX, borderMinY, borderMaxY;

	FreeNodeArrayList* allMemoryList;
	FreeNodeArrayList* currentMemoryBlock;

	GraphEdge* allEdges;
	GraphEdge* iteratorEdges;

	float minDistanceBetweenSites;
	
};

int scomp(const void *p1,const void *p2);


#endif


