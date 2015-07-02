/*
 * OOPSHELL TO DO LIST
 *
 * KNOWN BUGS:
 * (NONE)
 *
 * BEFORE RELEASE:
 * (NONE)
 *
 * WISH LIST:
 * TODO add hasNext/getNext methods to ScannedInput
 * TODO Move "builtin" core actions to "Runtime" methods
 * TODO Move "builtin" init into "builtin initilizer" class method
 * TODO up/down-arrow history searching
 * TODO report alias insertion failures to user
 * TODO fix alias to accept alias word[ ]*=[ ]*"string"
 * TODO find better way to handle saving paths from one session to next (maybe use env struct?)
 * TODO add the ability to remove paths (other than editing the oopshell_rc file)
 * TODO fix "saving settings..." execution after forked cmd execution. (probably have to check child pid var ==0)
 * TODO better error handling / passing
 * TODO output errno for failed system commands
 * TODO better input validation for input/output file names
 * TODO clean up "dirty" looking methods
 * TODO Test script files: OopShell < testscript
 * TODO check dir validity before adding to path
 * TODO Externalize String Constants
 *
 */

#include "OopShell.h"

#include <iostream> // for cout, endl
#include <stdlib.h> // for atexit

using std::cout;
using std::endl;

/**
 * This is the main shell loop that handles execution and termination of the shell.
 */
int main() {
	atexit(exitCleanup);
	Runtime* runtime = Runtime::getRuntime();
	cout << "Welcome to OopShell...";
	while (true) {
		cout << endl << (*runtime).prompt << " ";
		Scanner scanner;
		if (!scanner.readLine()) {
			if (scanner.readEOF) break;
			cout << scanner.ERROR_MSG << endl;
			continue;
		}
		Executor executor(scanner.getInput());
		while (executor.hasNext()) {
			if (!executor.execNext()) {
				cout << executor.ERROR_MSG << endl;
				break;
			}
		}
		executor.finish();
	}
	exit(0);
}
