#include "OSShell.h"

void shell();

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
			printf("touch [FILE]								Update the access and modification times of each FILE to the current time. A FILE argument that does not exist is created empty.\n");
			printf("ln                                          Create a new link for file1, named file2.\n");
			printf("rmln                                        Delete link for given file.\n");
			printf("mv                                          Modify file name or directory name.\n");
			printf("rm                                          Delete file.\n");
			printf("Commands about process operation:\n");
			printf("create [process]							Create a process named by user with random events.\n");
			printf("kill [pid]									Send the processes identified by PID a signal to terminate it\n");
			printf("ps											Display information about the active processes.\n");
			printf("run [FILE]									Run the executable [FILE]\n");
			printf("shutdown									Power-off the operating system.\n");
			printf("Commands about memory operation:\n");
			printf("free										Display amount of free and used memory in the system.\n");
		}
		else if (!strcmp(cmd, "cd"))
		{
			chdir(options);
		}
		else if (!strcmp(cmd, "ls"))
		{
			listfile();
		}
		else if (!strcmp(cmd, "mkdir"))
		{
			mkdir(options);
		}
		else if (!strcmp(cmd, "pwd"))
		{
			printf("%s\n",printwd());
		}
		else if (!strcmp(cmd, "touch"))
		{
			touch(options,0);
		}
		else if (!strcmp(cmd, "ln"))
		{
			char* file1;
			char* file2;
			int flg = 0;
			for (int i = 0; options[i]; i++)
			{
				if(flg == 0 && options[i] != " ")
				{
					file1 += options[i];
				}
				else
				{
					flg = 1;
					file2 += options[i];
				}
			}
			link(file1,file2);
		}
		else if (!strcmp(cmd, "rmln"))
		{
			unlink(options);
		}
		else if (!strcmp(cmd, "mv"))
		{
			char* oldfile;
			char* newfile;
			int flg = 0;
			for (int i = 0; options[i]; i++)
			{
				if(flg == 0 && options[i] != " ")
				{
					oldfile += options[i];
				}
				else
				{
					flg = 1;
					newfile += options[i];
				}
			}
			changeName(oldfile,newfile);
		}
		else if (!strcmp(cmd, "rm"))
		{
			deleteFile(options);
		}
		else if (!strcmp(cmd, "create"))
		{
			CreateMyProcess(options, -1);
		}
		else if (!strcmp(cmd, "kill"))
		{
			int pid = 0, flg = 1;
			for (int i = 0; options[i]; i++)
			{
				if (options[i] < '0' && options[i] > '9')
				{
					printf("Error: please enter the ID of process\n");
					flg = 0;
					break;
				}
				pid = pid * 10 + options[i] - '0';
			}
			if (!flg)
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