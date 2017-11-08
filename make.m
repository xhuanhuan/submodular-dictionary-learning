% make serc matlab wrapper

clear all;
clc;
restoredefaultpath;

% debugging
% mex -v -c -g SERS/MERCCInput.cpp
% mex -v -c -g SERS/MERCOutput.cpp
% mex -v -c -g SERS/MERCDisjointSet.cpp
% mex -v -c -g SERS/MERCFunctions.cpp
% mex -v -c -g SERS/MERCLazyGreedy.cpp
% mex -g SERS/mex_sers.cpp MERCCInput.o* MERCOutput.o* MERCDisjointSet.o* MERCFunctions.o* MERCLazyGreedy.o*

mex -v -c SERS/MERCCInput.cpp
mex -v -c SERS/MERCOutput.cpp
mex -v -c SERS/MERCDisjointSet.cpp
mex -v -c SERS/MERCFunctions.cpp
mex -v -c SERS/MERCLazyGreedy.cpp
%mex SERS/mex_sers.cpp MERCCInput.o* MERCOutput.o* MERCDisjointSet.o* MERCFunctions.o* MERCLazyGreedy.o*
% delete *.o*
mex SERS/mex_sers.cpp MERCCInput.obj MERCOutput.obj MERCDisjointSet.obj MERCFunctions.obj MERCLazyGreedy.obj

delete *.obj

