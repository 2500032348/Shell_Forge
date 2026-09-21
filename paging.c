#include <stdio.h>

int main()
{
    // Page size = 1024 bytes (1 KB)
    int page_size = 1024;

    // Page Table: Page Number -> Frame Number
    int page_table[] = {5, 2, 7, 1, 0};

    int logical_address;
    int page_number;
    int offset;
    int frame_number;
    int physical_address;

    printf("====================================\n");
    printf("       PAGING DEMONSTRATION\n");
    printf("====================================\n");

    printf("\nPage Table:\n");
    printf("Page 0 -> Frame 5\n");
    printf("Page 1 -> Frame 2\n");
    printf("Page 2 -> Frame 7\n");
    printf("Page 3 -> Frame 1\n");
    printf("Page 4 -> Frame 0\n");

    // User enters logical address
    printf("\nEnter Logical Address: ");
    scanf("%d", &logical_address);

    /*
       Step 1:
       Find Page Number

       Page Number = Logical Address / Page Size
    */
    page_number = logical_address / page_size;

    /*
       Step 2:
       Find Offset

       Offset = Logical Address % Page Size
    */
    offset = logical_address % page_size;

    // Check if page exists in page table
    if (page_number >= 5)
    {
        printf("\nPage Fault!\n");
        printf("Page %d is not present in Page Table.\n", page_number);
        return 0;
    }

    /*
       Step 3:
       Get Frame Number from Page Table
    */
    frame_number = page_table[page_number];

    /*
       Step 4:
       Calculate Physical Address

       Physical Address =
       (Frame Number × Page Size) + Offset
    */
    physical_address = (frame_number * page_size) + offset;

    printf("\n----- Address Translation -----\n");
    printf("Logical Address  : %d\n", logical_address);
    printf("Page Number      : %d\n", page_number);
    printf("Offset           : %d\n", offset);
    printf("Frame Number     : %d\n", frame_number);
    printf("Physical Address : %d\n", physical_address);

    return 0;
}
