#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <glib/gstdio.h>

#include <mpeg4ip.h>
#include "fileops.h"
#include "mp4ops.h"
#include "aac.h"
#include "interface.h"

#include "mp4fileops.h"

/* List of tracks to extract */
char *extract_track_types[] = { MP4_VIDEO_TRACK_TYPE, MP4_AUDIO_TRACK_TYPE, NULL };


void do_join(char *infile1, char *infile2, char *outfile)
{
    MP4FileHandle mp4File1, mp4File2, mp4NewFile;
    MP4TrackId trackId1, trackId2, *pTrackId;
        
    char **type;
    char *path, *file1, *file2, *trackname1, *trackname2, *ext;
    char *msg;
    int res;

    if (infile1 == NULL || infile2 == NULL || outfile == NULL ) 
        return;

    /* Debug messages */
    msg = (char *) malloc (sizeof(char) * MAX_MSG_LEN);
    /* Holds temporary path - output file path */
    path = (char *) malloc (sizeof(char)*255);
    /* Holds tracknames */
    trackname1 = (char *) malloc (sizeof(char)*255);
    trackname2 = (char *) malloc (sizeof(char)*255);
    
    memset(trackname1, 0, 255);
    memset(trackname2, 0, 255);
    
    get_path(outfile, path);
            
    /* Create the new mp4 container */
    mp4NewFile = MP4Create(outfile,  MP4_DETAILS_ERROR, 0); 
    /* Hardcoded. ToDo: get Timescale from new tracks MP4GetTrackTimeScale */
    MP4SetTimeScale(mp4NewFile, 90000);
     
    type = extract_track_types;
    
    snprintf (msg, MAX_MSG_LEN, "Reading file %s\n", infile1);
    message(msg, 10);
    
    mp4File1 = MP4Read(infile1, 0);
    
    snprintf (msg, MAX_MSG_LEN, "Reading file %s\n", infile1);
    message(msg, 20);
    
    mp4File2 = MP4Read(infile2, 0);
    
    file1 = get_filename(infile1);
    file2 = get_filename(infile2);
    
    
    while (*type != NULL) {
        trackId1 = MP4FindTrackId(mp4File1, 0, *type, 0);
        trackId2 = MP4FindTrackId(mp4File2, 0, *type, 0);
        
        if (*type == MP4_VIDEO_TRACK_TYPE) ext = ".m4v";
        else ext = ".aac";
                
        snprintf(trackname1, 255, "%s%s%s",path, file1, ext);
        snprintf(trackname2, 255, "%s%s%s",path, file2, ext);
        
        snprintf (msg, MAX_MSG_LEN, "%s: extracting track type %s to %s\n",infile1, *type, trackname1);
        message(msg, 30);
        ExtractTrack(mp4File1, trackId1, trackname1);
        
        snprintf (msg, MAX_MSG_LEN, "%s: extracting track type %s to %s\n",infile2, *type, trackname2);
        message(msg, 40);
        ExtractTrack(mp4File2, trackId2, trackname2);
        
        snprintf (msg, MAX_MSG_LEN, "Joining %s tracks %s and %s to %s\n", *type, trackname1,  trackname2, trackname2);
        message(msg, 50);
        res = fappend(trackname1, trackname2);
                
        if (res < 0) 
             break;
        
        snprintf (msg, MAX_MSG_LEN, "Adding track %s to %s\n", trackname2, outfile);
        message(msg, 60);
        
        pTrackId = CreateMediaTracks(mp4NewFile, trackname2, 0);
        
        if (pTrackId == NULL) 
            break;
        
        snprintf (msg, MAX_MSG_LEN, "Removing temporary tracks\n");
        message(msg, 70);
        
        g_remove(trackname1);
        g_remove(trackname2);
        
        type++;
    }

    MP4Close(mp4File1);
    MP4Close(mp4File2);
    MP4Close(mp4NewFile);
   
    free(file1);
    free(file2);
    free(trackname1);
    free(trackname2);
    free(path);
    
    snprintf (msg, MAX_MSG_LEN, "Done\n");
    message(msg, 0);
    
    free(msg);
}


unsigned int get_tracktype (MP4FileHandle mp4File, MP4TrackId track)
{
    unsigned long long int temp;
    unsigned int verbosity;
    int ret;
    const char *type = MP4GetTrackType(mp4File, track);
    
    // look for objectTypeId (GetTrackEsdsObjectTypeId)
    verbosity = MP4_DETAILS_ERROR & ~(MP4_DETAILS_ERROR);
    MP4SetVerbosity(mp4File, verbosity);
    ret = MP4GetTrackIntegerProperty(mp4File,
            track, "mdia.minf.stbl.stsd.*.esds.decConfigDescr.objectTypeId", &temp);
    
    MP4SetVerbosity(mp4File, MP4_DETAILS_ERROR);
    if (ret) {
        if (!strcmp(type, MP4_AUDIO_TRACK_TYPE)) {
           return MP4_MPEG4_AUDIO_TYPE;
        } else if (!strcmp(type, MP4_VIDEO_TRACK_TYPE)) {
            return MP4_MPEG4_VIDEO_TYPE;
        }
    }
    return MP4_INVALID_TRACK_ID;
}

    
void set_metadata (MP4FileHandle mp4File)
{
    unsigned int verbosity;
    char *buffer;
    char *value;
    int ret;
    
    verbosity = MP4_DETAILS_ERROR & ~(MP4_DETAILS_ERROR);
    MP4SetVerbosity(mp4File, verbosity);
    ret = MP4GetMetadataTool(mp4File, &value);
    MP4SetVerbosity(mp4File, MP4_DETAILS_ERROR);
    if (ret && value != NULL) {
        if (strncasecmp(TOOLTAG, value, strlen(TOOLTAG)) != 0) {
            buffer = (char *)malloc(strlen(value) + 80);
            sprintf(buffer, "%s TOOLTAG %s", value, "TOOLVERSION");
            MP4SetMetadataTool(mp4File, buffer);
            free(buffer);
        }
    } else {
        buffer = (char *)malloc(80);
        sprintf(buffer, "TOOLTAG %s", "TOOLVERSION");
        MP4SetMetadataTool(mp4File, buffer);
    }
}


