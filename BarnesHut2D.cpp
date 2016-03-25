// barneshut2d.cpp : Defines the entry point for the console application.
//
#include <iostream>

#include <vector>
#include <algorithm>
#include <limits>
#include <cstdlib>

#include <GL/freeglut.h>

#include "rc4.h"
#include "Region.h"

#include <ctime>

#define NUM_PARTICLES 1000

Region theRegion;

int numIterations;

int startTime;

void initialize()
{
	startTime = time(NULL);
	numIterations = 0;
	particle p;
	p.vx = p.vy = p.ax = p.ay = 0.0;
	for (int i = 0; i < NUM_PARTICLES; ++i) {
		p.x = 200+200.0*((double)rand32()/std::numeric_limits<unsigned int>::max());
		p.y = 100+200.0*((double)rand32()/std::numeric_limits<unsigned int>::max());
		p.mass = 1.0+((double)rand32()/std::numeric_limits<unsigned int>::max());
		theRegion.addParticle(p);
	}
	/*for (double r = 50.0f; r < 70.0; r += 2.0) {
		for (double theta = 0; theta < 3.141592*2.0; theta += 3.1415926/r) {
			p.x = 640.0+r*cos(theta);
			p.y = 400.0+r*sin(theta);
			p.vx = 300.0*exp(-r/50.0)*sin(theta);
			p.vy = -200.0*exp(-r/50.0)*cos(theta);
			p.mass = 10.0;
			theRegion.addParticle(p);
		}
	}*/
	//p.x = 640.0;
	//p.y = 400.0;
	//p.vx = p.vy = p.ax = p.ay = 0.0;
	//p.mass = 50000.0;
	//theRegion.addParticle(p);
	//for (double r = 20.0f; r < 100.0; r += 1.0) {
	//	for (double theta = 0; theta < 3.141592*2.0; theta += 3.1415926/(r/2.0)) {
	//		p.x = 640.0+r*cos(theta);
	//		p.y = 400.0+r*sin(theta);
	//		p.mass = 1.0;
	//		theRegion.addParticle(p);
	//	}
	//}

	std::cout << "Number of particles: " << theRegion.getParticles().size() << std::endl;
}

void process()
{
	theRegion.update(0.00128);
	++numIterations;
	std::cout << numIterations << std::endl;
	//std::cout << "Done!" << std::endl;
}

void onRender()
{
	glClear(GL_COLOR_BUFFER_BIT);
	const std::vector<particle>& particles = theRegion.getParticles();
	glBegin(GL_POINTS);
	for (std::vector<particle>::const_iterator it = particles.begin();
		it != particles.end(); ++it) {
			glColor4f(1.0f, 1.0f, 1.0f, 0.2f);
			glVertex2f(it->x, it->y);
	}
	glEnd();
	/*glBegin(GL_LINES);
	for (std::vector<particle>::const_iterator it = particles.begin();
		it != particles.end(); ++it) {
			glColor4f(0.0f, 0.0f, 1.0f, 0.1f);
			glVertex2f(it->x, it->y);
			glVertex2f(it->x+it->vx*0.1, it->y+it->vy*0.1);
	}
	glEnd();
	glBegin(GL_LINES);
	for (std::vector<particle>::const_iterator it = particles.begin();
		it != particles.end(); ++it) {
			glColor4f(1.0f, 0.0f, 0.0f, 0.1f);
			glVertex2f(it->x, it->y);
			glVertex2f(it->x+it->ax*0.05, it->y+it->ay*0.05);
	}
	glEnd();*/
	theRegion.renderBhTree(20);
	glutSwapBuffers();
}

void onReshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, (GLdouble)w, 0.0, (GLdouble)h);
}

void onIdle()
{
	process();
	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	rc4::apply_key("TESTING_KEY2");

	initialize();

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(1280, 800);
	glutCreateWindow("Barnes-Hut Simulation");

	glutIdleFunc(onIdle);
	glutDisplayFunc(onRender);
	glutReshapeFunc(onReshape);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glShadeModel(GL_FLAT);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPointSize(2);
	glutMainLoop();

	return 0;
}

