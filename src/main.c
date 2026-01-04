#include <stdio.h>
#include <stdbool.h>
#include <getopt.h> // command line arguments
#include <stdlib.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[]) 
{
    printf("Usage: %s -n -f <database file>\n", argv[0]);
    printf("\t -n - create new database file\n");
    printf("\t -f - (required) path to database file\n");

    return;
}

int main(int argc, char *argv[]) 
{ 
    bool newfile = false;
    char* filepath = NULL;
	int c;

    int dbfd = -1;
    struct dbheader_t *dbhdr = NULL;
    struct employee_t *employees = NULL;

    // first flag -n is the boolean flag, and -f is the filename flag for filename string
    // a colon after the flag f means it takes a string
    while ((c = getopt(argc, argv, "nf:")) != -1) 
    {
        switch(c)
        {
            case 'n':
                newfile = true;
                break;

            case 'f':
                filepath = optarg;  // optarg gets the pointer for the argument that getopt tracked, in this case the pointer to the myfile.db file
                break;

            case '?':
                printf("Unknown option -%c\n", c);
                break;

            default:
                return -1;  // if for whatever case the ? case doesn't pick up the unknown/unsupported tack, return -1
        }
    }

    if (filepath == NULL)
    {
        printf("A filepath is a required argument.\n");
        print_usage(argv);

        return 0;
    }

    if (newfile)
    {
        dbfd = create_db_file(filepath);
        if (dbfd == STATUS_ERROR)
        {
            printf("Unable to create database file\n");
            return -1;
        }

        if (create_db_header(dbfd, &dbhdr) == STATUS_ERROR)
        {
            printf("Failed to create database header!\n");
            return -1;
        }
    }
    else 
    {
        dbfd = open_db_file(filepath);
        if (dbfd == STATUS_ERROR)
        {
            printf("Unable to open database file\n");
            return -1;
        }
        if (validate_db_header(dbfd, &dbhdr) == STATUS_ERROR)
        {
            printf("Failed to validate database header\n");
            return STATUS_ERROR;
        }
    }

    output_file(dbfd, dbhdr, employees);
}
