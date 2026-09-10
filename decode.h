#ifndef DECODE_H
#define DECODE_H

#include "types.h"

/* Structure to store information required for decoding */

typedef struct _DecodeInfo
{
    /* Stego Image info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

    /* Secret File info */
    char *secret_fname;
    FILE *fptr_secret;

    long size_secret_file;

    /* Magic String */
    char magic[10];
    int magic_string_size;

    int extn_size;
    char extn_secret_file[10];

} DecodeInfo;


/* Check operation type */
OperationType check_operation_type(char *argv[]);
//check validation
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);
Status do_decoding(DecodeInfo *decInfo);
Status open_decode_file(DecodeInfo *decInfo);
Status skip_bmp_header(DecodeInfo *decInfo);
Status decode_magic_string_size(DecodeInfo *decInfo);
int decode_int_from_image(char *ms_size);
Status decode_magic_string(DecodeInfo *decInfo);
char decode_char_from_image(char *arr);
Status decode_secret_file_extn_size(DecodeInfo *decInfo);
Status decode_secret_file_extn(DecodeInfo *decInfo);
Status decode_secret_file_size(DecodeInfo *decInfo);
Status decode_secret_file_data(DecodeInfo *decInfo);


#endif

