#include "OopShell.h"

#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>   // file I/O
#include <stdio.h>
#include <iomanip>   // I/O format manipulation

using std::string;
using std::vector;
using std::map;
using std::pair;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::ios;

Runtime* Runtime::runtime = NULL;

/**
 * Runtime Constructor
 * This initializes shellHomeDir, prompt,
 * built-in commands, and loads settings from a file.
 */
Runtime::Runtime() {
	prompt = "OopShell$ ";
	initBuiltIn();
	shellHomeDir = getPwd();
	loadSettingsFile();
}

/**
 * Runtime Destructor
 * This writes settings to an output file and cleans up the memory allocations it made.
 */
Runtime::~Runtime() {
	// write aliases to user settings file
	// write path to user settings file
	// write prompt to user settings file
	writeSettingsFile();
	// remove builtins
	map<string,BuiltInI*>::iterator bicItr = builtInCmds.begin();
	while (bicItr != builtInCmds.end()) {
		delete (*bicItr).second;
		++bicItr;
	}
//	// delete singleton instance
	delete runtime;
}

/**
 * Creates new instance of runtime if one does not already exists
 * and returns runtime.
 *
 * @return runtime pointer
 */
Runtime* Runtime::getRuntime() {
	if (runtime == NULL)
		runtime = new Runtime();
	return runtime;
}

/**
 * Handles the loading of the state of alias, prompt, and path.
 * The method searches the current working directory of OopShell for the settings file.
 */
void Runtime::loadSettingsFile() {
	string filename = "oopshell_rc";
	vector<string> v;
	string line;
	ifstream fp_in;
	fp_in.open(filename.c_str(), ifstream::in);
	// exit if settings file does not exist
	if ( !fp_in.is_open()) return;
	// process each line of settings file
	while (!fp_in.eof()) {
		v.clear();
		getline(fp_in,line);
		// ignore null lines & null settings values
		if (line.size()==0) continue;
		tokenize(&line," ",&v);
		if (v.size()<2) continue;
		// set prompt
		if (v[0].compare("prompt")==0 && v.size()==2)
			prompt = v[1];
		// set alias
		if (v[0].compare("alias")==0 && v.size()==3)
			addAlias(v[1],v[2]);
		// set path
		if (v[0].compare("path")==0 && v.size()>1) {
			addToPath(v[1]);
		}
	}
	fp_in.close();
}

/**
 * Handles saving of alias, prompt, and path.
 * The settings file will be saved in the original working directory for OopShell at launch.
 *
 * @return false if output could not be written, otherwise return true
 */
bool Runtime::writeSettingsFile() {
	// be sure to save dir in shell's default working directory
	if (!chDir(&shellHomeDir))
		return false;
	// open settings file
	ofstream fp_out;
	fp_out.open("oopshell_rc",ofstream::trunc);
	// write to settings file
	if (fp_out.is_open()) {
		//write aliases
		map<string,string>::iterator aItr = aliases.begin();
		map<string,string>::iterator aEnd = aliases.end();
		while (aItr != aEnd) {
			fp_out << "alias " << aItr->first << " " << aItr->second << endl;
			++aItr;
		}
		// write prompt
		fp_out <<"prompt "<< prompt <<endl;
		// write new paths
		vector<string>::iterator pItr = newPaths.begin();
		vector<string>::iterator pEnd = newPaths.end();
		while (pItr != pEnd) {
			// escape '/' chars
			escapeString(&(*pItr),"/");
			fp_out <<"path "<< *pItr << endl;
			++pItr;
		}
	}
	// if output could not be written, return false
	else return false;
	// otherwise return true
	return true;
}

/**
 * Creates a mapping between alias name and target.
 * This method will refuse to add an alias name/target pair that results in a cycle.
 *
 * @param word -- alias name to be added.
 * @param val -- alias value desired for association with name.
 * @return true if alias was inserted, otherwise return false
 */
bool Runtime::addAlias(string word, string val) {
	// check for cycles before inserting
	// cycle example: A->B, B->C, C->A
	string tmp = val;
	expandAlias(&tmp);
	if (word == tmp)
		return false;
	return ( aliases.insert(pair<string,string>(word,val)) ).second;
}

/**
 * This replaces cmd with a targeted command.
 *
 * This will expand aliases of aliases;
 *  e.g., if A->B, and B->C, expandAlias(cmd) will replace A with C.
 *
 * @param cmd -- string pointer to the cmd name intended for expansion.
 */
void Runtime::expandAlias(string* cmd) {
	// This loop will check to see if the aliased command is also an alias, until no more alias links can be found
	// this loop has a problem for mutually recursive aliases: e.g. if ls -> dir, then dir -> ls, this loop will never exit
	map<string,string>::iterator itr = aliases.find(*cmd);
	map<string,string>::iterator end = aliases.end();
	while (itr != end) {
		*cmd = itr->second;
		itr = aliases.find(*cmd);
	}
}

/**
 * Iterates through list of aliases on map and prints them to the screen.
 */
void Runtime::printAlias() {
	map<string,string>::iterator itr;
	itr = aliases.begin();
	while (itr != aliases.end()) {
		cout << "Alias: " << itr->first << "	Command: " << itr->second << endl;
		++itr;
	}
}

/**
 * Iterates through alias map to find the specified alias and removes it from the map.
 *
 * @param word -- string pointer to the alias name intended for removal
 * @return true if alias was found and removed, else return false
 */
bool Runtime::removeAlias(string* word) {
	map<string,string>::iterator itr = aliases.find(*word);
	if (itr != aliases.end()) {
		aliases.erase(itr);
		return true;
	} else return false;
}

/**
 * Finds and retrieves built-in command from builtInCmds map.
 *
 * @param cmd -- string pointer to the command name intended for retrieval.
 * @return built-in command object pointer
 */
BuiltInI* Runtime::getBuiltIn(string* cmd) {
	map<string,BuiltInI*>::iterator bicItr = builtInCmds.find(*cmd);
	if (bicItr != builtInCmds.end() )
		return bicItr->second;
	else return NULL;
}


/**
 * Determines if specified command is a built-in command or not.
 *
 * @param cmd
 * @return true if built-in command is found on builtInCmds map,
 * 		   otherwise return false if end of map is reached.
 */
bool Runtime::isBuiltIn(string* cmd) {
	map<string,BuiltInI*>::iterator itr;
	itr = builtInCmds.find(*cmd);
	return (itr != builtInCmds.end() ? true : false);
}

/**
 * Prints all built-in commands on builtInCmds map.
 */
void Runtime::printBuiltIn() {
	map<string,BuiltInI*>::iterator itr = builtInCmds.begin();
	while (itr != builtInCmds.end()) {
		cout << (*itr).first << " ";
		++itr;
	}
}

/**
 * Initializes built-in commands.
 */
void Runtime::initBuiltIn() {
	// set pwd to cwd
	long size = pathconf(".", _PC_PATH_MAX);
	char *buf;
	char *ptr;
	if ((buf = (char *)malloc((size_t)size)) == NULL) return;
	ptr = getcwd(buf, (size_t)size);
	setenv("PWD",ptr,1);
	free(buf);
	std::string name;
	std::string usage;
	BuiltInI* bic;

	// create "cd" command
	name = "cd";
	usage = "cd usage:\n"
			"cd [noargs]: changes directory to current user home.\n"
			"cd directory_name: changes directory to directory_name if it exists.";
	bic = new Cd(name,usage);
	builtInCmds.insert(pair<string,BuiltInI*>(name,bic));

	// create "bye" command
	name = "bye";
	usage = "bye usage:\n"
			"bye [noargs]: This will exit OopShell cleanly.";
	bic = new Bye(name, usage);
	builtInCmds.insert(pair<string,BuiltInI*>(name,bic));

	// create "alias"/"unalias" command
	name = "alias";
	usage = "alias & unalias usage:\n"
			"alias [noargs]: prints out current aliases in session.\n"
			"alias word=\"val\": creates alias word for val\n"
			"unalias word: removes alias associated with word.";
	bic = new Alias(name, usage);
	builtInCmds.insert(pair<string,BuiltInI*>(name,bic));
	name = "unalias";
	builtInCmds.insert(pair<string,BuiltInI*>(name,bic));

	// create "set" command
	name = "set";
	usage = "set usage:\n"
			"set [noargs]: prints out the current PATH and prompt variables.\n"
			"set path [directory_name]+: adds specified directory_name(s) to PATH.\n"
			"set prompt val: sets shell prompt to val.";
	bic = new Set(name, usage);
	builtInCmds.insert(pair<string,BuiltInI*>(name,bic));

	// create "pwd" command
	name = "pwd";
	usage = "pwd usage:\n"
			"pwd [noargs]: This command will display the current working directory.";
	bic = new Pwd(name, usage);
	builtInCmds.insert(pair<string,BuiltInI*>(name,bic));

	// create "clear" command
	name = "clear";
	usage = "clear usage:\n"
			"clear [noargs]: This command will clear the terminal of previous output.";
	bic = new Clr(name, usage);
	builtInCmds.insert(pair<string,BuiltInI*>(name,bic));

	// create "history"/"last" command
	name = "history";
	usage = "history & prev usage:\n"
			"history [noargs]: This command will print the session command history.\n\n"
			"prev [noargs]: This command will print the previously entered command.";
	bic = new History(name,usage);
	builtInCmds.insert(pair<string,BuiltInI*>(name,bic));
	name = "prev";
	builtInCmds.insert(pair<string,BuiltInI*>(name,bic));

	// create "help" command
	name = "help";
	usage = "Are you trying to be funny? Try entering help instead.";
	bic = new Help(name,usage);
	builtInCmds.insert(pair<string,BuiltInI*>(name,bic));
}

/**
 * Adds cmd to the command session history list.
 *
 * @param cmd command to be added to history
 */
void Runtime::addToHistory(string cmd) {
	Runtime::cmdHistory.push_back(cmd);
}

/**
 * Returns session history of commands, including invalid commands.
 *
 * @return history vector pointer
 */
vector<string>* Runtime::getHistory() {
	return &cmdHistory;
}
/**
 * Find the "best" match in the command history to partially inputed command.
 * If a match is found, *cmd will be the matched command.
 *
 * Best Match:
 *	 1. largest command which matches input
 *	 2. most recent command which matches input
 *
 * @param cmd command to be matched and replaced with best match
 * @return true if match is found
 */
bool Runtime::completeCommand(string* cmd) {
	removeToken(cmd,"\\");
	if ((*cmd).size()==0) return false;
	string match;
	vector<string>::reverse_iterator hItr = cmdHistory.rbegin();
	while (hItr != cmdHistory.rend()) {
		if ((*hItr).find(*cmd) != string::npos)
			if ((*hItr).size()>match.size())
				match = *hItr;
		++hItr;
	}
	// set the val of cmd to the best match and return true
	if (match.size()>0) {
		*cmd = match;
		return true;
	}
	// if no match was found, return false
	else return false;
}

/**
 * This takes the inputed directory and appends it to PATH.
 *
 * @param newdir directory to add to path
 * @return true if new path is added
 */
bool Runtime::addToPath(string newdir) {
	string path = getenv("PATH");
	string sep=":";
	path.append(sep+newdir);
	newPaths.push_back(newdir);
	setenv("PATH",path.c_str(),1);
	return true;
}
