#ifndef __REGION_H__
#define __REGION_H__

#include <vector>
#include <algorithm>
#include <limits>
#include <cstdlib>

#include <boost/threadpool.hpp>

#include "ObjectPool.h"

#define CONST_G 10.0
#define SOFTENING_FACTOR 0.5
#define THETA_SQUARED 0.25

#define NUM_THREADS 8

enum bhNodeType { EMPTY, INTERNAL, EXTERNAL };
enum bhQuadrant { NW=0, NE, SW, SE };

struct particle {
	double x, y, vx, vy, ax, ay;
	double mass;
	int id;
};

struct bhNode {
	bhNode *quads[4];	// Quadrants
	double cmX, cmY, mass;	// Center of mass
	double minX, minY, maxX, maxY;
	const particle *containedParticle;
	bhNodeType type;
};

struct RegionStatistics {
	double cmX, cmY, mass;
	double minX, minY, maxX, maxY;
};

class SolverThread;

class Region {
public:
	Region()
		: mNextId(0), mThreadPool(NUM_THREADS)
	{}

	void addParticle(particle p);
	void update(double epsilon);
	
	RegionStatistics getStatistics() const;

	const std::vector<particle>& getParticles() const { return mParticles; }

	void renderBhTree(int maxDepth) const;

	friend class SolverThread;

private:
	bhNode *getBarnesHutTree() const;
	static bhQuadrant assignQuadrant(bhNode *node, const particle& p);
	static void subdivide(bhNode *parent);
	static void updateCM(bhNode *node);
	static void buildSubtree(bhNode *parent, const particle& p, bhQuadrant quad);
	static void freeBarnesHutTree(bhNode *root);

	static void renderSubTree(bhNode *node, int depth, int maxDepth);

	static void solveAcceleration(particle& p, bhNode *root, double epsilon);

	static void solverFunction(bhNode *root, std::vector<particle> *toSolve, int begin, int end, double epsilon);

	std::vector<particle> mParticles;
	int mCurrentVector;
	int mNextId;

	static ObjectPool<bhNode> sNodePool;
	//static ObjectPool<SolverThread> sThreadPool;
	boost::threadpool::pool mThreadPool;
};

#endif
