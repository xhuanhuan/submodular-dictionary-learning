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

// Entropy Rate Superpixel Segmentation
#include "mex.h"
#include "matrix.h"
#include "MERCLazyGreedy.h"
#include "MERCInputMatrix.h"
#include "MERCOutputImage.h"
#include "DataMatrix.h"
#include "LabelVectors.h"
#include <iostream>
#include <string>

using namespace std;

void mexFunction(int nlhs, mxArray *plhs[ ],int nrhs, const mxArray *prhs[ ]) 
// Arguments:
//		- nlhs: Number of expected output mxArrays
//		- plhs: Array of pointers to the expected output mxArrays
//		- nrhs: Number of input mxArrays
//		- prhs: Array of pointers to the input mxArrays.
//				In this code, require:
//					prhs[0]:(double) data matrix;
//					prhs[1]:(double) label vector;
//					prhs[2]:(int) number of classes
//					prhs[3]:(int) nC number of components
//					prhs[4]:(double) lambda
//					prhs[5]:(double) sigma
//					prhs[6]:(int) K connection (k-nearest-neighbors)
//					prhs[7]:(int) mode==0:Supervised; mode==1: original unsupervised
{
	int nClasses;
	int nC = 0;
	double lambda,sigma;
	int K;
	int mode;
	int row,col;
	double *data; // pointer to the M-by-N data matrix
	double *pLabel;// pointer to the M label vector
	double *pNCls,*pNCmp,*pLambda,*pSigma,*ptrK,*ptrmode;
	double *out, *outlabels, *labelVect;
	//size_t width,height;
	
	MERCLazyGreedy merc;	
	
///////////////////////////////////////////////////////////////////////////////
//		Part 1: Arguments parsing
///////////////////////////////////////////////////////////////////////////////
	if(!(nrhs==8))
	{
		mexPrintf("Syntax Error!!!\n");
		mexErrMsgTxt("[labels] = mex_ers(data,label,nCls,nCC,lambda,sigma,KNN,mode)\n");
	}
	
	data  = mxGetPr(prhs[0]);
	pLabel = mxGetPr(prhs[1]);
	pNCls = mxGetPr(prhs[2]);
	pNCmp = mxGetPr(prhs[3]);
	pLambda = mxGetPr(prhs[4]);
	pSigma = mxGetPr(prhs[5]);
	ptrK = mxGetPr(prhs[6]);
	ptrmode = mxGetPr(prhs[7]);

	nClasses = (int)(*pNCls);
	nC = (int)(*pNCmp);
	lambda = *pLambda;
	sigma = *pSigma;
	K = (int)(*ptrK);
	mode = (int)(*ptrmode);


	if(mode==0)
	{
		//mexPrintf("Supervised Entropy Rate Clustering Version 0.1!\n");
		mexPrintf("Submodular Dictionary Learning Version 0.1!\n");
	}

///////////////////////////////////////////////////////////////////////////////
//		Part 2: Manipulate the data matrix
///////////////////////////////////////////////////////////////////////////////	
	
	// ********* Requirement for the data matrix: M-by-N, points in rows and variables in columns
	int nPoints = mxGetM(prhs[0]);
	int nFeatures = mxGetN(prhs[0]);
	
	if (!(mxGetM(prhs[1])==nPoints))
	{
		mexErrMsgTxt("Label number does not agree with the data.");
	}
	// check the dimension of the label input
	int dim_label = mxGetN(prhs[1]);
	if (!((nClasses==dim_label)||(dim_label==1)))
	{
		mexErrMsgTxt("Number of classes does not agree with the label input matrix.");
	}

	DataMatrix<float> inputMatrix;
	LabelVectors clusterLabels;
	MERCInputMatrix<float> input;
	inputMatrix.Resize(nFeatures, nPoints, false);
	clusterLabels.Resize(nClasses, nPoints, false);
	clusterLabels.Init();

	int oneLabel;
	for (row=0; row<nPoints; row++)
	{
		if (dim_label ==1)
		{
			oneLabel = (int)mxGetPr(prhs[1])[row]-1;
			clusterLabels.access[row][oneLabel] = 1;
		}
		else
		{
			for (col=0; col<dim_label; col++)
				clusterLabels.access[row][col]=pLabel[row+col*nPoints];
		}
		for (col=0; col<nFeatures; col++)
		{
			inputMatrix.access[row][col]=(float)data[row+col*nPoints];
		}
	}

 /*   for (row=0; row<nPoints; row++)
	 {
		 for (col=0; col<dim_label; col++)
		 { 
			 mexPrintf("%d ",clusterLabels.access[row][col]);
			 mexEvalString("pause(0.001);");
		 }
		 mexPrintf("\n");
	 }*/
     

	
///////////////////////////////////////////////////////////////////////////////
//		Part 3: Entropy rate superpixel segmentation
///////////////////////////////////////////////////////////////////////////////
		
	input.ReadMatrix(&inputMatrix,K);	// construct an initial graph, and compute the distance matrix.
	input.ReadLabels(&clusterLabels,nPoints);
	// Entropy rate superpixel segmentation
	merc.ClusteringTreeIF(input.nNodes_,input,0,sigma,lambda*1.0*nC,nC,mode);
	vector<int> reps = MERCOutput::FindClusterRep(merc.disjointSet_);
	// Reassign the labels to the final clusters
	vector<int> label = MERCOutputImage::DisjointSetToLabel(merc.disjointSet_);

	// Allocate memory for the labeled image.
	plhs[0] = mxCreateDoubleMatrix(nPoints, 1, mxREAL);
	plhs[1] = mxCreateDoubleMatrix(nC,nClasses,mxREAL);
	plhs[2] = mxCreateDoubleMatrix(nPoints, nClasses, mxREAL);
	out =  mxGetPr(plhs[0]);
	outlabels = mxGetPr(plhs[1]);
	labelVect = mxGetPr(plhs[2]);

	// Fill in the labeled image
	for (row=0; row < nPoints; row++)
		out[row] = (double)label[row];	
	
	// Fill in the label matrix
	for (row=0; row<nC; row++)
	{
		int id = reps[row];
		for (col=0; col<nClasses; col++)
		{
			outlabels[row+col*nC] = input.labelMerged_->access[id][col];
		}
	}

	for (row=0;row<nPoints; row++)
		for (col=0; col<nClasses; col++)
			labelVect[row+col*nPoints] = input.labelMerged_->access[row][col];
	
    return;
}
