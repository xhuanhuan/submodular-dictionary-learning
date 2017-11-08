#ifndef _labelvector_h_
#define _labelvector_h_


#include <cmath>
#include <cstring>
#include <assert.h>
#include <algorithm>
#include <vector>

typedef unsigned char uchar;

#define lvRef(dm, x, y) (lv->access[x][y])
#define lvPtr(dm, x, y) &(lv->access[x][y])

class LabelVectors
{
public:
	inline LabelVectors();					// constructor
	inline LabelVectors(const int width, const int height, const bool init = true);
	inline ~LabelVectors();					// delete a LabelVector
	inline void Release();					// release the LabelVector if any
	inline void Resize(const int width, const int height, const bool init = true);
	inline void Init();
	inline int width() const { return w; } // get the width of the LabelVectors, ie the number of classes
	inline int height() const { return h; }	// get the height of the LabelVectors, ie the current number of the clusters
	inline LabelVectors* Copy() const;
	inline double& Access(int x, int y) {return access[x][y]; } // returning a reference to the parituclar location.
	
	inline double FindMergedMax(int setID1, int setID2);		
	inline double FindMergedSum(int setID1, int setID2);//add myself								
	inline double FindRowMax(int row);
	inline void Merge(int setID1, int setID2);

	/* LabelVectors pointer. */
	double *data;
	/* row pointers. */
	double **access;	

private:
	int w, h;
};


LabelVectors::LabelVectors()
{	
	w = 0;
	h = 0;
	data = NULL;
	access = NULL;
}


LabelVectors::LabelVectors(const int width, const int height, const bool init) 
{
	w = width;
	h = height;
	data = new double[w * h];  // allocate space for Image data
	access = new double*[h];   // allocate space for row pointers
	
	// initialize row pointers
	for (int i = 0; i < h; i++)
		access[i] = data + (i * w);  

	if (init)
		memset(data, 0, w * h * sizeof(int));
}

LabelVectors::~LabelVectors() 
{
	Release();
}

void LabelVectors::Release()
{
	/*if(data)
		delete [] data;
	if(access)
		delete [] access;

	h = 0;
	w = 0;*/
}

void LabelVectors::Resize(const int width, const int height, const bool init) 
{
	Release();
	w = width;
	h = height;
	data = new double[w * h];  // allocate space for DataMatrix
	access = new double*[h];   // allocate space for row pointers

	// initialize row pointers
	for (int i = 0; i < h; i++)
		access[i] = data + (i * w);  

	if (init)
		memset(data, 0, w * h * sizeof(double));
}

void LabelVectors::Init()
{
	for (int i = 0; i < h; i++)
		for (int j = 0; j < w; j++)
			access[i][j] = 0;
}

double LabelVectors::FindMergedMax(int setID1, int setID2)
{
	// according to MERCDisjointSet::Join(int x, int y),
	// newID=aID; delID=bID;
	// i.e. always put everything in s1 and delete s2
	double* ptr1= access[setID1];
	double* ptr2= access[setID2];

	double maxInRow = 0;
	// combine the two row labels together
	for (int i = 0; i < w; i++)
		maxInRow = (maxInRow < (ptr1[i]+ptr2[i]))? (ptr1[i]+ptr2[i]):maxInRow;
	return maxInRow;
}

//add myself
double LabelVectors::FindMergedSum(int setID1, int setID2)
{
	// according to MERCDisjointSet::Join(int x, int y),
	// newID=aID; delID=bID;
	// i.e. always put everything in s1 and delete s2
	double* ptr1= access[setID1];
	double* ptr2= access[setID2];

	double sumInRow = 0;
	// combine the two row labels together
	for (int i = 0; i < w; i++)
		sumInRow = sumInRow + ptr1[i]+ptr2[i];
	return sumInRow;
}


double LabelVectors::FindRowMax(int row)
{
	double* rowptr = access[row];
	double maxInRow = 0;
	for (int i = 0; i < w; i++)
	{
		maxInRow = (maxInRow < rowptr[i])? rowptr[i]:maxInRow;
		//rowptr++;
	}
	return maxInRow;
}

void LabelVectors::Merge(int setID1, int setID2)
{
	double* ptr1 = access[setID1];
	double* ptr2 = access[setID2];
	// combine the two row labels together
	for (int i = 0; i < w; i++)
	{
		ptr1[i] = ptr1[i] + ptr2[i];
		ptr2[i] = ptr1[i];
	}
}

inline LabelVectors* LabelVectors::Copy() const 
{
	LabelVectors *lv = new LabelVectors(w, h, false);
	memcpy(lv->data, data, w * h * sizeof(double));
	return lv;
}
#endif