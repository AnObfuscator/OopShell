#include "OopShell.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <signal.h> //for kill

using std::stringstream;
using std::cout;
using std::endl;
using std::string;
using std::vector;

/**
 * This method splits a string by a given token,
 * starting at the beginning of the string.
 * Each split result will be stored, in order, in input vector v.
 *
 * This method will ignore any nulls or single spaces which result from the split.
 *
 * @param input -- string pointer to be split
 * @param token -- token on which to split input
 * @param v -- pointer to vector which will contain results
 */
void tokenize(string* input, string token, vector<string>* v) {
	const char t = *token.c_str();
	stringstream ss(*input);
	string s;
	vector<string> sv;
	while (getline(ss, s, t)) {
		if (s.compare("")==0 || s.compare(" ")==0)
			continue;
		(*v).push_back(s);
	}
}

/**
 * This method replaces "\t" with " " characters to improve ease of processing.
 *
 * @param input -- pointer to string which will be modified
 */
void collapseTabs(std::string* input) {
	size_t pos = (*input).find('\t',0);
	while (pos!=string::npos) {
		(*input).replace(pos,1," ");
		pos=(*input).find('\t',pos);
	}
}

/**
 * This method counts the number of times token appears in input.
 *
 * @param input -- string which may contain tokens
 * @param token -- token to count
 * @return number of tokens found in input
 */
int countTokens(string* input, string token) {
	int counter = 0;
	char* val;
	string::iterator stItr = (*input).begin();
	while (stItr != (*input).end()) {
		val = &(*stItr);
		if (token.compare(val)==0) ++counter;
		++stItr;
	}
	return counter;
}
/**
 * This method determines if, at any point in string input,
 * the given token is adjacent to another token.
 *
 * @param input -- string which may contain tokens
 * @param token -- token to test
 * @return true if at least two tokens are adjacent, else false
 */
bool adjacentTokens(string* input, string token) {
	size_t pos = (*input).find(token,0);
	size_t prevPos = pos;
	while (pos!=string::npos) {
		pos = (*input).find(token,pos+1);
		if (pos == prevPos+1) return true;
		else prevPos = pos;
	}
	return false;
}

/**
 * This method removes space characters, " ", from the inputed string.
 * This is a convenience method for removeToken.
 *
 * @param str -- pointer to string which will be modified.
 */
void removeWhiteSpaces(string* str) {
	removeToken(str," ");
}

/**
 * This method removes the given token from the inputed string.
 *
 * @param str -- pointer to string which will be modified.
 * @param token -- token to be removed from *str.
 */
void removeToken(string* str, string token) {
	const char* tok = token.c_str();
	string::iterator end = remove((*str).begin(), (*str).end(), tok[0]);
	(*str).erase(end, (*str).end());
}

/**
 * This method removes " " and "\t" from the end of the inputed string.
 *
 * @param str -- pointer to string which will be modified.
 */
void removeTrailingSpaces(string* str) {
	size_t pos=(*str).find_last_not_of(" \t");
	if (pos != (*str).npos) {
		(*str) = (*str).substr(0,pos+1);
	}
}

/**
 * This method removes " " and "\t" from the beginning of the inputed string.
 *
 * @param str -- pointer to string which will be modified.
 */
void removeLeadingSpaces(string* str) {
	size_t pos=(*str).find_first_not_of(" \t");
	if (pos != (*str).npos) {
		(*str) = (*str).substr(pos, (*str).size());
	}
}

/**
 * This method removes " " and "\t" from the beginning and end of the string.
 * This is a convinience method for removeTrailingSpaces and removeLeadingSpaces.
 *
 * @param str -- pointer to string which will be modified.
 */
void trimString(string* str) {
	removeLeadingSpaces(str);
	removeTrailingSpaces(str);
}

/**
 * This method should be registered with atexit.
 * It will attempt to clean up loose processes, and
 * trigger writing shell settings to a file.
 */
void exitCleanup() {
	Runtime::getRuntime()->writeSettingsFile();
	pid_t gpid = Command::gpid;
	if (gpid>0)
		killpg(gpid, SIGKILL);
}

/**
 * Gets the present working directory.
 *
 * @return -- string describing the fully qualified path to PWD.
 */
string getPwd() {
	char *buf;
	long size = pathconf(".", _PC_PATH_MAX);
	if ((buf = (char *)malloc((size_t)size)) == NULL) return NULL;
	string pwd = getcwd(buf, (size_t)size);
	free(buf);
	return pwd;
}

/**
 * Change directory to specified dir name.
 *
 * @param dir -- new directory
 * @return true if directory was changed.
 */
bool chDir(string* dir) {
	string path = *dir;
	int size = path.size();
	char newPath [size+1];
	path.copy(newPath,size,0);
	newPath[size] = '\0';
	if(chdir(newPath)==0) {
		long size = pathconf(".", _PC_PATH_MAX);
		char *buf = (char *)malloc((size_t)size);
		char *ptr = getcwd(buf, (size_t)size);
		// set PWD = CWD
		setenv("PWD",ptr,1);
		free(buf);
		return true;
	}
	return false;
}

/**
 * BROKEN -- DO NOT USE (yet)
 * @param str
 * @param token
 */
void escapeString(string* str, string token) {
	if (countTokens(str,token)>0) {
		size_t pos = (*str).find(token,0);
		while (pos!=string::npos) {
			(*str).replace(pos,1,"\\/");
			pos=(*str).find(token,pos);
		}
	}
}
