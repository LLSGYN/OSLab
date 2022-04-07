#include "OSShell.h"

void shell();

void shell()
{
	char input[512] = { 0 }, cmd[16] = { 0 }, options[480] = { 0 };
	int running = 1;

	printf("Hello, user!\n");

	do
	{
		printf("$root:");
		gets(input);
		int i = 0;
		for (; i < 512 && input[i] != ' ' && input[i] != 0; i++)
		{
			cmd[i] = input[i];
		}
		for (int j = i + 1; j < 512 && input[j] != ' ' && input[j] != 0; j++)
		{
			options[j - i - 1] = input[j];
		}
		// test cmd and opertaions
		// printf("%s\n%s\n", cmd, options);
		if (!strcmp(cmd, "help"))
		{
			printf("All commands:\n");
			printf("Commands about file operation:\n");
			printf("cd [DIRECTORY]								Change the shell working directory.\n");
			printf("ls											List  information  about the FILEs (the current directory by default).\n");
			printf("mkdir [DIRECTORY]							Create the DIRECTORY(ies), if they do not already exist.\n");
			printf("pwd											Print the full filename of the current working directory.\n");
			printf("touch [FILE]								Update the access and modification times of each FILE to the current time.	A FILE argument that does not exist is created empty.\n");
			printf("Commands about process operation:\n");
			printf("kill [pid]									Send the processes identified by PID a signal to terminate it\n");
			printf("ps											Display information about the active processes.\n");
			printf("run [FILE]									Run the executable [FILE]\n");
			printf("shutdown									Power-off the operating system.\n");
			printf("Commands about memory operation:\n");
			printf("free										Display amount of free and used memory in the system.\n");
		}
		else if (!strcmp(cmd, "cd"))
		{
			// pass
		}
		else if (!strcmp(cmd, "ls"))
		{
			// pass
		}
		else if (!strcmp(cmd, "mkdir"))
		{
			// pass
		}
		else if (!strcmp(cmd, "pwd"))
		{
			// pass
		}
		else if (!strcmp(cmd, "touch"))
		{
			// pass
		}
		else if (!strcmp(cmd, "kill"))
		{
			// pass
		}
		else if (!strcmp(cmd, "ps"))
		{
			showAllProcess();
		}
		else if (!strcmp(cmd, "run"))
		{
			// pass
		}
		else if (!strcmp(cmd, "shutdown"))
		{
			exit(0);
		}
		else if (!strcmp(cmd, "free"))
		{
			// pass
		}
		else
		{
			printf("%s: command not found. You can use help to see all the commands.\n", cmd);
		}
		memset(input, 0, strlen(input));
		memset(cmd, 0, strlen(cmd));
		memset(options, 0, strlen(options));
	} while (running);

	return;
}