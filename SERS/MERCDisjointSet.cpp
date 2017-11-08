/*
Copyright 2011, Ming-Yu Liu

All Rights Reserved 

Permission to use, copy, modify, and distribute this software and 
its documentation for any non-commercial purpose is hereby granted 
without fee, provided that the above copyright notice appear in 
all copies and that both that copyright notice and this permission 
notice appear in supporting documentation, and that the name of 
the author not be used in advertising or publicity pertaining to 
distribution of the software without specific, written prior 
permission. 

THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, 
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
ANY PARTICULAR PURPOSE. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR 
ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES 
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN 
AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING 
OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
*/
#include "MERCDisjointSet.h"
#include <vector>
#include <algorithm>
using namespace std;

MERCDisjointSet::MERCDisjointSet(int nElements) 
{	
	nElements_ = nElements;
	nVertices_ = nElements_;
	p_ = new int [nElements_];
	size_ = new int [nElements_];
	lists_ = new MList<int> [nElements_];


	int reservedSize = (int)std::sqrt( 1.0*nElements );
	// Initialization with the cluster size and id
	for (int i = 0; i < nElements; i++) 
	{
		p_[i] = i;
		size_[i] = 1;
		lists_[i].PushBack(i);
	}
}
  
MERCDisjointSet::~MERCDisjointSet() 
{
	Release();
}

void MERCDisjointSet::Release()
{
	delete [] p_;
	delete [] size_;
	delete [] lists_;
	p_ = NULL;
	size_ = NULL;
	lists_ = NULL;

}

void MERCDisjointSet::Set(int x,int l)
{
	p_[x] = l;
}

int MERCDisjointSet::Find(int x) 
{
	// return the cluster ID
	return p_[x];
}
int MERCDisjointSet::Join(LabelVectors *lv, LabelVectors *lv_merge, int aID, int bID) 
//int MERCDisjointSet::Join(LabelVectors *lv, LabelVectors *lv_merge, int x, int y) 
{
//	int aID = Find(x);
//	int bID = Find(y);

	// The size is only maintained for cluster ID.
	int aSize = size_[aID];
	int bSize = size_[bID];

	// Update the sets
	int newID,delID;
	if( bSize < aSize )
	{
		newID = aID;
		delID = bID;
	}
	else
	{
		newID = bID;
		delID = aID;
	}

	size_[newID] = aSize+bSize;
	size_[delID] = 0;

	MListNode<int> *iter;
	// Label the elements in cluster y (delID) as x (newID)
	for(iter=lists_[delID].first_;iter!=NULL;iter=iter->next_)
		p_[iter->data_] = newID;		
	lists_[newID].Append(lists_[delID]); // append cluster y to cluster x

	MListNode<int> *lsptr = lists_[newID].first_;
	
	int *arr = new int[aSize+bSize];
	for (int i=0;i<aSize+bSize;i++)
	{
		arr[i] = lsptr->data_;
		lsptr = lsptr->next_;
	}

	// ** add the distributions of all the elements in a and b ** 
	int nClasses = lv->width();
	double *dist = new double[nClasses];

	for (int i=0;i<nClasses;i++)
		dist[i] = 0;

	for( int fID=0;fID<aSize+bSize;fID++)
	{
		int memberID = arr[fID];
		for (int i=0;i<nClasses;i++)
			dist[i]+= lv->access[memberID][i];		
	}
	// ** update the merged labelvectors with the final dist[] for the new cluster **
	for( int fID=0;fID<aSize+bSize;fID++)
	{
		int memberID = arr[fID];
		for (int i=0;i<nClasses;i++)
			lv_merge->access[memberID][i]=dist[i];
	}

	nElements_--;
	return newID;
}