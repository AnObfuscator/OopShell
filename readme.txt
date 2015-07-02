 * ********************************************************************************
 * INTRODUCTION:
 * ********************************************************************************
This is a course project I built in 2010 for my Operating Systems class. It is a simple shell with basic functionality for executing commands, piping, and reading from/witing to files.

I have made it available for educational purposes. I found forking and piping to be somewhat challenging, so if you also have difficulties I hope this helps you.


 * ********************************************************************************
 * BUILD INSTRUCTIONS:
 * ******************************************************************************** 
 COMMAND LINE:
 Navigate to OopShell/ directory
 enter command: make all
 run executable "OopShell"
 
 
* ********************************************************************************
* USAGE INSTRUCTIONS:
* ******************************************************************************** 
 
 OopShell accepts commands of the form:
 cmd [arg]* [ | cmd [agr]*]* [ < file1] [> file2]

  OopShell will expand the character ~ as follows:
	~ -> /path/to/home/currentuser
	~word -> /path/to/home/word
	~/word -> /path/to/home/currentuser/word

  OopShell will expand cmd\\ to the best match it can find in the session command history. It will then ask for confirmation before executing.

  OopShell has the following built in commands:
  alias bye cd clear help history prev pwd set unalias 
 
  alias & unalias usage:
  alias [noargs]: prints out current aliases in session.
  alias word="val": creates alias word for val
  unalias word: removes alias associated with word.

  bye usage:
  bye [noargs]: This will exit OopShell cleanly.

  clear usage:
  clear [noargs]: This command will clear the terminal of previous output.

  help usage:
  help [noargs]: Outputs general shell usage, and a list of recognized built-in commands.
  help cmd_name: Outputs help text for built-in command cmd_name.

  history & prev usage:
  history [noargs]: This command will print the session command history.
  prev [noargs]: This command will print the previously entered command.

  pwd usage:
  pwd [noargs]: This command will display the current working directory.

  set usage:
  set [noargs]: prints out the current PATH and prompt variables.
  set path [directory_name]+: adds specified directory_name(s) to PATH.
  set prompt val: sets shell prompt to val.
 
 
 * ********************************************************************************
 * PLATFORMS TESTED:
 * ********************************************************************************
  Mac OS X 10.5, GCC 4.0.1
  Ubuntu 9.04, GCC 4.3.3
 
  All specified functionality exists.
 
  example test cmds:
  	ls -laF | grep -e o | more
	last | grep -e r > testin
	wc -l < testin
	grep -e o | wc < testin > testout
	sort | grep -e o | wc < testin > testout
 