#include "OSShell.h"

void shell();
int string2Int(char string[], int start);

void shell()
{
	char input[512] = { 0 }, cmd[16] = { 0 }, options[495] = { 0 };
	int running = 1;

	printf("Hello, user!\n");

	while(running)
	{
		memset(input, 0, strlen(input));
		memset(cmd, 0, strlen(cmd));
		memset(options, 0, strlen(options));
		WaitForSingleObject(printMutex, INFINITE);
		printf("$root:");
		gets(input);
		int i = 0;
		for (; i < 512 && input[i] != ' ' && input[i] != 0; i++)
		{
			cmd[i] = input[i];
		}
		if (i >= 512)
		{
			printf("Error: input is too long.\n");
			continue;
		}
		for (int j = i + 1; j < 512 /*&& input[j] != ' '*/ && input[j] != 0; j++)
		{
			options[j - i - 1] = input[j];
		}
		if (options[494] != 0)
		{
			printf("Error: option is too long.\n");
			continue;
		}
		// test cmd and opertaion;
		// printf("%s\n%s\n", cmd, options);
		if (!strcmp(cmd, "help"))
		{
			printf("All commands:\n");
			printf("Commands about file operation:\n");
			printf("cd [DIRECTORY]				Change the shell working directory.\n");
			printf("ls					List  information  about the FILEs (the current directory by default).\n");
			printf("mkdir [DIRECTORY]			Create the DIRECTORY(ies), if they do not already exist.\n");
			printf("mv [OLD NAME] [NEW NAME]		Rename the file or directory name from OLD NAME to NEW NAME, if it exists.\n");
			printf("pwd					print the full filename of the current working directory.\n");
			printf("rm [FILE]				Remove the FILE, if it exists.\n");
			printf("rmdir [DIRECTORY]			Remove the DIRECTORY and files in it, if they exist.\n");
			printf("touch [FILE] [SIZE]			A FILE argument that does not exist is created with SIZE.\n");
			printf("Commands about process operation:\n");
			printf("create [process]			Create a process named by user with random events.\n");
			printf("kill [pid]				Send the processes identified by PID a signal to terminate it\n");
			printf("ps					Display information about the active processes.\n");
			printf("run [FILE]				Run the executable [FILE]\n");
			printf("shutdown				Power-off the operating system.\n");
			printf("Commands about memory operation:\n");
			printf("free					Display amount of free and used memory in the system.\n");
		}
		else if (!strcmp(cmd, "cd"))
		{
			changeDir(options);
		}
		else if (!strcmp(cmd, "ls"))
		{
			showDir();
		}
		else if (!strcmp(cmd, "mkdir"))
		{
			creatDir(options);
		}
		else if (!strcmp(cmd, "mv"))
		{
			char oldName[30] = {0}, newName[464] = {0};
			int i;
			for (i = 0; i < strlen(options) && options[i] != 0 && i < 30; i++)
			{
				if (options[i] == ' ')
					break;
				oldName[i] = options[i];
			}
			if (options[i++] != ' ')
			{
				printf("Error: touch needs two parameters.\n");
				continue;
			}
			for (int j = i; j < strlen(options) && options[j] != 0 && j < 30; j++)
			{
				if (options[i] == ' ')
					break;
				newName[j - i] = options[j];
			}
			changeName(oldName, newName);
		}
		else if (!strcmp(cmd, "pwd"))
		{
			char* pwd = getPath();
			puts(pwd);
		}
		else if (!strcmp(cmd, "rm"))
		{
			deleteFile(options);
		}
		else if (!strcmp(cmd, "rmdir"))
		{
			deleteDir(options);
		}
		else if (!strcmp(cmd, "touch"))
		{
			char fileName[30] = {0}, fileSize[464] = {0};
			int i;
			for (i = 0; i < strlen(options) && options[i] != 0 && i < 30; i++)
			{
				if (options[i] == ' ')
					break;
				fileName[i] = options[i];
			}
			if (options[i++] != ' ')
			{
				printf("Error: touch needs two parameters.\n");
				continue;
			}
			int size = string2Int(options, i);
			creatFile(fileName, size);
		}
		else if (!strcmp(cmd, "create"))
		{
			CreateMyProcess(options, -1);
		}
		else if (!strcmp(cmd, "kill"))
		{
			int pid = string2Int(options, i);
			if (pid == -1)
				continue;
			if (pid > 15)
			{
				printf("Error: ID is invalid.\n");
				continue;
			}
			KillProcess(pid);
		}
		else if (!strcmp(cmd, "ps"))
		{
			showAllProcess();
		}
		else if (!strcmp(cmd, "run"))
		{
			printf("Run the executable file: %s\n", options);
			if (CreateMyDiyProcess(options, -1, options))
			{
				printf("Create process successfully.\n");
			}
			else
			{
				printf("Fail to create process.\n");
			}
		}
		else if (!strcmp(cmd, "shutdown"))
		{
			running = 0;
		}
		else if (!strcmp(cmd, "free"))
		{
			// pass
		}
		else
		{
			printf("%s: command not found. You can use help to see all the commands.\n", cmd);
		}
	}

	return;
}

int string2Int(char string[], int start)
{
	int res = 0, legal = 1;
	for (int i = start; string[i]; i++)
	{
		if (string[i] < '0' && string[i] > '9')
		{
			printf("Error: please enter the integer.\n");
			legal = 0;
			break;
		}
		res = res * 10 + string[i] - '0';
	}
	return legal ? res : -1;
}