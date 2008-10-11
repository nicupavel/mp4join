#include<stdio.h>

#include <mpeg4ip.h>                                                                                                             
#include <mp4.h>                                                                                                                 
#include <mp4av.h> 

#include <time.h>

#include "fappend.h"
#include "mp4ops.h"
#include "aac.h"
#include "config.h"

#include "main.h"


int main (int argc, char **argv)
{
    clock_t start, end;
    double duration, totalduration = 0.0;
    uint32_t newverbosity;
    int res;
    config *params;
    

    params = (config *) malloc (sizeof (config));
    
    res = parse_cmd_line(argc, argv, params);
    if ( res < 0 ) { 
	    fprintf(stderr, "Usage: %s infile1 infile2 outfile\n", argv[0]);
	    return 1;
    }
    
    printf("Checking file: %s...\n", params->infile1);
    //MP4TrackId extractTrackId = MP4_INVALID_TRACK_ID;
    char* info = MP4FileInfo(params->infile1, MP4_INVALID_TRACK_ID);
    MP4FileHandle mp4File; 

    if (!info) {
	fprintf(stderr, "%s: can't open %s\n", argv[0], params->infile1);
	exit(-1);
    }

    fputs(info, stdout);
    free(info);
    
    mp4File = MP4Read(params->infile1, 0);
    if (!mp4File) {
	fprintf(stderr, "Error reading MP4 file\n");    
	exit(-2);
    }
    
    printf("Extracting video track ...\n");
    start = clock();
    ExtractTrack(mp4File, 1, "samples/1.m4v"); 
    end = clock();
    duration = (end-start)*1000 / CLOCKS_PER_SEC;
    totalduration += duration;
    printf("Time spent: %2.1f ms\n", duration);
    
    printf("Extracting audio track ...\n");
    start = clock();
    ExtractTrack(mp4File, 2, "samples/1.aac"); 
    end = clock();
    duration = (end-start)*1000 / CLOCKS_PER_SEC;
    totalduration += duration;
    printf("Time spent: %2.1f ms\n", duration);
    MP4Close(mp4File);

// second file

    printf("Checking file: %s...\n", params->infile2);
    char* dinfo = MP4FileInfo(params->infile2, MP4_INVALID_TRACK_ID);

    if (!dinfo) {
	fprintf(stderr, "%s: can't open %s\n", argv[0], params->infile2);
	exit(-1);
    }

    fputs(dinfo, stdout);
    free(dinfo);
    
    mp4File = MP4Read(params->infile2, 0);
    if (!mp4File) {
	fprintf(stderr, "Error reading MP4 file\n");    
	exit(-2);
    }

    printf("Extracting video track ...\n");
    start = clock();
    ExtractTrack(mp4File, 1, "samples/2.m4v"); 
    end = clock();
    duration = (end-start)*1000 / CLOCKS_PER_SEC;
    totalduration += duration;
    printf("Time spent: %2.1f ms\n", duration);
    
    printf("Extracting audio track ...\n");
    start = clock();
    ExtractTrack(mp4File, 2, "samples/2.aac"); 
    end = clock();
    duration = (end-start)*1000 / CLOCKS_PER_SEC;
    totalduration += duration;
    printf("Time spent: %2.1f ms\n", duration);
    MP4Close(mp4File);
    
    printf("Joining video tracks ...\n");
    start = clock();
    res = fappend ("samples/1.m4v", "samples/2.m4v");
    end = clock();
    duration = (end-start)*1000 / CLOCKS_PER_SEC;
    totalduration += duration;
    printf("Time spent: %2.1f ms\n", duration);
    
    if (res < 0) {
	fprintf(stderr, "Error in join\n");
	exit (-1);
    }
    
    printf("Joining audio tracks ...\n");
    start = clock();
    res = fappend ("samples/1.aac", "samples/2.aac");
    
    end = clock();
    duration = (end-start)*1000 / CLOCKS_PER_SEC;
    totalduration += duration;
    printf("Time spent: %2.1f ms\n", duration);
    
    if (res < 0) {
	fprintf(stderr, "Error in join\n");
	exit (-1);
    }


    
    printf("Creating the new file...\n");

    start = clock();    
    // No 64 bit support for now
    mp4File = MP4Create(params->outfile,  MP4_DETAILS_ERROR, 0); 
    //Hardcoded future: get Timescale from new tracks MP4GetTrackTimeScale
    MP4SetTimeScale(mp4File, 90000);
    
    //Open the file created for modify: for future in file modification
    //mp4File = MP4Modify("samples/new.mp4", MP4_DETAILS_ERROR, 0);
    
    bool allMpeg4Streams = true;
    
    static MP4TrackId CreatedTrackIds[MAX_TRACKS] = { 
	MP4_INVALID_TRACK_ID, MP4_INVALID_TRACK_ID 
    };
    
    MP4TrackId *pTrackId;
    int index = 0;
    
    pTrackId = CreateMediaTracks(mp4File, "samples/2.m4v", 0);
    if (pTrackId == NULL) {
    	MP4Close(mp4File);
    	fprintf(stderr, "Error Adding track 2.m4v\n");    
	exit(-4);
    }
    
    CreatedTrackIds[index] = *pTrackId;
    
    pTrackId = CreateMediaTracks(mp4File, "samples/2.aac", 0);
    
    if (pTrackId == NULL) {
    	MP4Close(mp4File);
    	fprintf(stderr, "Error Adding track 2.aac\n");    
	exit(-4);
    }
    CreatedTrackIds[index + 1] = *pTrackId;
    
    end = clock();
    duration = (end-start)*1000 / CLOCKS_PER_SEC;
    totalduration += duration;
    printf("Time spent: %2.1f ms\n", duration);
    
    while (index < MAX_TRACKS) {			       
	const char *type = MP4GetTrackType(mp4File, CreatedTrackIds[index]);
	// look for objectTypeId (GetTrackEsdsObjectTypeId)
	uint64_t temp;
	newverbosity = MP4_DETAILS_ERROR & ~(MP4_DETAILS_ERROR);
	MP4SetVerbosity(mp4File, newverbosity);
	fprintf(stderr, "--------------------------\n");
	bool ret = MP4GetTrackIntegerProperty(mp4File, CreatedTrackIds[index],
			 "mdia.minf.stbl.stsd.*.esds.decConfigDescr.objectTypeId",
			 &temp);
	MP4SetVerbosity(mp4File, MP4_DETAILS_ERROR);
	if (ret) {
	  if (!strcmp(type, MP4_AUDIO_TRACK_TYPE)) { 
	    printf("Track type Audio ... \n");
	    allMpeg4Streams &= (MP4GetTrackEsdsObjectTypeId(mp4File, CreatedTrackIds[index]) == MP4_MPEG4_AUDIO_TYPE);
	  } else if (!strcmp(type, MP4_VIDEO_TRACK_TYPE)) { 
	    printf("Track type Video ... \n");
	    allMpeg4Streams &= (MP4GetTrackEsdsObjectTypeId(mp4File, CreatedTrackIds[index]) == MP4_MPEG4_VIDEO_TYPE);
	  }
	}
	index++;
      }

    fprintf(stderr, "Adding tool tag ...\n");
    char *buffer;
    char *value;
    
    newverbosity = MP4_DETAILS_ERROR & ~(MP4_DETAILS_ERROR);
    MP4SetVerbosity(mp4File, newverbosity);
    bool retval = MP4GetMetadataTool(mp4File, &value);
    MP4SetVerbosity(mp4File, MP4_DETAILS_ERROR);
    if (retval && value != NULL) {
      if (strncasecmp("panic mpeg joiner", value, strlen("panic mpeg joiner")) != 0) {
	buffer = (char *)malloc(strlen(value) + 80);
	sprintf(buffer, "%s panic mpeg joiner %s", value, "0.0.1");
	MP4SetMetadataTool(mp4File, buffer);
	free(buffer);
      }
    } else {
      buffer = (char *)malloc(80);
      sprintf(buffer, "panic mpeg joiner %s", "0.0.1");
      MP4SetMetadataTool(mp4File, buffer);
    }

    start = clock();
    
    fprintf(stderr, "Flushing and closing file...\n");
    MP4Close(mp4File);
    
    end = clock();
    duration = (end-start)*1000 / CLOCKS_PER_SEC;
    totalduration += duration;
    printf("Time spent: %2.1f ms\n", duration);
    
    
    printf("Total time spent: %2.1f ms\n", totalduration);
    printf("Output file info:\n");
    
    dinfo = MP4FileInfo(params->outfile, MP4_INVALID_TRACK_ID);

    if (!dinfo) {
	fprintf(stderr, "%s: can't open %s\n", argv[0], params->outfile);
	exit(-1);
    }

    fputs(dinfo, stdout);
    free(dinfo);
    
    return 0;
}
