#ifndef EDIT_H
#define EDIT_H

#include"type.h"

#define MAX_FRAME_SIZE 1024 * 1024

typedef struct _Edit
{
    FILE *fptr_mp3;
    FILE *fptr_temp;

    char frame_id[5];
    char frame_id_value[100];
    uint frame_id_size;

}Edit;

/* Editing MP3 details Function Prototype */

/* Perform Validation of the Arguments passed and store in the Structure */
Status read_and_validate_mp3_file_args(char *argv[], Edit *editInfo);

/* Perform the Editing Tag details Operation */
Status edit_tag(char *argv[], Edit *editInfo);

/* Copy the Frame Contents from the Source to Temporary MP3 file */
Status copy_data(Edit *editInfo, int version);

/* Copy the contents after the Frames have been copied from Source to Temporary MP3 file */
Status copy_remaining_data(Edit *editInfo);

/* Helper for big-endian frame size */
uint get_frame_size(char *header, int version);
void write_big_endian_uint(FILE *fp, uint value);

#endif