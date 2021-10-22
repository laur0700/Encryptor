#include <ctype.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

void shuffle(char *array, int perm[][50], int n, int cuv_curent)
{
	int i, j;
	char aux1;

	int index = 1, aux2;

	for (i = 0; i < n; i++)
	{
		perm[cuv_curent][i] = index;
		index++;
	}

	for (i = n - 1; i > 0; i--)
	{
		j = random() % (i + 1);
		//shuffle cuvant
		aux1 = array[j];
		array[j] = array[i];
		array[i] = aux1;
		//shuffle indici
		aux2 = perm[cuv_curent][j];
		perm[cuv_curent][j] = perm[cuv_curent][i];
		perm[cuv_curent][i] = aux2;
	}
}

void decrypt(char *array, int perm[][50], int n, int cuv_curent)
{
	int i, j, aux2;
	char aux1;

	for (i = 0; i < n - 1; i++)
	{
		for (j = i + 1; j < n; j++)
		{
			if (perm[cuv_curent][i] > perm[cuv_curent][j])
			{
				//sortare indici
				aux2 = perm[cuv_curent][i];
				perm[cuv_curent][i] = perm[cuv_curent][j];
				perm[cuv_curent][j] = aux2;
				//aranjare cuvant
				aux1 = array[i];
				array[i] = array[j];
				array[j] = aux1;
			}
		}
	}
}

int main(int argc, char *argv[])
{
	if (argc == 2) //criptarea
	{
		int fd = open(argv[1], O_RDONLY);
		int nr_cuv = 0;
		char *token;

		srandom(time(NULL));
		long r = random();

		struct stat sb;
		stat(argv[1], &sb);
		long int filesize = sb.st_size;

		char *s = (char *)malloc(filesize * sizeof(char));
		read(fd, s, filesize);
		//s[strlen(s) - 1] = '\0';

		for (int i = 0; i < filesize; i++)
		{
			if (s[i] == ' ' || i == filesize - 1 || s[i] == '\n')
			{
				nr_cuv++;
			}
		}

		char *cuvinte[nr_cuv];
		int i, v[nr_cuv][50];

		i = 0;
		token = strtok(s, " \n");

		while (token != NULL)
		{
			cuvinte[i] = token;
			i++;
			token = strtok(NULL, " \n");
		}

		i = 0;
		int fd_crypt = open("./crypt.txt", O_CREAT | O_RDWR | O_TRUNC | O_APPEND, 00700);
		int fd_keys = open("./keys.txt", O_CREAT | O_RDWR | O_TRUNC | O_APPEND, 00700);
		char *space = " ";
		char *new_line = "\n";

		while (i < nr_cuv)
		{
			pid_t pid = fork();

			if (pid < 0)
			{
				return errno;
			}
			else if (pid == 0)
			{
				exit(0);
			}
			else
			{
				shuffle(cuvinte[i], v, strlen(cuvinte[i]), i);
				write(fd_crypt, cuvinte[i], strlen(cuvinte[i]));

				for (int j = 0; j < strlen(cuvinte[i]); j++)
				{
					char index[10];
					sprintf(index, "%d", v[i][j]);
					write(fd_keys, index, strlen(index));
				}
				
				if (i < nr_cuv - 1)
				{
					write(fd_crypt, space, strlen(space));
					write(fd_keys, space, strlen(space));
				}

				i++;
			}
		}

		close(fd);
		close(fd_crypt);
		close(fd_keys);
		free(s);
	}
	else
	{
		if (argc == 3) //decriptarea
		{

			int fd_crypt = open(argv[1], O_RDONLY);
			int fd_keys = open(argv[2], O_RDONLY);
			int nr_cuv = 0;
			char *token;

			srandom(time(NULL));
			long r = random();

			struct stat sb;
			stat(argv[1], &sb);
			long int filesize = sb.st_size;

			char *s = (char *)malloc(filesize * sizeof(char));
			read(fd_crypt, s, filesize);

			for (int i = 0; i < filesize; i++)
			{
				if (s[i] == ' ' || i == filesize - 1)
				{
					nr_cuv++;
				}
			}

			char *cuvinte[nr_cuv];
			int i, v[nr_cuv][50];

			i = 0;
			token = strtok(s, " \n");

			while (token != NULL)
			{
				cuvinte[i] = token;
				i++;
				token = strtok(NULL, " \n");
			}

			char *p = (char *)malloc(filesize * sizeof(char));
			read(fd_keys, p, filesize);

			i = 0;
			token = strtok(p, " \n");

			while (token != NULL)
			{
				int n = strlen(cuvinte[i]);
				for (int j = 0; j < n; j++)
				{
					v[i][j] = token[j] - '0';
				}
				i++;
				token = strtok(NULL, " \n");
			}

			int fd_decrypt = open("./decrypt.txt", O_CREAT | O_RDWR | O_TRUNC | O_APPEND, 00700);
			char *space = " ";

			for (i = 0; i < nr_cuv; i++)
			{
				decrypt(cuvinte[i], v, strlen(cuvinte[i]), i);
				write(fd_decrypt, cuvinte[i] + (strlen(cuvinte[i]) - v[i][strlen(cuvinte[i]) - 1]), v[i][strlen(cuvinte[i]) - 1]);

				if (i < nr_cuv - 1)
				{
					write(fd_decrypt, space, strlen(space));
				}
			}

			close(fd_crypt);
			close(fd_keys);
			close(fd_decrypt);
			free(s);
			free(p);
		}
	}

	return 0;
}
