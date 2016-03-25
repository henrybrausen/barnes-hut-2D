#include "Region.h"
#include "SolverThread.h"

#include <cmath>
#include <GL/freeglut.h>

#include <iostream>

ObjectPool<bhNode> Region::sNodePool;

// Used to render quadtree
void drawBox(double minX, double minY, double maxX, double maxY)
{
	glBegin(GL_LINES);
	glVertex2d(minX, minY);
	glVertex2d(minX, maxY);
	glVertex2d(minX, maxY);
	glVertex2d(maxX, maxY);
	glVertex2d(maxX, maxY);
	glVertex2d(maxX, minY);
	glVertex2d(maxX, minY);
	glVertex2d(minX, minY);
	glEnd();
}


void Region::addParticle(particle p)
{
	p.id = mNextId++;
	mParticles.push_back(p);
}

//void Region::update(double epsilon)
//{
//	bhNode *root = getBarnesHutTree();
//
//	SolverThread *threads[NUM_THREADS];
//
//	SolverJob thread_job;
//
//	thread_job.epsilon = epsilon;
//	thread_job.toSolve = &mParticles;
//	thread_job.root = root;
//
//	int step = mParticles.size()/NUM_THREADS;
//
//	for (int i = 0; i < NUM_THREADS; ++i) {
//		thread_job.begin = i*step;
//		thread_job.end = std::min((i+1)*step, (int)mParticles.size());
//
//		threads[i] = &sThreadPool.acquireObject();
//		threads[i]->start_job(thread_job);
//	}
//	for (int i = 0; i < NUM_THREADS; ++i) {
//		threads[i]->wait_for_finish();
//		sThreadPool.releaseObject(*threads[i]);
//	}
//	freeBarnesHutTree(root);
//}

void Region::update(double epsilon)
{
	bhNode *root = getBarnesHutTree();

	int step = mParticles.size()/NUM_THREADS;
	
	int i;
	
	for (i = 0; i < NUM_THREADS - 1; ++i) {
		mThreadPool.schedule(boost::bind(&Region::solverFunction, root, &mParticles, i*step,
			(i+1)*step, epsilon));
	}
	mThreadPool.schedule(boost::bind(&Region::solverFunction, root, &mParticles, i*step,
		(int)mParticles.size(), epsilon));
	mThreadPool.wait();
	freeBarnesHutTree(root);
}

RegionStatistics Region::getStatistics() const
{
	RegionStatistics ret;
	ret.cmX = ret.cmY = ret.mass = 0.0;
	ret.minX = ret.maxX = mParticles[0].x;
	ret.minY = ret.maxY = mParticles[0].y;
	for (std::vector<particle>::const_iterator it = mParticles.begin(); it != mParticles.end(); ++it) {
		ret.minX = std::min(it->x, ret.minX);
		ret.minY = std::min(it->x, ret.minY);
		ret.maxX = std::max(it->x, ret.maxX);
		ret.maxY = std::max(it->x, ret.maxY);
		ret.cmX += (it->x)*(it->mass);
		ret.cmY += (it->y)*(it->mass);
		ret.mass += it->mass;
	}
	ret.cmX /= ret.mass;
	ret.cmY /= ret.mass;
	return ret;
}

bhQuadrant Region::assignQuadrant(bhNode *node, const particle& p)
{
	if ((p.x <= (node->minX+node->maxX)/2.0) && (p.y <= (node->minY+node->maxY)/2.0)) {
		return NW;
	}
	if ((p.x > (node->minX+node->maxX)/2.0) && (p.y <= (node->minY+node->maxY)/2.0)) {
		return NE;
	}
	if ((p.x <= (node->minX+node->maxX)/2.0) && (p.y > (node->minY+node->maxY)/2.0)) {
		return SW;
	}
	if ((p.x > (node->minX+node->maxX)/2.0) && (p.y > (node->minY+node->maxY)/2.0)) {
		return SE;
	}
}

void Region::renderBhTree(int maxDepth) const
{
	bhNode *root = getBarnesHutTree();
	glPushMatrix();
		glColor4f(0.0f, 1.0f, 0.2f, 1.0f);
		drawBox(root->minX, root->minY, root->maxX, root->maxY);
		for (int i = 0; i < 4; ++i)
			Region::renderSubTree(root->quads[i], maxDepth - 1, maxDepth);
	glPopMatrix();
	Region::freeBarnesHutTree(root);
}

void Region::renderSubTree(bhNode *node, int depth, int maxDepth)
{
	if (depth <= 0) return;
	glColor4f(0.0f, 1.0f, 0.2f, 0.1*(float)depth/(float)maxDepth);
	drawBox(node->minX, node->minY, node->maxX, node->maxY);
	if (node->type == INTERNAL)
		for (int i = 0; i < 4; ++i)
			Region::renderSubTree(node->quads[i], depth - 1, maxDepth);
}

void Region::subdivide(bhNode *parent)
{
	for (int i = 0; i < 4; ++i) {
		parent->quads[i] = &(sNodePool.acquireObject());
		parent->quads[i]->type = EMPTY;
		parent->quads[i]->mass = 0.0;
	}
	parent->quads[NW]->minX = parent->quads[SW]->minX = parent->minX;
	parent->quads[NW]->minY = parent->quads[NE]->minY = parent->minY;
	parent->quads[NE]->maxX = parent->quads[SE]->maxX = parent->maxX;
	parent->quads[SW]->maxY = parent->quads[SE]->maxY = parent->maxY;
	parent->quads[NW]->maxX = parent->quads[SW]->maxX = parent->quads[NE]->minX = parent->quads[SE]->minX = (parent->maxX+parent->minX)/2.0;
	parent->quads[NW]->maxY = parent->quads[NE]->maxY = parent->quads[SW]->minY = parent->quads[SE]->minY = (parent->maxY+parent->minY)/2.0;
	parent->quads[NE]->minX = parent->quads[SE]->minX = (parent->maxX+parent->minX)/2.0;
	parent->quads[SW]->minY = parent->quads[SE]->minY = (parent->maxY+parent->minY)/2.0;
}

void Region::updateCM(bhNode *node)
{
	node->cmX = node->cmY = node->mass = 0.0;
	for (int i = 0; i < 4; ++i) {
		node->mass += node->quads[i]->mass;
		node->cmX += (node->quads[i]->cmX)*(node->quads[i]->mass);
		node->cmY += (node->quads[i]->cmY)*(node->quads[i]->mass);
	}
	node->cmX /= node->mass;
	node->cmY /= node->mass;
}

void Region::buildSubtree(bhNode *parent, const particle& p, bhQuadrant quad)
{
	bhNode *child = parent->quads[quad];
	switch (child->type) {
	case EXTERNAL:
		// Prevent (possibly) infinite recursion!
		if ((child->containedParticle->x == p.x) && (child->containedParticle->y == p.y)) {
			child->cmX = (p.x+child->containedParticle->x)/2.0;
			child->cmY = (p.y+child->containedParticle->y)/2.0;
			child->mass += p.mass;
			return;
		}
		child->type = INTERNAL;
		subdivide(child);
		Region::buildSubtree(child, *(child->containedParticle), Region::assignQuadrant(child, *(child->containedParticle)));
		Region::buildSubtree(child, p, Region::assignQuadrant(child, p));
		Region::updateCM(child);
		break;
	case INTERNAL:
		Region::buildSubtree(child, p, Region::assignQuadrant(child, p));
		Region::updateCM(child);
		break;
	case EMPTY:
		child->cmX = p.x;
		child->cmY = p.y;
		child->mass = p.mass;
		child->containedParticle = &p;
		child->type = EXTERNAL;
		break;
	}
}

bhNode *Region::getBarnesHutTree() const
{
	bhNode *root = &(sNodePool.acquireObject());
	root->minX = root->maxX = mParticles[0].x;
	root->minY = root->maxY = mParticles[0].y;
	for (std::vector<particle>::const_iterator it = mParticles.begin(); it != mParticles.end(); ++it) {
		root->minX = std::min(it->x, root->minX);
		root->minY = std::min(it->y, root->minY);
		root->maxX = std::max(it->x, root->maxX);
		root->maxY = std::max(it->y, root->maxY);
	}
	root->type = EMPTY;
	subdivide(root);

	for (std::vector<particle>::const_iterator it = mParticles.begin(); it != mParticles.end(); ++it) {
		Region::buildSubtree(root, *it, Region::assignQuadrant(root, *it));
	}
	Region::updateCM(root);

	root->type = INTERNAL;

	return root;
}

void Region::freeBarnesHutTree(bhNode *root)
{
	if (root->type == INTERNAL)
		for (int i = 0; i < 4; ++i) Region::freeBarnesHutTree(root->quads[i]);
	sNodePool.releaseObject((*root));
}

void Region::solveAcceleration(particle& p, bhNode *root, double epsilon)
{
	if (root->type == EMPTY) return;
	double dx, dy, dsq, distance, acc;

	dx = root->cmX-p.x;
	dy = root->cmY-p.y;
	dsq = dx*dx+dy*dy+SOFTENING_FACTOR;
	distance = sqrt(dsq);
	if (root->type == EXTERNAL) {
		if (root->containedParticle->id == p.id) return;
		
		acc = (CONST_G*root->mass) / dsq;
		p.ax += dx/distance * acc;
		p.ay += dy/distance * acc;
		return;
	}

	double s = abs(root->maxX - root->minX)*abs(root->maxY - root->minY);

	if (s/dsq < THETA_SQUARED) {	// 0.5**2
		acc = (CONST_G*root->mass) / dsq;
		p.ax += dx/distance * acc;
		p.ay += dy/distance * acc;
		return;
	}

	for (int i = 0; i < 4; ++i)
		Region::solveAcceleration(p, root->quads[i], epsilon);
}

void Region::solverFunction(bhNode *root, std::vector<particle> *toSolve, int begin, int end, double epsilon)
{
	for (std::vector<particle>::iterator it = toSolve->begin()+begin; it < toSolve->begin()+end; ++it) {
		double last_ax = it->ax;
		double last_ay = it->ay;
		double last_vx = it->vx;
		double last_vy = it->vy;
		it->ax = it->ay = 0.0;
		Region::solveAcceleration(*it, root, epsilon);
		it->vx += epsilon*((it->ax)+last_ax)/2.0;
		it->vy += epsilon*((it->ay)+last_ay)/2.0;
		it->x += epsilon*((it->vx)+last_vx)/2.0;
		it->y += epsilon*((it->vy)+last_vy)/2.0;
	}
}
