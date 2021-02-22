
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h> // Used for SIGINT and SIGSTP
#include <unistd.h> // Used for getppid(), chdir
#include <sys/types.h> //Used for pid_t
#include <sys/stat.h>
#include <fcntl.h>

#define INPUTLENGTH 2048

// Function Prototypes
// Driver Functions
void isRunningProcess();
void getUserInput();
void checkForCommands();
void checkForBuiltIns();
void runCDCommand();
void runForkCommand();
void executeCommands();

// Helper Functions
void catchSigIntSignal(int sign);
void catchSigStpSignal();
void printStatus(status);
void modifyInput(char* command);

// Global variables for the program
int exitFlag = -1,
backgroundProcessFlag = 0,
SIGSTPFlag = 0,
sigStopCounter = 0,
statusCode,
backgroundProcessArray[40],
backgroundProcessCount,
childExitStatus;

char userInput[INPUTLENGTH];
pid_t childPID = -5;

int main() {

	// Setup SIGINT For Ctrl-C For It Be Ignored
	struct sigaction SIGINT_Signal = { 0 };

	// Set up handler to catch the signal
	SIGINT_Signal.sa_handler = catchSigIntSignal;
	sigfillset(&SIGINT_Signal.sa_mask);

	// Create sigaction for SIGINT
	sigaction(SIGINT, &SIGINT_Signal, NULL);


	// Setup SIGSTP For Ctrl-Z
	struct sigaction SIGSTP_Signal = { 0 };

	// Set up handler to catch the signal
	SIGSTP_Signal.sa_handler = catchSigStpSignal;
	sigfillset(&SIGSTP_Signal.sa_mask);

	// Create sigaction for SIGSTP
	sigaction(SIGTSTP, &SIGSTP_Signal, NULL);


	do {

		// Get the user input
		isRunningProcess();
		getUserInput();
		checkForCommands();
		checkForBuiltIns();

	} while (exitFlag != 1);

	printf("\nNow exiting.\n");
}

/* void isRunningProcess()
*
* Description: Checks to see if any background processes is running. Where the background processes are tracked by storing them in an array
*              If there is a terminated signal, the signal will be passed printStatus(childExitStatus) and be displayed to the user
*
* Receives: N/A
* Returns:  N/A
* Pre-Conditions: N/A
*
*/

void isRunningProcess() {

	for (int i = 0; i < backgroundProcessCount; i++) {

		if (waitpid(backgroundProcessArray[i], &childExitStatus, WNOHANG) > 0)
			printStatus(childExitStatus);
	}
}

/* void getUserInput()
*
* Description: Simply gathers the user's input and strips any possible new line characters from the input
*
* Receives: N/A
* Returns:  N/A
* Pre-Conditions: N/A
*
*/

void getUserInput() {


	// Display the 'console' to the user
	printf(": ");

	// Flush the input and acquire it from the user
	fflush(stdout);
	fgets(userInput, INPUTLENGTH, stdin);

	// Remove all possible new lines
	int newLineFoundFlag = 0;

	for (int i = 0; !newLineFoundFlag && i < INPUTLENGTH; i++) {

		if (userInput[i] == '\n') {
			userInput[i] = -'\0';
			newLineFoundFlag = 1;
		}
	}

}

/* void checkForCommands()
*
* Description: Checks for basic commands such as echo, or tell a process to run in the background.
*
* Receives: N/A
* Returns:  N/A
* Pre-Conditions: N/A
*
*/

void checkForCommands() {

	// Copy the first four characters from the user input, so we can use it to compare strings
	char smallInput[5] = { 0 };

	// Extract the first five characters and see if they says echo
	strncpy(smallInput, userInput, 4);

	if (strcmp(smallInput, "echo") != 0)
	{
		//Check if this will run in the background
		if (strchr(userInput, '&') != NULL)
			modifyInput("background");
	}


	// Check to see if there is no SIGTSP and there's an $$
	// If so, expand the $$ to show the PID
	if (strstr(userInput, "TSTP") == NULL && (strstr(userInput, "$$") != NULL))
		modifyInput("expand");

	/*// Check to see if it contains $$, if so the $$ must replaced by the pid
	if (strstr(userInput, "$$") != NULL)
		modifyInput("expand");*/

		// Catch the SIGSTP command
	if (strstr(userInput, "TSTP") != NULL)
		catchSigStpSignal();

}

/* void checkForBuiltIns()
*
* Description: Checks in for built in commands, such as CD,  status, exit.  As well it catches comment lines
*
* Receives: N/A
* Returns:  N/A
* Pre-Conditions: N/A
*
*/

void checkForBuiltIns() {

	// Check for CD
	if (strncmp(userInput, "cd", 2) == 0)
		runCDCommand();

	// Check for a comment
	else if (strcmp(userInput, " ") == 0 || strncmp(userInput, "#", 1) == 0) {
		// It's a comment! So do do anything :)
	}

	// Check for the status command
	else if (strcmp(userInput, "status") == 0) {

		printStatus(childExitStatus);
		fflush(stdout);
	}

	// Check for the exit command
	else if (strcmp(userInput, "exit") == 0)
		exitFlag = 1;

	else
		runForkCommand();

	// Reset the backgroud flag
	backgroundProcessFlag = 0;
}

/* void runCDCommand()
*
* Description: Changes the directory to the the one specified by the user.  If there is no directory, the user is brought to the home directory.
*
* Receives: N/A
* Returns:  N/A
* Pre-Conditions: N/A
*
*/

void runCDCommand() {

	// Create a directory
	char directory[256];
	char* newDirectory;

	// Get the current directory
	getcwd(directory, sizeof(directory));

	newDirectory = strstr(userInput, " ");

	if (newDirectory) {

		//Increment the new directory pointer to point to the next character
		newDirectory++;

		// Add / to the directory and concatenate the new string and then change to that directory
		strcat(directory, "/");
		strcat(directory, newDirectory);

		chdir(directory);
	}

	// Change the directory to home
	else
		chdir(getenv("HOME"));

	// Get the current directory and print it out to the user
	getcwd(directory, sizeof(directory));
	printf("\nThe current directory is: %s\n", directory);
	fflush(stdout);

}

/* void runForkCommand()
*
* Description: Runs the fork commandm which will create a child process from the parent.  The child process may run in the foreground or background.
*			   If the child is running in the background, the the parent will continue without the child.
*
* Receives: N/A
* Returns:  N/A
* Pre-Conditions: N/A
*
*/


void runForkCommand() {

	// Fork the current process to create a child
	childPID = fork();

	// Ensure the child process was created correctly
	if (childPID < 0) {

		printf("Error! There was an error creating the child progress\n");
		fflush(stdout);

		// Kill the process
		exit(1);
	}

	else if (childPID == 0) {

		//Check to see if the SIGSTPFlag has been flagged
		if (sigStopCounter > 0) {

			// Check to see if there's a kill command
			if (strstr(userInput, "kill") != NULL)
				modifyInput("kill");
		}

		executeCommands();
	}

	// Parent is working
	else {

		// Check to see if the child process is running in the background
		if (backgroundProcessFlag == 1) {

			// Add the child process ID to the array and increment the counter
			backgroundProcessArray[backgroundProcessCount] = childPID;
			backgroundProcessCount++;

			// Let the parent continue without the child
			waitpid(childPID, &childExitStatus, WNOHANG);
			backgroundProcessFlag = 0;

			printf("The background process ID is: %d\n", childPID);
			fflush(stdout);
		}

		// Let the parent wait for the child
		else {

			waitpid(childPID, &childExitStatus, 0);

			// Check to see if there was an error, and if so, assign the error
			if (WIFEXITED(statusCode))
				statusCode = WEXITSTATUS(statusCode);
		}
	}
}

/* void executeCommands()
*
* Description: Used to manage the children processes
*
* Receives: N/A
* Returns:  N/A
* Pre-Conditions: N/A
*
*/

void executeCommands() {

	char* commandAray[512];

	int size = 0,
		redirectFlag = 0,
		fileDesc,
		index = 0,
		std = 2;

	// Get the bash command
	commandAray[0] = strtok(userInput, " ");

	while (commandAray[size] != NULL) {

		//Increment size of the array and then add it
		size++;
		commandAray[size] = strtok(NULL, " ");
	}

	// Keep going until the array size isn't 0!
	while (size != 0) {

		// Check for < for file reading
		if (strcmp(commandAray[index], "<") == 0) {

			// Open the file for reading
			fileDesc = open(commandAray[index + 1], O_RDONLY, 0);

			// Ensure the file was opened without error
			if (fileDesc < 0) {

				printf("Error! Cannot open %s for reading\n", commandAray[index + 1]);
				fflush(stdout);
				exit(1);
			}

			else {

				std = 0;
				redirectFlag = 1;
			}
		}

		// Check for a redirect for output
		else if (strcmp(commandAray[index], ">") == 0) {

			// Create a file for writing thte output
			fileDesc = open(commandAray[index + 1], O_CREAT | O_WRONLY, 0755);
			std = 1; //1 = stdout					
			redirectFlag = 1;
		}

		if (redirectFlag == 1) {

			dup2(fileDesc, std);

			// Remove the redirection 
			commandAray[index] = 0;

			// Call exec and close the file
			execvp(commandAray[0], commandAray);
			close(fileDesc);
		}

		// Decrement the counter and increment i
		size--;
		index++;

		// Reset the flags
		redirectFlag = 0;
		std = -2;
	}

	// There is no redirect, thus execute the command
	if (statusCode = execvp(commandAray[0], commandAray) != 0); {

		//Output the error and exit
		printf("Error! There's no such file named %s\n", userInput);
		exit(statusCode);
	}
}

/* void catchSigIntSignal()
*
* Description: Used to detect the SIGINT signal
*
* Receives: N/A
* Returns:  N/A
* Pre-Conditions: N/A
*
*/

void catchSigIntSignal(int sign) {

	printf("\n Terminated by signal %d\n", sign);
	fflush(stdout);
}

/* void catchSigStpSignal()
*
* Description: Used to detect the SIGSTP signal
*
* Receives: N/A
* Returns:  N/A
* Pre-Conditions: N/A
*
*/

void catchSigStpSignal() {


	if (SIGSTPFlag == 0) {

		// Reset the counter for a new run
		sigStopCounter = 0;

		// Ensure background commands cannot run
		backgroundProcessFlag = 0;
		SIGSTPFlag = 1;

		// Tell the user they are entering the foreground
		printf("Now entering foreground-only mode (& is now ignored)\n");
		fflush(stdout);

		sigStopCounter++;
	}

	else {

		// Reset the flag
		SIGSTPFlag = 0;

		// Print output and flush
		printf("Leaving foreground-only mode\n");
		fflush(stdout);

		sigStopCounter++;
	}
}

/* void printStatus(int status)
*
* Description: Used to print the status of an exit value or termination signal.
*
* Receives: int status
* Returns:  N/A
* Pre-Conditions: N/A
*
*/


void printStatus(int status)
{
	if (WIFEXITED(status)) {
		int actualExit = WEXITSTATUS(status);
		printf("exit value %d\n", actualExit);
	}

	else {
		int actualSignal = WTERMSIG(status);
		printf("terminated by signal %d.\n", actualSignal);
	}
}

/* vvoid modifyInput(char* command)
*
* Description: Used to modify the user's input so the command can be executed correctly
*
* Receives: int status
* Returns:  N/A
* Pre-Conditions: N/A
*
*/


void modifyInput(char* command) {

	if (strcmp(command, "kill") == 0) {

		// Create an array and remove 11 spaces to remove the $$ and signal
		char array[INPUTLENGTH];
		int size = (strlen(userInput) - 11);

		// Copy the array and put it back to the correct format
		strncpy(array, userInput, size);
		strcpy(userInput, array);
		sprintf(array, "%d", getpid());
		strcat(userInput, array);
	}

	if (strcmp(command, "expand") == 0) {

		// Create an buffer array
		char array[INPUTLENGTH] = { 0 };

		// Remove the $$ by reducing the size by two
		int size = (strlen(userInput) - 2);

		// Copy the userInput to the new array
		strncpy(array, userInput, size);

		// Copy it back to the original array, with the $$ removed
		strcpy(userInput, array);					//Copy expandCommand back to input with correct format

		// Append the pId to the user input
		sprintf(array, "%d", getppid());
		strcat(userInput, array);
	}

	if (strcmp(command, "background") == 0) {

		// Create an empty array to store possible inputs
		char background[INPUTLENGTH] = { 0 };

		int size = (strlen(userInput) - 2);

		if (SIGSTPFlag == 0)
			backgroundProcessFlag = 1;

		// Copy the input into background
		strncpy(background, userInput, size);

		// Now copy it back with the correct format
		strcpy(userInput, background);

	}

}
