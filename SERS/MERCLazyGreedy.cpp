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
#include "MERCLazyGreedy.h"
#include "mex.h"
MERCDisjointSet* MERCLazyGreedy::ClusteringTree(int nVertices,MERCInput &edges,int kernel,double sigma,double lambda,int nC,int mode)
{
	//LARGE_INTEGER t1, t2, f;
	//QueryPerformanceFrequency(&f);
	//QueryPerformanceCounter(&t1);


	int nEdges = edges.nEdges_;
	MERCDisjointSet *u = new MERCDisjointSet(nVertices);

	MERCFunctions::ComputeSimilarity(edges,sigma,kernel);
	double *loop = MERCFunctions::ComputeLoopWeight(nVertices,edges);
	double wT = MERCFunctions::ComputeTotalWeight(loop,nVertices);
	MERCFunctions::NormalizeEdgeWeight(edges,loop,wT);

	//
	// Compute initial gain and decide the weighting on the balancing term
	//
	double *erGainArr = new double [nEdges];// gain in entropy rate term
	double *bGainArr = new double [nEdges];	// gain in balancing term
	double *bGainArr1 = new double [nEdges];	//add
	for (int j=0; j<nEdges;j++)
		bGainArr[j]=0;

	double maxERGain=0,maxBGain=1e-20,maxBGain1=1e-20;
	for(int i=0;i<nEdges;i++)
	{	// (By GX)
		// Loop through all edges and find the one which maximizes the gain.
		// First, find the balancing parameter in terms of maxERGain and maxBGain
		// Then find the edge which maximizes:
		// --- Gain = erGgain + balancing * bGain

		erGainArr[i] = MERCFunctions::ComputeERGain( 
			edges.edges_[i].w_, 
			loop[edges.edges_[i].a_]-edges.edges_[i].w_,
			loop[edges.edges_[i].b_]-edges.edges_[i].w_);
		int a = u->Find(  edges.edges_[i].a_ );
		int b = u->Find(  edges.edges_[i].b_ );
		// mexPrintf("a_: %d,b_: %d \n",edges.edges_[i].a_,edges.edges_[i].b_);
		// mexPrintf("a: %d,b: %d	\n",a,b);//相等的a,a_
		if(a!=b)
		{ 
			//int aClusterID = u->Find(a);
			//int bClusterID = u->Find(b);
			// if (mode==1)
                //discriminative term
				bGainArr1[i] = MERCFunctions::ComputeBGain(nVertices, u->rSize(a), u->rSize(b) );
			// else
				bGainArr[i] = MERCFunctions::ComputeBGainSupervised(edges.labelMerged_, nVertices, a, b,u->rSize(a),u->rSize(b));
		}
		else
			bGainArr[i] = 0;
		if(erGainArr[i]>maxERGain)
			maxERGain = erGainArr[i];
		if(bGainArr[i]>maxBGain)
			maxBGain = bGainArr[i];
		if(bGainArr1[i]>maxBGain1)
			maxBGain1 = bGainArr1[i];
	}
/*	// ----- For debugging -------
	for (int i=0;i<nEdges;i++)
		mexPrintf("%d:	%f	\n",i+1,bGainArr[i]);
	// ----- End of debugging -----*/

	double balancing = lambda*maxERGain/std::abs(maxBGain);
	//double balancing1 = lambda* log( 1.0*nVertices )/ log( 1.0*nC );
	double balancing1=lambda*maxERGain/std::abs(maxBGain1);
	/*
	std::cout.precision(8);
	std::cout.setf(ios::fixed,ios::floatfield);
	std::cout<<"maxERGain = "<<maxERGain<<std::endl;
	std::cout<<"maxBGain = "<<maxBGain<<std::endl;
	std::cout<<"Balancing gain = "<<balancing<<std::endl;
	*/

	for(int i=0;i<nEdges;i++)
	{                           // entry term + discriminative term
		//mexPrintf("bGainArr %f	:\n",balancing*bGainArr[i]);
		edges.edges_[i].gain_ = erGainArr[i]+balancing*bGainArr[i]+balancing1*bGainArr1[i];
	}
		
	delete [] erGainArr;
	delete [] bGainArr;
	delete [] bGainArr1;

	// Heap
	MSubmodularHeap<MERCEdge> heap((MERCEdge *)edges.edges_,nEdges);
	heap.BuildMaxHeap();

	//
	// sequentially add edges to the graph and track cluster numbers and loop weights.
	//
	MERCEdge bestEdge;
	double timeUsed = 0;
	int cc = nVertices;	//number of clusters
	int a,b;

	while( cc > nC )
	{

	/*	if(heap.IsEmpty())
		{
			cout<<"Empty"<<endl;
			delete [] loop;
			return u;
		}
	*/
		// find the best edge to add
		bestEdge = heap.HeapExtractMax();

		// insert the edge into the graph 
		a = u->Find(  bestEdge.a_ ); // a: cluster ID which contains node bestEdge.a_
		b = u->Find(  bestEdge.b_ );

		if(a!=b)
		{
			u->Join(edges.labelv_,edges.labelMerged_,a,b);
			
//			double* ptr1 = edges.labelMerged_->access[a];
//			double* ptr2 = edges.labelMerged_->access[b];

			cc--;
			loop[bestEdge.a_] -= bestEdge.w_;
			loop[bestEdge.b_] -= bestEdge.w_;
			
		}

		//For debugging....
		//mexPrintf("%d, %d:	%f\n", bestEdge.a_,bestEdge.b_,bestEdge.gain_);
		if(heap.IsEmpty())
		{
			cout<<"Empty"<<endl;
			delete [] loop;
			return u;
		}
		
		heap.EasyPartialUpdateTree(u,edges.labelMerged_,balancing,loop,mode);
	}


	delete [] loop;
	//QueryPerformanceCounter(&t2);
	//std::cout.precision(6);
	//std::cout<<std::fixed<<"[TIME] "<<(t2.QuadPart - t1.QuadPart)/(f.QuadPart*1.0)<<" sec."<<std::endl;

	return u;
}