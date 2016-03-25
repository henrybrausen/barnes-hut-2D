#include "SolverThread.h"	

SolverThread::SolverThread()
	: mHasJob(false), mRunning(true)
{
	std::cout << "Thread started." << std::endl;
	mThread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&SolverThread::process, this)));
}

SolverThread::~SolverThread()
{
	std::cout << "Thread terminated." << std::endl;
	mRunning = false;
	mWaitCond.notify_all();
	if (mThread != NULL) mThread->join();
}

// Solve forces for particles in range [begin, end)
void SolverThread::start_job(const SolverJob& job) {
	mJob = job;
	mHasJob = true;
	std::cout << "Sending job start notification . . ." << std::endl;
	mWaitCond.notify_all();
}

volatile void SolverThread::wait_for_finish() {
	std::cout << "My condition variable is " << &mWaitCond << std::endl;
	std::cout << "Getting lock . . ." << std::endl;
	boost::unique_lock<boost::mutex> lock(mFinishedMutex);
	std::cout << "Lock acquired." << std::endl;
	std::cout << "Current state: " << mHasJob << std::endl;
	while (mHasJob) {
		std::cout << "Waiting . . ." << std::endl;
		mFinishedCond.wait(lock);
		std::cout << "Notified: " << mHasJob << std::endl;
	}
	std::cout << "Returning . . ." << std::endl;
}
void SolverThread::nap() {
	static boost::xtime xt;
	boost::xtime_get(&xt, boost::TIME_UTC_);
	xt.nsec += NAP_NS;
	boost::thread::sleep(xt);
}

void SolverThread::process() {
	boost::unique_lock<boost::mutex> lock(mWaitMutex);
	while (mRunning) {
		mWaitCond.wait(lock);
		std::cout << "Woke up" << std::endl;
		if (mHasJob) process_job();
	}
}

void SolverThread::process_job() {
	std::cout << "Starting job . . ." << std::endl;
	for (std::vector<particle>::iterator it = mJob.toSolve->begin()+mJob.begin; it < mJob.toSolve->begin()+mJob.end; ++it) {
		double last_ax = it->ax;
		double last_ay = it->ay;
		double last_vx = it->vx;
		double last_vy = it->vy;
		it->ax = it->ay = 0.0;
		Region::solveAcceleration(*it, mJob.root, mJob.epsilon);
		it->vx += mJob.epsilon*((it->ax)+last_ax)/2.0;
		it->vy += mJob.epsilon*((it->ay)+last_ay)/2.0;
		it->x += mJob.epsilon*((it->vx)+last_vx)/2.0;
		it->y += mJob.epsilon*((it->vy)+last_vy)/2.0;
	}
	mHasJob = false;
	std::cout << "Notifying . . ." << std::endl;
	mFinishedCond.notify_all();
	std::cout << "Finished." << std::endl;
}
