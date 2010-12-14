/// Custom vector class. Implemented after STL Vector.

/// Copyright 2010 Ninja Unicorn. All rights reserved.
/// \author Tommy Nguyen

#ifndef VECTOR_H_
#define VECTOR_H_

#include <cassert>
#include <cstring>

template<class T>
class Vector
{
public:
	Vector(const unsigned int reserve = 8) : count(0), reserved(reserve)
	{
		assert(reserve > 0);
		this->array = new T[this->reserved];
	}

	~Vector()
	{
		delete[] this->array;
	}

	void add(const T &element)
	{
		// Check that there is enough space
		if (this->count == this->reserved)
		{
			this->reserved <<= 1;
			T *a = new T[this->reserved];
			memcpy(a, this->array, (this->count << 2) * sizeof(T));
			delete[] this->array;
			this->array = a;
		}
		this->array[this->count++] = element;
	}

	inline T* c_arr()
	{
		return this->array;
	}

	/// Returns the number of elements in this vector.
	inline unsigned int size()
	{
		return this->count;
	}

	/// Returns the element stored at index.
	inline T& operator[](unsigned int i)
	{
		return this->array[i];
	}

private:
	unsigned int count;
	unsigned int reserved;
	T *array;
};

#endif
