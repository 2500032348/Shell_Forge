#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#define SIZE 4096

int main()
{
    int fd = open("page.txt", O_RDONLY);

    if (fd < 0)
    {
        perror("open");
        return 1;
    }

    // Map the file into memory
    char *p = mmap(NULL, SIZE, PROT_READ,
                   MAP_PRIVATE, fd, 0);

    if (p == MAP_FAILED)
    {
        perror("mmap");
        close(fd);
        return 1;
    }

    printf("Accessing page...\n");

    // Access the mapped page
    printf("Data = %c\n", p[0]);

    munmap(p, SIZE);
    close(fd);

    return 0;
}
