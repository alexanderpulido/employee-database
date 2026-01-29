#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "parse.h"

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) 
{
    
    if (fd < 0)
    {
        printf("Got bad fd from the user\n");
        return STATUS_ERROR;
    }
    int count = dbhdr->count;
    struct employee_t* employee = calloc(count, sizeof(struct employee_t));
    if (employee == -1)
    {
        printf("Malloc failed!\n");
        return STATUS_ERROR;
    }
    
    read(fd, employee, count * sizeof(struct employee_t));
    
    int i = 0;
    
    for (; i < count; i++)
    {
        employee[i].hours = ntohl(employee[i].hours);
    }

    *employeesOut = employee;

    return STATUS_SUCCESS;
}

int add_employee(struct dbheader_t *dbhdr, struct employee_t **employees, char *addstring)
{
    if (NULL == dbhdr) return STATUS_ERROR;
    if (NULL == employees) return STATUS_ERROR;
    if (NULL == *employees) return STATUS_ERROR;
    if (NULL == addstring) return STATUS_ERROR;
    
    char* name = strtok(addstring, ",");    // strtok takes our addstring argument and first gets the name of the employee to the db and turns it into tokens, where "," is our delimiter (aka where it will seperate each)
    if (NULL == name) return STATUS_ERROR;

    char* addr = strtok(NULL, ",");         // here this goes into our string and gets the next element (in this case the street address)
    if (NULL == addr) return STATUS_ERROR;
    
    char* hours = strtok(NULL, ",");        // lastly this gets the hours from our addstring argument
    if (NULL == hours) return STATUS_ERROR;

    struct employee_t* e = *employees;
    e = realloc(e, sizeof(struct employee_t) * dbhdr->count+1);
    if (e == NULL)
    {
        return STATUS_ERROR;
    }

    dbhdr->count++;
    
    printf("%s %s %s\n", name, addr, hours);
    
    strncpy(e[dbhdr->count-1].name, name, sizeof(e[dbhdr->count-1].name) - 1);
    strncpy(e[dbhdr->count-1].address, addr, sizeof(e[dbhdr->count-1].address) - 1);

    *employees = e; // Overwrite the old pointer to the employees with the new pointer e

    e[dbhdr->count-1].hours = atoi(hours);  // atoi converts a string (here it's the hours) into an integer
    
    return STATUS_SUCCESS;
}

int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) 
{
    if (fd < 0)
    {
        printf("Got bad fd from the user\n");
        return STATUS_ERROR;
    }

    int realcount = dbhdr->count;

    // host endian to network endian (with long data type)
    dbhdr->magic = htonl(dbhdr->magic);
    dbhdr->filesize = htonl(sizeof(struct dbheader_t) + sizeof(struct employee_t) * realcount);

    // host endian to network endian (with short data type)
    dbhdr->count = htons(dbhdr->count);
    dbhdr->version = htons(dbhdr->version);

    // bring the "cursor" back to the front of the file so it doesn't write to the end of the file
    lseek(fd, 0, SEEK_SET);

    write(fd, dbhdr, sizeof(struct dbheader_t));

    for (int i = 0; i < realcount; i++)
    {
        employees[i].hours = htonl(employees[i].hours);
        write(fd, &employees[i], sizeof(struct employee_t));
    }

    return STATUS_SUCCESS;
}	

int validate_db_header(int fd, struct dbheader_t **headerOut) 
{
    if (fd < 0)
    {
        printf("Got bad fd from the user\n");
        return STATUS_ERROR;
    }
    struct dbheader_t* header = calloc(1, sizeof(struct dbheader_t));
    if (header == -1)
    {
        printf("Malloc failed to create a db header!\n");
        return STATUS_ERROR;
    }
    if (read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t))
    {
        perror("read");
        free(header);
        return STATUS_ERROR;
    }

    // network endian to host endian (with short data type)
    header->version = ntohs(header->version);   
    header->count = ntohs(header->count);

    // network endian to host endian (with long data type)
    header->magic = ntohl(header->magic);
    header->filesize = ntohl(header->filesize);

    if (header->magic != HEADER_MAGIC)
    {
        printf("Improper header magic.\n");
        free(header);
        return STATUS_ERROR;
    }

    if (header->version != 1)
    {
        printf("Improper header version.\n");
        free(header);
        return STATUS_ERROR;
    }

    struct stat dbstat = {0};
    fstat(fd, &dbstat);
    if (header->filesize != dbstat.st_size)
    {
        printf("Error: Corrupted database\n");
        free(header);
        return STATUS_ERROR;
    }

    *headerOut = header;

    return STATUS_SUCCESS;
}

int create_db_header(struct dbheader_t **headerOut) 
{
	struct dbheader_t* header = calloc(1, sizeof(struct dbheader_t));
    if (header == -1)
    {
        printf("Malloc failed to allocate header!\n");
        return STATUS_ERROR;
    }
    header->version = 0x1;
    header->count = 0;
    header->magic = HEADER_MAGIC;
    header->filesize = sizeof(struct dbheader_t);

    *headerOut = header;

    return STATUS_SUCCESS;
}


