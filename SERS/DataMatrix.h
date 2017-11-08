#ifndef _datamatrix_h_
#define _datamatrix_h_

#include <cmath>
#include <cstring>
#include <assert.h>
typedef unsigned char uchar;

#define dmRef(dm, x) (dm->access[x])
#define dmPtr(dm, x) &(dm->access[x])


template <class T>
class DataMatrix 
{
public:
	inline DataMatrix();					// constructor
	inline DataMatrix(const int width, const int height, const bool init = true);
	inline ~DataMatrix();					// delete a DataMatrix
	inline void Release();					// release the DataMtrix if any
	inline void Resize(const int width,const int height, const bool init = true);
	inline void Init(const T &val);			// initialize a DataMatrix
	inline DataMatrix<T> *Copy() const;

	inline int width() const { return w; }	// get the width of the DataMatrix
	inline int height() const { return h; }	// get the height of the DataMatrix
	inline T& Access(int x,int y) {return access[x][y];};
											// returning a reference to the parituclar location.
	T *data;						/* DataMatrix. */
	T **access;						/* row pointers. */

private:
	int w, h;
};

template <class T>
DataMatrix<T>::DataMatrix()
{
	w = 0;
	h = 0;
	data = NULL;
	access = NULL;
}


template <class T>
DataMatrix<T>::DataMatrix(const int width, const int height, const bool init) 
{
	w = width;
	h = height;
	data = new T[w * h];  // allocate space for Image data
	access = new T*[h];   // allocate space for row pointers
	// initialize row pointers
	for (int i = 0; i < h; i++)
		access[i] = data + (i * w);  

	if (init)
		memset(data, 0, w * h * sizeof(T));
}

template <class T>
DataMatrix<T>::~DataMatrix() 
{
	Release();
}

template <class T>
void DataMatrix<T>::Release()
{
	if(data)
		delete [] data;
	if(access)
		delete [] access;

	h = 0;
	w = 0;
}


template <class T>
void DataMatrix<T>::Resize(const int width, const int height, const bool init) 
{
	Release();
	w = width;
	h = height;
	data = new T[w * h];  // allocate space for DataMatrix
	access = new T*[h];   // allocate space for row pointers

	// initialize row pointers
	for (int i = 0; i < h; i++)
		access[i] = data + (i * w);  

	if (init)
		memset(data, 0, w * h * sizeof(T));
}

template <class T>
void DataMatrix<T>::Init(const T &val) 
{
	T *ptr = dmPtr(this, 0, 0);
	T *end = dmPtr(this, w-1, h-1);
	while (ptr <= end)
		*ptr++ = val;
}

template <class T>
DataMatrix<T> *DataMatrix<T>::Copy() const 
{
	DataMatrix<T> *dm = new DataMatrix<T>(w, h, false);
	memcpy(dm->data, data, w * h * sizeof(T));
	return dm;
}

#endif
  