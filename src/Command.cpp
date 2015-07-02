#include "OopShell.h"

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

using std::cout;
using std::endl;
using std::string;
using std::vector;

pid_t Command::gpid = 0;

/**
 * Execute Command
 *
 * If the command is built-in, it will execute in the current process and return.
 *
 * If the command is standard, execute will:
 * 1. fork a process, and store pid
 *    -- set child process group id if group id is initilized;
 *    -- otherwise, initialize group id to child pid and set child process group id.
 * 2. open, set, and close file descriptors for input & output.
 * 3. execute the command
 *
 * @return 0 if execution is successful, otherwise set ERROR_MSG and return -1.
 */
int Command::execute() {
	Runtime* runtime = Runtime::getRuntime();
	// execute builtin cmd
	if (builtIn==true) {
		BuiltInI* bii = (*runtime).getBuiltIn(&cmd);
		if (!(*bii).execute(&args)) {
			ERROR_MSG = (*bii).ERROR_MSG;
			return -1;
		}
		return 0;
	}
	// execute regular cmd
	pid = fork();
	// return error if fork fails
	if (pid == -1) {
		ERROR_MSG = "Failed to fork cmd "+cmd;
		return -1;
	}
	if(pid == 0) {
		// Set Input for Pipes & Files
		if (inputType==PIPE || inputType==FILEIO) {
			// redirect stdin (0) to fdIn (the read end of a pipe) then close fdIn
			dup2(fdIn,STDIN_FILENO);
			close(fdIn);
		}
		// Set Output for Pipes & Files
		if (outputType==PIPE || outputType==FILEIO) {
			// redirect stdout (1) to fdOut (the write end of a pipe) then close fdOut
			dup2(fdOut,STDOUT_FILENO);
			close(fdOut);
		}
		childState = evalCmd(&args);
		exit(0);
	}
	// Parent cleans up after child exits
	else {
		// set group id for child process
		if (gpid<=0)
			gpid = pid;
		setpgid(pid,gpid);
		// for pipes & files, we send an EOF to the parent's reference to fd by closing it
		// this signals a process reading that fd to stop reading & start executing
		if (outputType==PIPE || outputType==FILEIO)
			close(fdOut);
		//otherwise pipe output to stdout
		else dup2(fdOut,STDOUT_FILENO);
	}
	return 0;
}

/**
 * This method waits on the forked process associated with member variable pid.
 */
void Command::wait() {
	if (pid!=0) waitpid(pid,&childState,0);
}

/**
 * Setter method for Command File descriptors
 * @param fdIni -- input File Descriptor
 * @param fdOuti -- output File Descriptor
 */
void Command::setFd(int fdIni, int fdOuti) {
	fdIn = fdIni;
	fdOut = fdOuti;
}

/**
 * Evaluate Command
 *
 * This will execute the standard command given in the input vector, with associated arguments.
 *
 * IF the command is fully qualified, it will be executed directly.
 * Otherwise, the execution will search the current PATH variable, plus the current working directory
 * for the command.
 *
 * @param v -- vector of command & args in form { cmd, arg0, ... argn }
 * @return 0 if command is successful, else set ERROR_MSG and return execvp failure code.
 */
int Command::evalCmd(vector<string>* v) {
	// add cwd to path (it will only exist during this child process execution) so it is searched by execvp
	string path = getenv("PATH");
	string sep=":";
	string pwd = getPwd();
	path.append(sep+pwd);
	setenv("PATH",path.c_str(),1);
	// copy the vector strings into char* array for exec
	char *args[(*v).size()+1];
	for (size_t i=0;i<(*v).size();i++)
		args[i] = const_cast<char *>((*v)[i].c_str());
	args[(*v).size()] = '\0'; // null termination to keep exec happy
	int result = execvp(args[0], args);
	if (result==0)
		return result;
	else {
		cout << "Command " << (*v)[0] << " was not found." << endl;
		return result;
	}
	// this should never execute
	return -1;
}

/**
 * Prints out the internal state of the command as a table.
 *
 * @param header used as the caption for the printed state table.
 */
void Command::printState(string* header) {
	cout << endl << "********** " << *header << " **********";
	cout << endl << "COMMAND: " << cmd;
	cout << endl << "ARGS: ";
	vector<string>::iterator argItr = args.begin();
	vector<string>::iterator argEnd = args.end();
	while (argItr != argEnd) {
		cout << " " << *argItr;
		++argItr;
	}
	cout << endl << "BUILT IN: " << builtIn;
	cout << endl << "INPUT TYPE: " << inputType;
	cout << endl << "OUTPUT TYPE: " << outputType << endl;
	cout << endl << "****************************************"<< endl;
}

/**
 * Convenince method to return the size of the list of commands stored in CommandList.
 *
 * @return number of commands to execute
 */
int CommandList::size() {
	return cmdV.size();
}
