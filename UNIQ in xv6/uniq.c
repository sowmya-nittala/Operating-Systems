#include "types.h"
#include "user.h"

int main(int argc, char *argv[])
{

    int fg = 0;            //file descriptor
    char buf[1024];        //read into buf
    char prev[1024] = " "; //store prev string
    char curr[1024];       //store curr string
    int n;                  //number of characters read
    int itr;               //iterator for loops
    int k = 0;             //iterator for curr array when copying from buff
    int i, flag = 1;
    int cflag = 0;
    int dflag = 0;
    int iflag = 0;
    int countlines = 1; //counting lines for -c
    int lastlinecount = 1;

    //set flags and filename depending on the arguments
    for (itr = 1; itr < argc; itr++)
    {

        if (strcmp(argv[itr], "-c") == 0)
        {
            cflag = 1;
        }

        else if (strcmp(argv[itr], "-d") == 0)
        {
            dflag = 1;
        }

        else if (strcmp(argv[itr], "-i") == 0)
        {
            iflag = 1;
        }

        else
        {
            //Take input from terminal
            if (argc == 1)
            {
                fg = 0;
            }

            else
            {
                fg = open(argv[itr], 0);
                if (fg < 0)
                {
                    printf(1, "error! file is not valid");
                    return -1;
                }
            }
        }
    }
    // -c and -d cannot be called at the same time
    if (cflag && dflag)
    {
        printf(1, "Invalid input, please try either with -c or -d \n");
        exit();
    }
    //read (n) bytes from the file
    while ((n = read(fg, buf, sizeof buf)) > 0)
    {

        for (itr = 0; itr < n; itr++)
        {
            //check each character and:
            //1. Add to current as long as it's not '\n'
            //2. if it is '\n' the sentence is complete so compare with prev and print
            if (fg == 0 && itr == n - 1 && buf[itr] != '\n' && dflag)
            {
                lastlinecount = countlines;
                break;
            }
            //it's not a line yet
            if (buf[itr] != '\n')
            {
                curr[k] = buf[itr];
                k++;
            }

            else if (buf[itr] == '\n')
            {
                curr[k]='\0';
                if (strcmp(prev, curr) != 0)
                {
                    if (iflag)
                    {
                        for (i = 0; i <= strlen(curr); i++)
                        {

                            if (prev[i] - curr[i] == 0 ||
                                (((prev[i] > 64 && prev[i] < 91) || (prev[i] > 96 && prev[i] < 123)) && ((curr[i] > 64 && curr[i] < 91) || (curr[i] > 96 && curr[i] < 123)) && (prev[i] - curr[i] == 32 || prev[i] - curr[i] == -32)))
                            {
                                flag = 0;
                            }
                            else
                            {
                                flag = 1;
                                break;
                            }
                        }
                        if (flag == 1)
                        {
                            if (cflag)
                            {
                                if ((strcmp(prev, " ") != 0))
                                {
                                    printf(1, "%d %s\n", countlines, prev);
                                }
                            }
                            else if (dflag)
                            {
                                if ((strcmp(prev, " ") != 0))
                                {
                                    if (countlines > 1)
                                    {
                                        printf(1, "%s\n", prev);
                                    }
                                }
                            }
                            else
                            {
                                printf(1, "%s\n", curr);
                            }
                            lastlinecount = countlines;
                            countlines = 1;
                            strcpy(prev, curr);
                        }
                        else
                        {
                            countlines++;
                            lastlinecount = countlines;
                        }
                    }
                    else
                    {
                        if (cflag)
                        {
                            if ((strcmp(prev, " ") != 0))
                            {
                                printf(1, "%d %s\n", countlines, prev);
                            }
                        }
                        else if (dflag)
                        {
                            if ((strcmp(prev, " ") != 0))
                            {
                                if (countlines > 1)
                                {
                                    printf(1, "%s\n", prev);
                                }
                            }
                        }
                        else
                        {
                            printf(1, "%s\n", curr);
                        }
                        lastlinecount = countlines;
                        countlines = 1;
                        strcpy(prev, curr);
                    }
                }
                 //They are the same so don't print
                else
                {
                    countlines++;
                    lastlinecount = countlines;
                }
                k = 0;
            }
        }
    }
    //for last line

    if (cflag)
    {
        printf(1, "%d %s\n", lastlinecount, prev);
    }
    else if (dflag)
    {
        if (lastlinecount > 1)
        {
            printf(1, "%s\n", prev);
        }
    }
    close(fg);
    exit();
}