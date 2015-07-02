
#ifndef OOPSHELL_H_
#define OOPSHELL_H_

#include <vector>
#include <map>
#include <string>

/**
 * Pipe Read/Write Definitions
 */
enum PipeEnds {
	READ=0,
	WRITE=1
};

/**
 * IOtype for class Command
 * Used to identify what type of File Descriptor Command should use for input & output
 */
enum IOtype {
	PIPE,
	STDIO,
	FILEIO
};

/**
 * Class Command
 * This encapsulates a command, as identified by scanner
 * It is initialized by Scanner, then executed by Executor
 *
 * Members:
 *	 ERROR_MSG -- if a method encounters an error, it will set ERROR_MSG to the error it encountered.
 *	 cmd -- command name to execute by execvp
 *	 args -- arg list for execvp
 *	 builtIn -- true if command is a builtin command
 *	 inputType, outputType -- used by Executor to identify what kind of input & output File Descriptors to use
 *	 gpid -- reference to the group ID of all child processes spawned by Command
 *	 (fdIn), (fdOut) -- store references to the File Descriptors for Input and Output
 *	 (pid) -- stores the pid for the forked child process executing cmd
 *	 (childState) -- holds exec state for child process pid
 *
 * Methods:
 *	 setFd -- used to initialize File Descriptors
 *	 execute -- called to execute cmd using fork & exec
 *	 printState -- convenience method to display internal state of Command to console
 *	 wait - tells command to wait on its child process
 *	 (evalCmd) -- helper method for execute
 */
class Command {
public:
	std::string ERROR_MSG;
	std::string cmd;
	std::vector<std::string> args;
	bool builtIn;
	IOtype inputType, outputType;
	static pid_t gpid;
	void setFd(int fdIn, int fdOut);
	int execute();
	void wait();
	void printState(std::string* header);
private:
	int fdIn, fdOut;
	pid_t pid;
	int childState;
	int evalCmd(std::vector<std::string>* v);
};

/**
 * Class CommandList
 * This class encapsulates a vector of Commands built by the Scanner class.
 *
 * Members:
 *	 inputFile, outputFile -- these hold the file names associated with file IO. they are blank if no file is used.
 *	 (cmdV) -- this is the vector of Commands, in order of intended execution.
 *
 * Methods:
 *	 size -- size of command vector
 */
class CommandList {
public:
	std::string inputFile, outputFile;
	std::vector<Command> cmdV;
	int size();
//	bool hasNext();
//	Command* getNext();
//	void resetItr();
private:
//	std::vector<Command>::iterator begin;
//	std::vector<Command>::iterator end;
};

/**
 * Class Scanner
 * This class scans a line of input and parses it into a series of Command objects.
 *
 * Members:
 *	 ERROR_MSG -- if a method encounters an error, it will set ERROR_MSG to the error it encountered.
 *	 FILE_IN_TOKEN,FILE_OUT_TOKEN,PIPE_TOKEN,ARG_SEP -- used for command parsing
 *	 input -- The CommandList class of Commands built from the parsed user input.
 *	 readEOF -- this is set to "true" if readLine encountered an EOF
 *
 * Methods:
 *	 readLine -- reads and proccesses a line from cin. It will return false on EOF or parse error.
 *	 getInput -- returns reference to the CommandList class holding parsed user input
 *	 (parse) -- parses the input read in from the shell prompt and builds a CommandList class
 *	 (verifyInput) -- validates user input structure
 *	 (expandTilde) -- expands ~ character
 *	 (verifyNotFirstOrLast) -- helper method for verify input
 */
class Scanner {
public:
	Scanner();
	std::string ERROR_MSG;
	std::string FILE_IN_TOKEN;
	std::string FILE_OUT_TOKEN;
	std::string PIPE_TOKEN;
	std::string ARG_SEP;
	CommandList input;
	bool readEOF;
	bool readLine();
	CommandList* getInput();
private:
	bool parse(std::string input);
	bool verifyInput(std::string* input);
	void expandTilde(std::string* val);
	bool verifyNotFirstOrLast(std::string* in, std::string token);
};

/**
 * Class Executor
 * This class takes in a CommandList class, and manages the execution of the Commands.
 *
 * Members:
 *	 ERROR_MSG -- if a method encounters an error, it will set ERROR_MSG to the error it encountered.
 *	 (input) -- pointer to CommandList received during initialization.
 *	 (cvItr) -- iterator over CommandList's cmd list data structure.
 *	 (cvEnd) -- convenience pointer to the end of CommandList's cmd list data structure.
 *
 * Methods:
 *	 hasNext -- true if there are still Commands to be executed.
 *	 execNext -- process and execute the next Command.
 *	 finish -- clean up any loose "threads" (so to speak) left "hanging" (if you will) after all Commands are executed.
 *	 (buildFds) -- Builds & sets pipe & file File Descriptors for Command objects before execution.
 *
 */
class Executor {
public:
	std::string ERROR_MSG;
	Executor(CommandList* input);
	~Executor();
	bool hasNext();
	bool execNext();
	void finish();
private:
	CommandList* input;
	std::vector<Command>::iterator cvItr;
	std::vector<Command>::iterator cvEnd;
	bool buildFds(std::vector<Command>* v, std::string inFileName, std::string outFileName);
};

/**
 * Class Builtin
 * This is a base class for all built-in commands.
 *
 * Members:
 *	 ERROR_MSG -- if a method encounters an error, it will set ERROR_MSG to the error it encountered.
 *	 USAGE -- Help text for builtin function
 *	 name -- referenced command name
 *
 * Methods:
 *	 execute -- Method to handle built-in command behavior. This should be overriden in all children.
 */
class BuiltInI {
public:
	std::string ERROR_MSG;
	std::string USAGE;
	BuiltInI(std::string cmdName, std::string usage);
	virtual ~BuiltInI();
	virtual bool execute(std::vector<std::string>* args);
	std::string name;
};
/**
 * Class Cd
 * Encapsulates "cd" cmd
 */
class Cd : public BuiltInI {
public:
	Cd(std::string name, std::string usage) : BuiltInI(name,usage) {}
	bool execute(std::vector<std::string>* args);
private:
	void finishCd();
};

/**
 * Class Set
 * Encapsulates "set" cmd
 */
class Set: public BuiltInI {
public:
	Set(std::string name, std::string usage) : BuiltInI(name, usage) {}
	bool execute(std::vector<std::string>* args);
};

/**
 * Class Alias
 * Encapsulates alias and unalias cmds
 */
class Alias: public BuiltInI {
public:
	Alias(std::string name, std::string usage) : BuiltInI(name, usage) {}
	bool execute(std::vector<std::string>* args);
};

/**
 * Class Bye
 * Encapsulates bye cmd
 */
class Bye: public BuiltInI {
public:
	Bye(std::string name, std::string usage) : BuiltInI(name, usage) {}
	bool execute(std::vector<std::string>* args);
};

/**
 * Class Pwd
 * Encapsulates pwd cmd
 */
class Pwd: public BuiltInI {
public:
	Pwd(std::string name, std::string usage) : BuiltInI(name, usage) {}
	bool execute(std::vector<std::string>* args);
};

/**
 * Class Clear
 * Encapsulates clear cmd
 */
class Clr: public BuiltInI {
public:
	Clr(std::string name, std::string usage) : BuiltInI(name, usage) {}
	bool execute(std::vector<std::string>* args);
};

/**
 * Class History
 * Encapulates history and prev cmds
 */
class History: public BuiltInI {
public:
	History(std::string name, std::string usage) : BuiltInI(name, usage) {}
	bool execute(std::vector<std::string>* args);
};

/**
 * Class Help
 * Encapulates help cmd
 */
class Help: public BuiltInI {
public:
	Help(std::string name, std::string usage) : BuiltInI(name, usage) {}
	bool execute(std::vector<std::string>* args);
private:
	std::string help;
};

/**
 * Class Runtime
 * This class holds system-wide settings such as aliases, built in commands, the prompt, etc.
 * It provides an API to provide interaction with these settings.
 * It is implemented as a singleton.
 *
 * Methods:
 *	 getBuiltIn -- returns a pointer to the class associated with a given command
 *	 getInstance -- returns a pointer to the singleton Runtime instance
 *	 getHistory -- returns a reference to the command history vector
 *	 expandAlias -- expands aliases recursively
 *	 isBuiltIn -- checks if a command is registered as a builtin command
 *	 printBuiltIn -- prints a list of built-in commands to stdout
 *	 printAlias -- prints a list of aliases to stdout
 *	 addAlias -- adds alias to map
 *	 removeAlias -- removes specified alias from map
 *	 getHistory -- gets a vector of session command history
 *	 (initBuiltIn) -- initializes & registers all built-in commands
 *
 * Members:
 *	 prompt -- the shell prompt string
 *	 (aliases) -- map of aliases & aliased commands
 *	 (builtInCmds) -- map of built-in cmd names and associated class instances
 *	 (runtime) -- self-reference to singleton instance
 *	 (cmdHistory) -- this holds all previous commands entered in the session
 *	 (newPaths) -- new paths added to default PATH this session
 *	 (shellHomeDir) -- the original default working directory of the shell on startup
 *
 */
class Runtime {
public:
	virtual ~Runtime();
	void loadSettingsFile();
	bool writeSettingsFile();
	BuiltInI* getBuiltIn(std::string* cmd);
	std::string prompt;
	static Runtime* getRuntime();
	void expandAlias(std::string* cmd);
	bool isBuiltIn(std::string* cmd);
	void printBuiltIn();
	void printAlias();
	bool addAlias(std::string word, std::string val);
	bool removeAlias(std::string* word);
	std::vector<std::string>* getHistory();
	void addToHistory(std::string cmd);
	bool completeCommand(std::string* cmd);
	bool addToPath(std::string path);
private:
	Runtime();
	static Runtime* runtime;
	std::vector<std::string> cmdHistory;
	std::vector<std::string> newPaths;
	void initBuiltIn();
	std::map<std::string,std::string> aliases;
	std::map<std::string,BuiltInI*> builtInCmds;
	std::string shellHomeDir;
};

/**
 * Utility Methods
 */
void tokenize(std::string* input, std::string token, std::vector<std::string>* v);
void collapseTabs(std::string* input);
int countTokens(std::string* input, std::string token);
bool adjacentTokens(std::string* input, std::string token);
void removeWhiteSpaces(std::string* str);
void removeToken(std::string* str, std::string token);
void removeTrailingSpaces(std::string* str);
void removeLeadingSpaces(std::string* str);
void trimString(std::string* str);
std::string getPwd();
void escapeString(std::string* str, std::string token);
bool chDir(std::string* newdir);
void exitCleanup();

#endif /* OOPSHELL_H_ */
