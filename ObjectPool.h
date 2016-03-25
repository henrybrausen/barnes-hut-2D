#ifndef __OBJECTPOOL_H__
#define __OBJECTPOOL_H__

#include <vector>
#include <queue>
#include <algorithm>

template <typename T>
class ObjectPool {
public:
	ObjectPool(const int chunkSize = kDefaultChunkSize);
	~ObjectPool();

	T& acquireObject();
	void releaseObject(T& obj);

protected:
	std::queue<T*> mFreeObjects;
	std::vector<T*> mAllObjects;

	int mChunkSize;
	static const int kDefaultChunkSize = 1000;

	void allocateChunk();
	static void arrayDeleteObject(T* obj);

	ObjectPool(const ObjectPool<T>& src);
	ObjectPool<T>& operator=(const ObjectPool<T>& rhs);
};

template <typename T>
ObjectPool<T>::ObjectPool(int chunkSize)
	: mChunkSize(chunkSize)
{
	allocateChunk();
}

template <typename T>
ObjectPool<T>::~ObjectPool()
{
	std::for_each(mAllObjects.begin(), mAllObjects.end(), arrayDeleteObject);
}

template <typename T>
T& ObjectPool<T>::acquireObject()
{
	if (mFreeObjects.empty())
		allocateChunk();
	T* obj = mFreeObjects.front();
	mFreeObjects.pop();
	return (*obj);
}

template <typename T>
void ObjectPool<T>::releaseObject(T& obj)
{
	mFreeObjects.push(&obj);
}

template <typename T>
void ObjectPool<T>::allocateChunk()
{
	T* newChunk = new T[mChunkSize];
	mAllObjects.push_back(newChunk);
	for (int i = 0; i < mChunkSize; ++i)
		mFreeObjects.push(&newChunk[i]);
}

template <typename T>
void ObjectPool<T>::arrayDeleteObject(T* obj)
{
	delete [] obj;
}

#endif
