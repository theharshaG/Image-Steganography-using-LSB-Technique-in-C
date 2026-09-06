#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"

OperationType check_operation_type(char *argv[]);

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: ./a.out -e <source_file> <output_file>\n");
        return 0;
    }

    OperationType res = check_operation_type(argv);

    if (res == e_encode)
    {
        EncodeInfo encInfo;

        if (read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            printf("you opened encode\n");
            do_encoding(&encInfo);
        }
        else
        {
            printf("Invalid arguments for Encode\n");
            return 0;
        }
    }
    else if (res == e_decode)
    {
        // do_decode();
    }
    else
    {
        printf("Unsupported operation\n");
    }

    return 0;
}

OperationType check_operation_type(char *argv[])
{
    if (strcmp(argv[1], "-e") == 0)
    {
        return e_encode;
    }
    else if (strcmp(argv[1], "-d") == 0)
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}