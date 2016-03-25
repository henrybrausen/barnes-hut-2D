#ifndef __SOLVERTHREAD_H__
#define __SOLVERTHREAD_H__

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#include "Region.h"

#include <iostream>

#define NAP_NS 500

struct SolverJob {
	SolverJob()
		: root(NULL), toSolve(NULL), begin(0), end(0), epsilon(0.0)
	{}
	bhNode *root;
	std::vector<particle>* toSolve;
	int begin, end;
	double epsilon;
};

class SolverThread {
public:
	SolverThread();

	~SolverThread();

	// Solve forces for particles in range [begin, end)
	void start_job(const SolverJob& job);

	volatile void wait_for_finish();

protected:
	boost::shared_ptr<boost::thread> mThread;

	boost::mutex mWaitMutex;
	boost::condition_variable mWaitCond;

	boost::mutex mFinishedMutex;
	boost::condition_variable mFinishedCond;

	volatile bool mHasJob;
	volatile bool mRunning;

	SolverJob mJob;

	void nap();

	void process();

	void process_job();
};

#endif
