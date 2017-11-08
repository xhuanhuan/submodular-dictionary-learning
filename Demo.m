% =========================================================================
% An example code for the algorithm proposed in
%
%   Zhuolin Jiang, Guangxiao Zhang, Larry S. Davis.
%   "Submodular Dictionary Learning for Sparse Coding", CVPR 2012.
%
% Author: Zhuolin Jiang (zhuolin@umiacs.umd.edu)
% Date: 05-20-2013
%
% parameter setting:
%    dictsize          - dictionary size
%    lambda            - relative contribution between entropy rate and 
%                       discriminative term
%    beta              - normalization factor
%    KNN               - parameter for k-nearest-neighbor graph construction
%    nClasses          - number of classes
%    nCC               - number of connected components
%    nTrainSamples     - number of training samples per class
%    sparsity          - sparsity constraint 
% =========================================================================

function Demo()

clear all;
%clc;
addpath(genpath('./libs/OMPbox'));
addpath(genpath('./libs/ksvdbox'));

% loading the dataset
% load('sc1.mat');
% featureMat=sc_fea;
% labelMat=zeros(3,60516);
% labelMat(1,1:20214)=1;
% labelMat(2,20215:55683)=1;
% labelMat(3,55684:60516)=1;
load('.//trainingdata//spatialpyramidfeatures4caltech101.mat','featureMat','labelMat');

%% parameter settings
dictsize = 510;  
lambda = 0.01; 
beta = 0.01;  
KNN = 30; 
nClasses = 102;  
% nClasses = 3;  
nCC = dictsize; 
nTrainSamples = 30; 
% nTrainSamples = 1000; 
sparsity = 30;
roundnum = 1;

rrate = zeros(1,roundnum);
for roundid = 1:roundnum
    fprintf('round:%d\n',roundid);
   
    % obtain training and testing features by random sampling
    [testing_feats,H_test,training_feats,H_train] = obtaintraingtestingsamples(featureMat,labelMat,nTrainSamples);
   
    % label vector for training and testing data
    label_train = zeros(size(training_feats,2),1);
    label_test = zeros(size(testing_feats,2),1);
    for ii=1:size(H_train,2)
        [maxv,maxind] = max(H_train(:,ii));
        label_train(ii) = maxind;
    end
    for ii=1:size(H_test,2)
        [maxv,maxind] = max(H_test(:,ii));
        label_test(ii) = maxind;
    end
   
    % normalization
    training_feats = normcols(training_feats);
     
    % Dictionary Training
    tic;
    [outlabels1] = mex_sers(training_feats',label_train,nClasses,nCC,lambda,beta,KNN,0);%×ÓÄ£¾ÛÀà
    outlabels1 = outlabels1 + 1;
    D1 = clusteringcenter(outlabels1,nCC,training_feats',label_train,nClasses);
    D1 = D1./repmat(sqrt(sum(D1.*D1,1)),size(D1,1),1);
    toc
    
    % classifier training for SERC
    G = D1'*D1;
    X1 = omp(D1'*training_feats,G,sparsity);
    W1 = (inv(X1*X1'+ eye(size(X1*X1')))*X1*H_train')';
    
    % testing 
    [label_pred1, accuracy1] = classification(D1, W1, testing_feats, label_test, sparsity);
    fprintf('classification accuracy for SDL: %f\n', accuracy1);
    rrate(roundid) = accuracy1;
end
fprintf('average classification accuracy for SDL %f with dictionary size %d\n',mean(rrate),dictsize);
end


function [label_pred, accuracy] = classification(D, W, testdata, labeltest, sparsity)
% classification process
% Inputs
%     D          - learned dictionary
%     W          - classifier parameter
%     testdata   - testing features
%     labeltest  - label vector for testing features
%     sparsity   - sparsity threshold
% Outputs
%     label_pred - predicted labels for testing features
%     accuracy   - classification accuracy

% sparse coding for testdata

G = D'*D;
sparsecodes = omp(D'*testdata,G,sparsity);

errnum = 0;
err = [];
label_pred = [];
for featureid=1:size(testdata,2)
    spcode = sparsecodes(:,featureid);
    score_est =  W * spcode;
    label_gt = labeltest(featureid);
    [maxv_est, label_est] = max(score_est);  % classifying
    label_pred = [label_pred; label_est];
    if(label_est~=label_gt)
        errnum = errnum + 1;
        err = [err;errnum featureid label_gt label_est];
    end
end
accuracy = (size(testdata,2)-errnum)/size(testdata,2);
end

function cc = clusteringcenter(label,nCC,data,labelorigin,nClass)
% computer cluster centers
% Inputs
%      label      - cluster ID list
%      ncc        - number of connected components
%      data       - training features
% Outputs
%      cc         - cluster centers
%

t=1:nClass;
s=sum(t);
cc = zeros(nCC,size(data,2));
for i = 1: nCC
    index=find(label==i);
    dt=data(index,:);
    lb=labelorigin(index);
    
    sumcount=0;
    for j=1:nClass
        loc=find(lb==j);
        count=size(loc,1);
        if count>0
            cc(i,:) = cc(i,:)+sum(dt(loc,:)*(j/s),1);
            sumcount=sumcount+1;
        end
    end
    cc(i,:)=cc(i,:)/sumcount;
end
cc = cc'/nCC;
end
