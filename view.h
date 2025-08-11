#ifndef VIEW_H
#define VIEW_H

#include<stdio.h>
#include "type.h"

#define FRAME_HEADER_SKIP 3L
#define FRAME_ID_SIZE     4
#define FRAME_ID_BUF_SIZE 5

typedef struct _View
{
    /* Source MP3 file */
    FILE *fptr_mp3;
    char frame_ID[FRAME_ID_BUF_SIZE];

    /* Title Info */
    char *title_tag;
    uint title_tag_size;

    /* Artist Info */
    char *artist_tag;
    uint artist_tag_size;

    /* Album Info */
    char *album_tag;
    uint album_tag_size;

    /* Year of Release Info */
    char *year_tag;
    uint year_size;

    /* Content Info */
    char *content_tag;
    uint content_tag_size;

    /* Comments Info */
    char *comments_tag;
    uint comments_tag_size;

}View;

/* Viewing MP3 details Function Prototype */

/* Check Operation Type */
Operation check_operation_type(char *argv[]);

/* Perform Validation */
Status read_and_validate_mp3_file(char **argv, View *viewInfo);

/* View Tag Details Operation */
Status view_tag(char **argv, View *viewInfo);

/* Display the various frame contents of MP3 file */
Status get_and_display(const char *str_frame, const char *str_id , uint *tag_size, char **tag, FILE *fp);

/* Function to convert Big endian to Little endian */
uint swap_endian(uint val);
#endif