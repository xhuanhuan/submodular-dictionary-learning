#ifndef _m_erc_input_matrix_h_
#define _m_erc_input_matrix_h_

#include "MERCInput.h"
#include "DataMatrix.h"
#include "LabelVectors.h"
#include "mex.h"
#include <cmath>

using namespace std;

template <class T>
class MERCInputMatrix: public MERCInput
{
public:
	void ReadMatrix(DataMatrix<T> *inputMatrix, int K);
	void ReadLabels(LabelVectors *lv, int nPoints);
	void ComputeDist(DataMatrix<T> *inputMatrix);
	
	int* kNN_id (T *dist, int nNodes, int K);	// find the ids for the nearest k neighbors.
	int maxD_KNN_id(T *arr, int K);	// returns the id of the maximum.
	T maxD_KNN(T *arr, int K);      // returns the maximum value of the k nearest distances.

	T** dist_;		// distance matrix
	int width_;		// width of the DataMatrix
	int height_;
};


template <class T>
void MERCInputMatrix<T>::ReadMatrix(DataMatrix<T> *inputMatrix, int K)
{
	width_ = inputMatrix->width();
	height_ = inputMatrix->height();
	nNodes_ = height_;	
	//cout<<width_<<", "<<height_<<endl;
	edges_ = new Edge[K*nNodes_];
	
	/* Compute the distances between every two points. */
	ComputeDist(inputMatrix);
	/* Construct the graph: find the K nearest neighbors */
	int num = 0;
	for (int x = 0; x < height_; x++)
	{
		int* kIDs = kNN_id(dist_[x], nNodes_, K);
		for (int k = 0; k < K; k++)
		{
			edges_[num].a_ = x;
			edges_[num].b_ = kIDs[k];
			edges_[num].w_ = dist_[x][kIDs[k]];
			num++;
		}
	}


	/*int* kIDs = kNN_id(dist_[0], nNodes_, K);
	for(int ii=0; ii<K; ii++)
	mexPrintf("%d ",kIDs[ii]);
    mexEvalString("pause(1);");*/

	/* Delete the duplicated edges. */
	for (int n = 0; n < num; n++)
	{
		int nextone = n+1;
		while (nextone < num)
		{
			if (edges_[n] == edges_[nextone])
			{
				// move the rest of edges forwards by one
				for (int bh = nextone; bh < num-1; bh++)
					edges_[bh] = edges_[bh+1];
				num--;
				nextone--;
			}
			nextone++;
		}
	}
	nEdges_ = num;
/*	for (int l = 0; l < num; l++)
		mexPrintf("%d,%d\n",edges_[l].a_,edges_[l].b_);*/

}

template <class T>
void MERCInputMatrix<T>::ReadLabels(LabelVectors *lv, int nPoints)
{
	labelv_ = lv->Copy();
	labelMerged_ = lv->Copy();

}

template <class T>
void MERCInputMatrix<T>::ComputeDist(DataMatrix<T> *inputMatrix)
{
	width_ = inputMatrix->width();
	height_ = inputMatrix->height();
	nNodes_ = height_;	

	T sqrdist,d;
	T *Dist = new T[height_*height_];	// allocate space for the distance matrix	
	T **distance = new T*[height_];
	

	for (int x = 0; x < height_; x++)
	{
		distance[x] = Dist + x * height_;
		for (int y = 0; y < height_; y++)
		{
			sqrdist = 0;
			for (int f = 0; f < width_; f++)
			{
				//d = inputMatrix->Access(x,f) - inputMatrix->Access(y,f);
				d = inputMatrix->access[x][f] - inputMatrix->access[y][f];
				sqrdist = sqrdist + d*d;
			}
	//		distance[x][y] = sqrt(sqrdist);
			distance[x][y] = sqrdist;
		}

	 /*  for(int ii=0; ii<20; ii++)
	   {
	     mexPrintf("%.2f ",distance[0][ii]);
         mexEvalString("pause(0.01);");
	   }*/
	}
	dist_ = distance;
}

template <class T>
int* MERCInputMatrix<T>::kNN_id (T *dist, int nNodes, int K)
{
	int *nearestID = new int[K];	
	// an array which records the smallest K distances so far.
	T *nearestDist = new T[K];
	int ptrk = 0;
	// initialize the array with the first K distances
	for (int k = 0;k < K; k++)
	{
		if (dist[ptrk] == 0)
			ptrk++;
		nearestID[k] = ptrk;
		nearestDist[k] = dist[ptrk];
		ptrk++;
	}
	// now search through the rest of the array;
	// replace the largest with the current one if it's smaller
	for (int k = K; k < nNodes; k++)
	{
		if (dist[k]!=0)
		{
			T maxD = MERCInputMatrix<T>::maxD_KNN(nearestDist,K);
			if (dist[k] < maxD)
			{
				int maxID = MERCInputMatrix<T>::maxD_KNN_id(nearestDist,K);
				nearestID[maxID] = k;
				nearestDist[maxID] = dist[k];
			}
		}
	}
	return nearestID;
}

template <class T>
T MERCInputMatrix<T>::maxD_KNN(T *arr, int K)
{
	T maxD = 0;
	for (int x = 0; x < K; x++)
		maxD = (maxD < arr[x])?arr[x]:maxD;
	return maxD;
}

template <class T>
int MERCInputMatrix<T>::maxD_KNN_id(T *arr, int K)
{
	T maxD = 0;
	int x, id=0;
	for (x = 0; x < K; x++)
	{
		if (maxD < arr[x])
		{
			maxD = arr[x];
			id = x;
		}
	}
	return id;
}

#endif