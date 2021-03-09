#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <omp.h>
#include <sys/wait.h>

int arr[9][9];

/*
 * 8 2 7 1 5 4 3 9 6
 * 9 6 5 3 2 7 1 4 8
 * 3 4 1 6 8 9 7 5 2
 * 5 9 3 4 6 8 2 7 1
 * 4 7 2 5 1 3 6 8 9
 * 6 1 8 9 7 2 4 3 5
 * 7 8 6 2 3 5 9 1 4
 * 1 5 4 7 9 6 8 2 3
 * 2 3 9 8 4 1 5 6 7
 * */

int checkDuplicity(int arr[]);

void *checkRows();

void *checkColumns();

void *checkRegions();

int main(int argc, char *argv[])
{

    omp_set_num_threads(1);
    printf("\n");
    // Open the file
    printf("Opening file \"%s\"...\n", argv[1]);
    int file = open(argv[1], O_RDONLY);
    if (file < 0)
    {
        printf("ERROR: Could not open file.\n");
        return -1;
    }
    else
    {
        printf("Opened file successfully!\n\n");
    }

    // Save the characters in a temporary variable
    char line[81];
    int tempArr[81];
    read(file, &line, 81);
    for (int i = 0; i < 81; i++)
    {
        switch (line[i])
        {
        case '1':
            tempArr[i] = 1;
            break;
        case '2':
            tempArr[i] = 2;
            break;
        case '3':
            tempArr[i] = 3;
            break;
        case '4':
            tempArr[i] = 4;
            break;
        case '5':
            tempArr[i] = 5;
            break;
        case '6':
            tempArr[i] = 6;
            break;
        case '7':
            tempArr[i] = 7;
            break;
        case '8':
            tempArr[i] = 8;
            break;
        case '9':
            tempArr[i] = 9;
            break;
        default:
            return -1;
        }
    }
    for (int i = 0, counter = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; ++j, counter++)
        {
            arr[i][j] = tempArr[counter];
            //            printf("Arr[%d][%d] = %d, tempArr[%d] = %d\n", i, j, arr[i][j], counter, tempArr[counter]);
        }
    }

    printf("-> Found the following sudoku solution: \n\n");
    for (int i = 0; i < 9; ++i)
    {
        printf("-> ");

        for (int j = 0; j < 9; ++j)
        {
            printf("%d ", arr[i][j]);
        }
        printf("<-\n");
    }

    // Here starts the children operations after the sudoku has been read
    pid_t child = fork();

    if (child == 0)
    {
        pid_t procIntID = getppid();
        char proc[10];
        sprintf(proc, "%ld", (long int)procIntID);

        char command[100] = "ps -p ";
        strcat(command, proc);
        strcat(command, " -lLf");
        printf("Executing command: %s\n", command);
        system(command);
    }
    else
    {
        int *validColumns;
        int *validRows;
        int *validRegion;

        pthread_t thread_idcol, thread_idrow, thread_idregion;

        printf("======______THREAD INFORMATION______======\n");
        printf("-> Main thread being executed in thread %lu", syscall(SYS_gettid));

        if (pthread_create(&thread_idcol, NULL, &checkColumns, NULL) != 0)
        {
            printf("Columns coulnd't be created.");
        }
        if (pthread_create(&thread_idrow, NULL, &checkRows, NULL) != 0)
        {
            printf("Rows coulnd't be created.");
        }
        if (pthread_create(&thread_idregion, NULL, &checkRegions, NULL) != 0)
        {
            printf("Regions coulnd't be created.");
        }
        if (pthread_join(thread_idcol, (void **)&validColumns) != 0)
        {
            printf("Thread waiting has failed: Columns\n");
        }
        if (pthread_join(thread_idrow, (void **)&validRows) != 0)
        {
            printf("Thread waiting has failed: Rows\n");
        }
        if (pthread_join(thread_idregion, (void **)&validRegion) != 0)
        {
            printf("Thread waiting has failed: Regions\n");
        }
        wait(NULL);

        printf("\n\n======______SUDOKU INFORMATION______======\n");
        if (*validColumns == 1 && *validRows == 1 && *validRegion == 1)
        {
            printf("->Solution for sudoku is valid!\n");
        }
        else
        {
            printf("->Solution for sudoku is invalid:");

            if (*validColumns == 0)
            {
                printf("\n-->The duplicity solver has found an error in Columns.");
            }

            if (*validRows == 0)
            {
                printf("\n-->The duplicity solver has found an error in Rows.");
            }
            if (*validRegion == 0)
            {
                printf("\n-->The duplicity solver has found an error in Regions.");
            }
            printf("\n");
        }

        pid_t sChild = fork();

        if (sChild == 0)
        {
            pid_t procIntID = getppid();
            char proc[10];
            sprintf(proc, "%ld", (long int)procIntID);

            char command[100] = "ps -p ";
            strcat(command, proc);
            strcat(command, " -lLf");
            printf("Executing command: %s\n", command);
            system(command);
        }
        else {
            wait(NULL);
            return 0;
        }
    }
}

void *checkRows()
{
    omp_set_nested(1);
    omp_set_num_threads(9);
    printf("\n-->Rows currently being checked by thread %lu", syscall(SYS_gettid));
    int *result = malloc(sizeof(int));
    *result = 1;

#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < 9; ++i)
    {
        if (checkDuplicity(arr[i]) == 0)
        {
            *result = 0;
        }
    }
    return (void *)result;
}

void *checkColumns()
{
    omp_set_nested(1);
    omp_set_num_threads(9);
    printf("\n-->Columns currently being checked by thread %lu", syscall(SYS_gettid));
    int *result = malloc(sizeof(int));
    *result = 1;
#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < 9; ++i)
    {
        int temp[9];
        for (int j = 0; j < 9; ++j)
        {
            temp[j] = arr[i][j];
        }
        if (checkDuplicity(temp) == 0)
        {
            *result = 0;
        }
    }
    return (void *)result;
}

void *checkRegions()
{
    omp_set_nested(1);
    omp_set_num_threads(9);
    printf("\n-->Regions currently being checked by thread %lu", syscall(SYS_gettid));
    int *result = malloc(sizeof(int));
    *result = 1;

    int temp[9];

    int x = 0, y = 0;

    do
    {
        int counter = 0;
        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                temp[counter++] = arr[i + 3 * x][j + 3 * y];
            }
        }
        if (x == 3)
        {
            x = 0;
            y = y + 1;
        }
        else
            x++;

        if (temp[0] != 0)
        {
            if (checkDuplicity(temp) == 0)
            {
                *result = 0;
            }
        }
    } while (y < 3);
    return (void *)result;
}

// O(n)
int checkDuplicity(int arr[9])
{
    omp_set_num_threads(9);
    int existent[9] = {0};
    int result = 1;
#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < 9; i++)
    {
        if (existent[arr[i] - 1] == 1)
        {
            result = 0;
        }
        else
        {
            existent[arr[i] - 1] = 1;
        }
    }
    return result;
}