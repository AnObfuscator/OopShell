#include "OopShell.h"

#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <stdlib.h>

using std::cout;
using std::endl;
using std::string;
using std::map;
using std::pair;
using std::vector;

/**
 * Constructor for objects of type BuiltInI.
 *
 * @param cmdName -- name by which the command will be referenced by the system.
 * @param usage -- help text for the command.
 */
BuiltInI::BuiltInI(string cmdName, string usage) {
	name = cmdName;
	USAGE = usage;
}

/**
 * Destructor for BuiltInI -- currently unused.
 */
BuiltInI::~BuiltInI() { }

/**
 * Base method implementation of BuiltInI.
 * This should be overridden by any object of type BuiltInI.
 */
bool BuiltInI::execute(vector<string>* args) {
	return false;
}
/**
 * Handles cd command input verification and executes cd command.
 * If no arguments are specified, user is taken to the home directory.
 * If one argument is specified, user is taken to the specified directory.
 * If argument is not a valid directory name, return false and set ERROR_MSG.
 * If more than one argument is passed, return false and set ERROR_MSG.
 *
 * @param args argument vector of the form {cmd, arg0, ... argn}
 * @return true if directory is changed, otherwise return false and set ERROR_MSG
 */
bool Cd::execute(vector<string>* args) {
	vector<string>* cmdV = args;
	// handle no arg: change cwd to home
	if ((*cmdV).size()==1) {
		if (chdir(getenv("HOME"))==0)
			finishCd();
	}
	// change cwd to inputed path
	else if ((*cmdV).size()==2) {
		string path = (*cmdV)[1];
		int size = path.size();
		char newPath [size+1];
		path.copy(newPath,size,0);
		newPath[size] = '\0';
		if(chdir(newPath)==0) {
			finishCd();
		}
		// handle invalid path
		else {
			ERROR_MSG = "Failed to find new path.";
			return false;
		}
	}
	// handle invalid input
	else {
		ERROR_MSG = "Invalid usage. See help cd for usage.";
		return false;
	}
	return true;
}

/**
 * Helper method for Cd::execute.
 * Handles changing to the specified directory.
 * Prints the change in directory.
 */
void Cd::finishCd() {
	long size = pathconf(".", _PC_PATH_MAX);
	char *buf = (char *)malloc((size_t)size);
	char *ptr = getcwd(buf, (size_t)size);
	// set PWD = CWD
	setenv("PWD",ptr,1);
	cout << "Changed directory to: " << ptr << endl;
	free(buf);
}

/**
 * Handles alias/unalias
 * If command "alias" is specified with no arguments, alias list is displayed.
 * If command "alias" is specified with one argument, alias is set.
 * If command "unalias" is specified with one arguement, alias word is unaliased and taken off alias list.
 * If command "alias" is specified with an invalid argument or more than one arguments, return false and set ERROR_MSG.
 * If command "unalias" is specified with an argument that is not on the alias list, return false and set ERROR_MSG.
 *
 * @param args argument vector of the form {cmd}, {cmd, arg0, ... argn}
 * @return true if alias list is displayed, alias is set, or a command is unaliased,
 * 		   otherwise return false and set ERROR_MSG
 */
bool Alias::execute(vector<string>* args) {
	vector<string>* cmdV = args;
	Runtime* runtime = Runtime::getRuntime();
	// display current aliases
	if ((*cmdV).size()==1) {
		(*runtime).printAlias();
	}
	// remove alias from list
	else if ((*cmdV)[0].compare("unalias")==0) {
		if ((*runtime).removeAlias(&(*cmdV)[1]))
			return true;
		else {
			ERROR_MSG = "Alias not on alias list.";
			return false;
		}
	}
	// add alias to list
	else if ((*cmdV).size()==2) {
		vector<string> v;
		tokenize(&(*cmdV)[1],"=",&v);
		if (v.size()!=2) {
			ERROR_MSG = "Invalid usage. See help alias for usage.";
			return false;
		}
		removeToken(&v[1],"\"");
		return (*runtime).addAlias(v[0],v[1]);
	}

	// handle invalid input
		else {
			ERROR_MSG = "Invalid usage. See help alias for usage.";
			return false;
		}

	return true;
}


/**
 * Handles setting and displaying of PATH and PROMPT
 * If command "set" is specified with no arguments, displays current PATH and PROMPT.
 * If command "set" is specified with arguments, false is returned and ERROR_MSG is set.
 * If command "set path" is specified with one or more arguments, arguments are added to PATH.
 * If command "set path" is specified with no arguments, false is returned and ERROR_MSG is set.
 * If command "set prompt" is specified with one argument, PROMPT is set to the argument.
 *
 * @param args argument vector of the form {cmd}, {cmd, arg0, ... argn}
 * @return true if path and prompt are displayed, directories are added to PATH, or PROMPT is set,
 * 		   otherwise return false and set ERROR_MSG.
 */
bool Set::execute(vector<string>* args) {
	Runtime* runtime = Runtime::getRuntime();
	vector<string>* cmdV = args;
	// display current path
	if ((*cmdV).size() == 1) {
		cout << "path: " << getenv("PATH");
		cout << endl << "prompt: " << (*runtime).prompt << endl;
	}
	// add to path
	else if ((*cmdV)[1].compare("path")==0) {
		// handle no paths specified
		if ((*cmdV).size()<3) {
			ERROR_MSG = "Invalid usage. See help set for usage.";
			return false;
		}
		// add paths
		for (size_t i=2;i<(*cmdV).size();i++)
			(*runtime).addToPath((*cmdV)[i]);
	}
	// change prompt
	else if ((*cmdV)[1].compare("prompt")==0) {
		if ((*cmdV).size()==3)
			(*runtime).prompt=(*cmdV)[2];
	}
	// handle invalid input
	else {
		ERROR_MSG =  "Invalid usage. See help set for usage.";
		return false;
	}
	return true;
}

/**
 * Handles exiting of shell
 * If command "bye" is specified, shell is terminated.
 * False should never be returned, unless there is a serious bug in the exit() function.
 *
 * @param args argument vector of the form {cmd}
 * @return false if some form of serious error occurrs
 */
bool Bye::execute(vector<string>* args) {
	exit(0);
	return false; // This *really* should never execute
}

/**
 * Displays the previous working directory
 * If command "pwd" is specified, previous working directory is displayed.
 *
 * @param args argument vector of the form {cmd}
 * @return true, otherwise return false if insufficient memory for allocating
 */
bool Pwd::execute(vector<string>* args) {
	long size;
	char *buf;
	char *ptr;
	size = pathconf(".", _PC_PATH_MAX);
	if ((buf = (char *)malloc((size_t)size)) == NULL)
		return false;
	ptr = getcwd(buf, (size_t)size);
	cout << ptr << endl;
	free(buf);
	return true;
}

/**
 * Handles clearing of the terminal
 * If command "clear" is specified, terminal is cleared.
 *
 * @param args argument vector of the form {cmd}
 * @return true if command successfully executes
 */
bool Clr::execute(vector<string>* args) {
	system("clear");
	return true;
}

/**
 * Displays history of commands
 * If command "history" is specified, all previously used commands are displayed.
 *
 * @param args argument vector of the form {cmd}
 * @return true if command successfully executes
 */
bool History::execute(vector<string>* args) {
	Runtime* runtime = Runtime::getRuntime();
	// print entire history
	if ( (*args)[0].compare("history")==0 ) {
		vector<string>* v = (*runtime).getHistory();
		vector<string>::iterator itr = (*v).begin();
		while (itr != (*v).end()) {
			cout << (*itr) << endl;
			++itr;
		}
	}
	// print last history
	else if ( (*args)[0].compare("prev")==0 ) {
		vector<string>* v = (*runtime).getHistory();
		if ((*v).size()<=1)
			return true;
		cout << *(----(*v).end()) << endl; // I <3 C++... take that, Java
	}
	return true;
}

/**
 * Displays help menu for command usage
 * If command "help" is specified with no arguments, shell help menu is displayed.
 * If command "help" is specified with one argument, usage for the argument is displayed.
 * If command "help" is specified with an invalid argument, return false and set ERROR_MSG.
 *
 * @param args argument vector of the form {cmd}
 * @return true if help menu or command usage is displayed, otherwise return false and set ERROR_MSG
 */
bool Help::execute(vector<string>* args) {
	help = "OopShell accepts commands of the form:\n cmd [arg]* [ | cmd [agr]*]* [ < file1] [> file2]\n"
			"\nOopShell will expand the character ~ as follows:\n"
			"~ -> /path/to/home/currentuser\n"
			"~word -> /path/to/home/word\n"
			"~/word -> /path/to/home/currentuser/word\n"
			"\nOopShell will expand cmd\\\\ to the best match it can find in the session command history. It will then ask for confirmation before executing.\n"
			"\nOopShell has the following built in commands:";
	Runtime* runtime = Runtime::getRuntime();
	// print shell help
	if ((*args).size()!=2) {
		cout << help << endl;
		(*runtime).printBuiltIn();
		cout << "\n\nSee help cmd_name for more usage." << endl;
	}
	// print cmd help
	else {
		BuiltInI* cmd = (*runtime).getBuiltIn(&(*args)[1]);
		if (cmd != NULL)
			cout << (*cmd).USAGE << endl;
		else {
			ERROR_MSG = "Command not found";
			return false;
		}
	}
	// all hunky-dory
	return true;
}
