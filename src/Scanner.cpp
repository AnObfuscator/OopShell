#include "OopShell.h"

#include <stdlib.h>
#include <iostream> // for cin,cout,endl
#include <vector>
#include <map>
#include <string>
#include <unistd.h> // for getcwd
#include <pwd.h> // for getpwnam

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::map;

Scanner::Scanner() {
	FILE_IN_TOKEN = "<";
	FILE_OUT_TOKEN = ">";
	PIPE_TOKEN = "|";
	ARG_SEP = " ";
}

/**
 * This method reads a line from the user. It dispatches commands to be validated. Valid commands are dispached to the parser.
 *
 * If the line contains command-completion (\\), readLine offers the completed command back to the user for validation;
 * if validated, it will continue on with the validation and parsing of the completed command.
 *
 * @return true if command was valid & parsed correctly, else false. If scanner read an EOF, readEOF is set true.
 */
bool Scanner::readLine() {
	readEOF=false;
	string rawInput;
	if (getline(cin,rawInput)) {
		// process command completion
		size_t ccPos = rawInput.find("\\\\",rawInput.size()-2);
		if (ccPos != string::npos) {
			Runtime* runtime = Runtime::getRuntime();
			// find command for user to input
			if ((*runtime).completeCommand(&rawInput)) {
				cout << "Execute: " << rawInput << endl << "(Press enter to accept, any other key + enter to cancel)";
				if (cin.get()!=10) {
					ERROR_MSG = "Command canceled.";
					cin.ignore(256,10);
					return false;
				}
			}
			// if no command is found, return to prompt
			else {
				ERROR_MSG = "No matching command found.";
				return false;
			}
		}
		// execute command
		return parse(rawInput);
	}
	// handle EOF situation
	readEOF = true;
	return false;
}

/**
 * This method take a raw input string to be turned into command objects inside a CommandList class.
 * If input/output files exist, it will set the CommandList data members to those names; otherwise, the names are left empty.
 * The parser will marshall each Command object, setting IOTYPE, cmd, and args.
 * The Command objects will be stored in CommandList in intended order of execution.
 *
 * @param rawInput validated input string from the user
 * @return true if parsing was successful. Otherwise, set ERROR_MSG and return false.
 */
bool Scanner::parse(string rawInput) {
	Runtime* runtime = Runtime::getRuntime();
	// add to command history
	(*runtime).addToHistory(rawInput);
	// initial input validation
	collapseTabs(&rawInput);
	if (!verifyInput(&rawInput)) {
		ERROR_MSG = "Invalid Input. See help for usage.";
		return false;
	}
	// Handle Output File
	vector<string> v;
	tokenize(&rawInput,FILE_OUT_TOKEN,&v);
	int vs = v.size();
	if (vs>2) {
		ERROR_MSG = "Too many output files. See help for usage.";
		return false;
	}
	if (vs==2) {
		string fileName = v[1];
		trimString(&fileName);
		input.outputFile = fileName;
	} else input.outputFile.clear();
	rawInput = v[0];

	// Handle Input File
	v.clear();
	tokenize(&rawInput,FILE_IN_TOKEN,&v);
	vs = v.size();
	if (vs>2) {
		ERROR_MSG = "Too many input files. See help for usage.";
		return false;
	}
	if (vs==2) {
		string fileName = v[1];
		trimString(&fileName);
		input.inputFile = fileName;
	} else input.inputFile.clear();
	rawInput = v[0];

	// Handle Pipes
	v.clear();
	trimString(&rawInput);
	tokenize(&rawInput,PIPE_TOKEN,&v);
	int cmdc = v.size();

	for (int i=0;i<cmdc;i++) {
		// make sure command is not null
		if (v[i].size()==0) {
			ERROR_MSG = "Invalid Input: null command. See help for usage.";
			return false;
		}

		Command c;
		// First Command Special Cases
		if (i==0) {
			c.inputType = ( input.inputFile.size()==0 ? STDIO : FILEIO );
			if (cmdc>1) c.outputType = PIPE;
			else if (input.outputFile.size()>0) c.outputType = FILEIO;
			else c.outputType = STDIO;
		}
		// Last Command Special Cases
		else if (i==cmdc-1) {
			c.inputType = PIPE;
			c.outputType = ( input.outputFile.size()==0 ? STDIO : FILEIO );
		}
		// Middle Commands
		else {
			c.inputType = PIPE;
			c.outputType = PIPE;
		}
		// clean up args, expand ~, and build arg vector
		tokenize(&v[i],ARG_SEP,&(c.args));
		vector<string>::iterator argItr = c.args.begin();
		while (argItr != c.args.end()) {
			removeWhiteSpaces(&(*argItr));
			expandTilde(&(*argItr));
			++argItr;
		}
		// clean up the cmd, expand aliases, and set Command->cmd
		Runtime* runtime = Runtime::getRuntime();
		(*runtime).expandAlias(&c.args[0]);
		c.cmd = c.args[0];
		c.builtIn = (*runtime).isBuiltIn(&c.cmd);
		// Disalow executing piped/redirected built-ins
		if (c.builtIn && (v.size()>1 || input.inputFile.size()>0 || input.outputFile.size()>0)) {
			ERROR_MSG = "OopShell does not allow piping or file redirection with built-in commands.";
			return false;
		}
		input.cmdV.push_back(c);
	}
	return true;
}

/**
 * This method examines the structure of the inputed string,
 * and rejects it if it matches the following cases:
 *
 * string is empty
 * string contains only spaces
 * string contains only spaces and |,<,> tokens
 * string begins or ends with |,<,> tokens
 * order of tokens |,<,>
 * more than one token each of type < or >
 * adjacent tokens |,<,>
 *
 * @param rawInput the string to be examined
 * @return true if rawInput passes examination, else false
 */
bool Scanner::verifyInput(string* rawInput) {
	// First, don't crash on receiving "return" or [ ]*
	if ((*rawInput).size()==0) return false;
	string tmp = (*rawInput).substr(0,(*rawInput).size());
	removeWhiteSpaces(&tmp);
	if (tmp.size()==0) return false;
	// make sure "<" and ">" are unique
	if (countTokens(&tmp,FILE_IN_TOKEN) > 1) return false;
	if (countTokens(&tmp,FILE_OUT_TOKEN) > 1) return false;
	// check for adjacent pipe tokens
	if (adjacentTokens(&tmp,PIPE_TOKEN)==true) return false;
	// make sure "<" comes before ">"
	size_t pos1 = tmp.find_first_of(FILE_IN_TOKEN,0);
	size_t pos2 = tmp.find_first_of(FILE_OUT_TOKEN,0);
	if ( pos1 != string::npos && pos2 != string::npos && (pos1 > pos2)  )return false;
	// make sure all "|" comes before "<" or ">"
	// also make sure no "|" is adjacent to "<" or ">"
	size_t pos3 = tmp.find_first_of(PIPE_TOKEN,0);
	while (pos3 != string::npos) {
		// check "|" vs "<"
		if ( pos1 != string::npos )
			if (pos3 > pos1) return false;
			else if (pos1==(pos3+1)) return false;
		// check "|" vs  ">"
		if ( pos2 != string::npos )
			if (pos3 > pos2) return false;
			else if (pos2==(pos3+1)) return false;
		pos3 = tmp.find_first_of(PIPE_TOKEN,pos3+1);
	}
	// make sure first or last command is not "|", "<", or ">"
	if (!verifyNotFirstOrLast(&tmp,FILE_OUT_TOKEN)) return false;
	removeToken(&tmp,FILE_OUT_TOKEN);
	if (!verifyNotFirstOrLast(&tmp,FILE_IN_TOKEN)) return false;
	removeToken(&tmp,FILE_IN_TOKEN);
	if (!verifyNotFirstOrLast(&tmp,PIPE_TOKEN)) return false;
	removeToken(&tmp,PIPE_TOKEN);
	//  make sure actual commands exist in cmd
	if (tmp.size()==0) return false;
	// if input survived that, it deserves the chance to crash my shell
	return true;
}

/**
 * This is a convenience method for input validation.
 * It checks the beginning and end of the input string for the presence of the given token.
 *
 * @param in -- pointer to the string to be analyzed
 * @param token -- token to
 * @return true if token is not the first or last char of string* in, else false
 */
bool Scanner::verifyNotFirstOrLast(string* in, string token) {
	size_t pos1 = (*in).find_first_of(token,0);
	size_t pos2 = (*in).find_last_of(token);
	// check if token is first
	if ( pos1 != string::npos && pos1==0 )
		return false;
	// check if token is lasat
	if (pos2 != string::npos && pos2==((*in).size()-1))
		return false;
	return true;
}

/**
 * This is a getter method for the CommandList filled by the parser.
 *
 * @return pointer to CommandList member object
 */
CommandList* Scanner::getInput() {
	return &input;
}

/**
 * This method replaces "~" in the value of a string pointer with a home path, depending on context:
 * Case ~: /path/to/current/user/home
 * Case ~name: /path/to/user/name/home
 * case ~/word: /path/to/current/user/home/word
 *
 * @param val a pointer to a cmd or arg string where tilde expansion is desirable.
 */
void Scanner::expandTilde(string* val) {
	if ((*val).find("~",0)==0) {
		string sep = "/";
		string path;
		// expand ~ to /path/to/home/currentuser
		if ((*val).size()==1) {
			path.append(getenv("HOME"));
			(*val).replace(0,1,path);
		}
		// expand ~username to /path/to/home/username
		else if ((*val).find(sep,1)==string::npos) {
			(*val).replace(0,1,"");
			struct passwd* pw = getpwnam( const_cast<char *>((*val).c_str()) );
			(*val).clear();
			(*val).append(pw->pw_dir);
		}
		// expand ~/dirname to /path/to/home/currentuser/dirname
		else {
			path.append(getenv("HOME"));
			if ((*val).find(sep,1)!=1)
				path.append(sep);
			(*val).replace(0,1,path);
		}
	}
}
