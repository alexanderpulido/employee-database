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
    struct employee_t* employee = calloc(1, sizeof(struct employee_t));
    if (employee == NULL)
    {
        printf("Malloc failed to create a db header!\n");
        return STATUS_ERROR;
    }
    if (read(fd, employee, sizeof(struct employee_t)) != sizeof(struct employee_t))
    {
        perror("read");
        free(employee);
        return STATUS_ERROR;
    }

    *employeesOut = employee;

    return STATUS_SUCCESS;
}

int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) 
{
    if (fd < 0)
    {
        printf("Got bad fd from the user\n");
        return STATUS_ERROR;
    }

    // host endian to network endian (with long data type)
    dbhdr->magic = htonl(dbhdr->magic);
    dbhdr->filesize = htonl(dbhdr->filesize);

    // host endian to network endian (with short data type)
    dbhdr->count = htons(dbhdr->count);
    dbhdr->version = htons(dbhdr->version);

    // bring the "cursor" back to the front of the file so it doesn't write to the end of the file
    lseek(fd, 0, SEEK_SET);

    write(fd, dbhdr, sizeof(struct dbheader_t));

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
    if (header == NULL)
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

int create_db_header(int fd, struct dbheader_t **headerOut) 
{
	struct dbheader_t* header = calloc(1, sizeof(struct dbheader_t));
    if (header == NULL)
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


