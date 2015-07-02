#include "OopShell.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
// these are for c file handling
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using std::cout;
using std::endl;
using std::string;
using std::vector;

/**
 * Constructor for Executor object.
 *
 * @param sinput -- pointer to parsed CommandList.
 */
Executor::Executor(CommandList* sinput) {
	input = sinput;
	cvItr = (*input).cmdV.begin();
	cvEnd = (*input).cmdV.end();
}

/**
 * Destructor for Executor object.
 * This attemts to prevent Executor from ending before finish is called,
 * to clean up any child processes that may remain.
 */
Executor::~Executor() {
	finish();
}

/**
 * Convenience method to facilitate Executor usage.
 *
 * @return true if Executor has remaining Commands queued for execution, otherwise returns false.
 */
bool Executor::hasNext() {
	return cvItr==cvEnd ? false : true;
}

/**
 * This method executes the next Command available on Executor's queue, if one exists.
 * If no command from the queue has been executed yet, it will call buildFds before execution.
 *
 * @return true if execution was successful, otherwise set ERROR_MSG and return false.
 */
bool Executor::execNext() {
	if (!hasNext()) return false;
	if (cvItr == (*input).cmdV.begin()) {
		bool bp = buildFds(&(*input).cmdV,(*input).inputFile,(*input).outputFile);
		if (!bp) return false;
	}
	if ((*cvItr).execute() >= 0) {
		++cvItr;
	} else {
		ERROR_MSG = (*cvItr).ERROR_MSG;
		return false;
	}
	return true;
}

/**
 * This method preps each Command object for execution:
 * 1. build pipes and open files as necessary, determined by number of commands, IOFLAG settings and Command position.
 * 2. set the appropriate values in each Command object.
 *
 * @param v vector of commands that will be executed
 * @param inFileName the input file name; this should be of size 0 if no input file is specified.
 * @param outFileName the output file name; this should be of size 0 if no output file is specified.
 * @return true if all pipes and files were able to be created and set.
 */
bool Executor::buildFds(vector<Command>* v, string inFileName, string outFileName) {
	// create & initialize n-1 pipes for n commands
	int pipeNum = (*v).size()-1;
	int fda[pipeNum][2];
	int i;
	for (i=0;i<pipeNum;i++) {
		if (pipe(fda[i])==-1) {
			ERROR_MSG = "Pipe failed ";
			return false;
		}
	}
	// for each command, set input & output file descriptors
	for (i=0;i<=pipeNum;i++) {
		int fdIn,fdOut, pp;
		Command* cmd = &(*v).at(i);
		/*
		 * build input fd
		 */
		switch ( (*cmd).inputType ) {
			case STDIO:
				// set FD to STDIN
				fdIn = STDIN_FILENO;
				break;
			case FILEIO:
				// set FD to file
				fdIn = open(const_cast<char *>(inFileName.c_str()), (O_RDONLY));
				if (fdIn<0) {
					ERROR_MSG = "Could not open input file "+inFileName;
					return false;
				}
				break;
			case PIPE:
				// set FD to previous pipe, which is i-1
				pp = i-1;
				if (pp<pipeNum && pp>=0)
					fdIn = fda[pp][READ];
				// catch possible error condition where i is out-of-bounds.
				else {
					ERROR_MSG = "input pipe does not exist";
					return false;
				}
				break;
			default:
				ERROR_MSG = "Could not build input for cmd "+(*cmd).cmd;
				return false;
				break;
		}
		/*
		 * build output fd
		 */
		switch ( (*cmd).outputType ) {
			case STDIO:
				fdOut = STDOUT_FILENO;
				break;
			case FILEIO:
				// set FD to input file
				fdOut = open(const_cast<char *>(outFileName.c_str()),(O_CREAT|O_RDWR|O_TRUNC),0666);
				if (fdOut<0) {
					ERROR_MSG = "Could not open output file "+outFileName;
					return false;
				}
				break;
			case PIPE:
				// set FD to the next pipe, which is i
				if (i<pipeNum)
					fdOut = fda[i][WRITE];
				//catch possible error condition where i is out-of-bounds.
				else {
					ERROR_MSG = "output pipe does not exist ";
					return false;
				}
				break;
			default:
				ERROR_MSG = "Could not build output for cmd "+(*cmd).cmd;
				return false;
				break;
		}
		(*cmd).setFd(fdIn,fdOut);
	}
	// happily exit method
	return true;
}

/**
 * This method should be called anytime Executor is finished executing, regardless of success.
 * It will clean up children processes and open file descriptors.
 */
void Executor::finish() {
	// if not all commands executed, we want to only wait on the commands that did
	if (cvItr != cvEnd)
		cvEnd = cvItr;
	// restart iterator
	cvItr = (*input).cmdV.begin();
	// wait on all children
	while (cvItr!=cvEnd) {
		(*cvItr).wait();
		++cvItr;
	}

}
