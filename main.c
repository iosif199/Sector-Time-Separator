/*
 *                  +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
 *                  |             IOSIF SAAD            |
 *                  |             2020-07-11	        |
 *	                +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
 *	                |		 Sector Time Separator	    |
 *	                +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef unsigned int u_int;

typedef enum{
    false, true
}bool;

typedef enum{
    NEWLINE, NEW_SECTOR, SONG, OTHER
}FILE_ACTION;

typedef struct{
    u_int hours, minutes, seconds;
}Time;

FILE *FileOpenner(const char *fname, const char *method);

short int LineType(const char *line);
void AddSong(FILE *file, Time *offset, char *red_line, bool *reset);
void FixSongTime(Time *song_t, Time offset_t);
void AddSongToOutput(FILE *output, Time *s_times, char *contents);

int main(void)
{
    bool reset_sector;
    char f_line[256], *scanned;
    FILE *input, *output;
    Time offset;

    //Opens the input and output file
    input = FileOpenner("tracklist.txt", "r");
    output = FileOpenner("tracklist_separated.txt", "w");

    for(;;)
    {
        scanned = fgets(f_line, 256, input);

        if(scanned == NULL) break; //Checks to see if we reached the EOF

        switch(LineType(f_line))
        {
            case NEWLINE:
                fputc('\n', output);
                break;
            case OTHER: /*If it finds anything else other than NEWLINE, NEW_SECTOR or SONG,
                *          it simply adds it to the output. Something like default */
                fputs(f_line, output);
                break;
            case NEW_SECTOR:
                reset_sector = true;
                fputs(f_line, output);
                break;
            case SONG:
                AddSong(output, &offset, f_line, &reset_sector);
                break;
            default: break;
        }
    }

    //Closes the files
    fclose(input);
    fclose(output);

    printf("Operation successfully finished!\n\n");
    system("pause");
    return 0;
}

FILE *FileOpenner(const char *fname, const char *method)
{
/* Function which opens and returns a FILE pointer to the
 *  requested file with name fname, and the open method method.
 *  It also checks to see if the file did not open. If that's
 *  the case, it terminates the program
*/
    FILE *io;

    io = fopen(fname, method);
    if(!io)
    {
        printf("Cannot open file [%s]\nExiting...\n\n", fname);

        system("pause");
        exit(EXIT_FAILURE);
    }

    return io;
}

short int LineType(const char *line)
{
/* Function which identifies which type of line it is:
 *
 * SONG, if a song timestamps are found (--h--m--s)
 * NEW_SECTOR, if the word "SECTOR" is found at the start
 * NELINE, if only a newline is red
 * OTHER, in any other case
*/
    short int is_line;

    is_line = (toupper(*(line + 2)) == 'H') &&
              (toupper(*(line + 5)) == 'M') &&
              (toupper(*(line + 8)) == 'S');
    //_

    if(is_line) return SONG;
    else if(strncmp(line, "SECTOR", 6) == 0) return NEW_SECTOR;
    else if(strcmp(line, "\n") == 0) return NEWLINE;
    else return OTHER;
}

void AddSong(FILE *out, Time *offset, char *red_line, bool *reset)
{
/* Procedure which adds the red song from the input, with the timestamps
 *  corrected. *reset is a bool that tels the procedure if the song that's
 *  being added is the first song of the sector. In that case, it sets the
 *  offset with the song's timestamps and then adds the song
*/
    short int nscan; //How many parameters fscanf red
    Time song_time;
    FILE *tmp;

    //A temporary file is created to put the contents of the file,
    // to make our life easier to extract the timestamps
    tmp = tmpfile();
    fputs(red_line, tmp);
    rewind(tmp);

    nscan = fscanf(tmp, "%uh%um%us %256[^\n]\n", &song_time.hours, &song_time.minutes, &song_time.seconds, red_line);

    if(nscan != 4) //Pretty much this will never happen, but just in case...
    {
        printf("Invalid song line!\nSkipping this line\n");
        return;
    }

    if(*reset) //First song in the sector
    {
        offset->hours = song_time.hours;
        offset->minutes = song_time.minutes;
        offset->seconds = song_time.seconds;
        *reset = false;

        //The fist song has a complete 0 timestamp (obviously =P)
        fprintf(out, "00h00m00s %s\n", red_line);
    }
    else
    {
        FixSongTime(&song_time, *offset);
        AddSongToOutput(out, &song_time, red_line);
    }

    fclose(tmp);
}

void FixSongTime(Time *song, Time offset)
{
/* Procedure which fixes the timestamps of the given song,
 *  according to the given offset. Offset is passed by value,
 *  because changes will be made to it that are not wanted to
 *  be taken into consideration at the next song.
 *
 * The correction is made by subtracting the timestamp of the offset
 *  from the song's timestamp. When there aren't enough seconds or
 *  minutes to be subtracted, a minute or hour is added accordingly,
 *  and then the subtraction takes place with 60 seconds/minutes added
 *  (and of course it's with +60 because we are dealing with time)
*/

    //Checks to see if correct values have been given
    if(
       (offset.hours > song->hours) ||
       (offset.minutes > 59) || (song->minutes > 59) ||
       (offset.seconds > 59) || (song->seconds > 59)
      )
    {
        printf("Invalid offset or song time format!\n");
        return;
    }

    //Seconds
    if(offset.seconds > song->seconds)
    {
        song->seconds = (song->seconds + 60) - offset.seconds;
        offset.minutes++;
    }
    else song->seconds -= offset.seconds;

    //Minutes
    if(offset.minutes > song->minutes)
    {
        song->minutes = (song->minutes + 60) - offset.minutes;
        offset.hours++;
    }
    else song->minutes -= offset.minutes;

    //Hours
    song->hours -= offset.hours;
}

void AddSongToOutput(FILE *output, Time *s_times, char *contents)
{
/* Procedure which adds a given song to the output file.
 * If the hours or minutes or seconds aren't two digit numbers,
 *  a 0 is being added at the front, to make the file's alignment
 *  is pretty, and also make the file easily readable
*/
    if((s_times->hours / 10) <= 0) fputc('0', output);
    fprintf(output, "%uh", s_times->hours);

    if((s_times->minutes / 10) <= 0) fputc('0', output);
    fprintf(output, "%um", s_times->minutes);

    if((s_times->seconds / 10) <= 0) fputc('0', output);
    fprintf(output, "%us", s_times->seconds);

    fprintf(output, " %s\n", contents);
}