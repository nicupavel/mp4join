#include <mpeg4ip.h>                                                                                                             
#include <mp4.h>                                                                                                                 
#include <mp4av.h> 

#include "mp4ops.h"
#include "aac.h"
#include "mp4v.h"

#define allowAvi 0
#define allowVariableFrameRate 0


MP4TrackId* CreateMediaTracks(MP4FileHandle mp4File, const char* inputFileName,
			      bool doEncrypt)
{
  FILE* inFile = fopen(inputFileName, "rb");

  if (inFile == NULL) {
    fprintf(stderr, 
	    "%s: can't open file %s: %s\n",
	    "CreateMediaTracks: ", inputFileName, strerror(errno));
    return NULL;
  }

  struct stat s;
  if (fstat(fileno(inFile), &s) < 0) {
    fprintf(stderr, 
	    "%s: can't stat file %s: %s\n",
	    "CreateMediaTracks", inputFileName, strerror(errno));
    return NULL;
  }

  if (s.st_size == 0) {
    fprintf(stderr, 
	    "%s: file %s is empty\n",
	    "CreateMediaTracks", inputFileName);
    return NULL;
  }

  const char* extension = strrchr(inputFileName, '.');
  if (extension == NULL) {
    fprintf(stderr, 
	    "%s: no file type extension\n", "CreateMediaTracks");
    return NULL;
  }

  static MP4TrackId trackIds[2] = {
    MP4_INVALID_TRACK_ID, MP4_INVALID_TRACK_ID
  };
  MP4TrackId* pTrackIds = trackIds;

 if (!strcasecmp(extension, ".aac")) {
    fprintf(stderr, "Creating AAC Audio track...\n");
    trackIds[0] = AacCreator(mp4File, inFile, doEncrypt);

  }  else if (!strcasecmp(extension, ".divx")
	     || !strcasecmp(extension, ".mp4v")
	     || !strcasecmp(extension, ".m4v")
	     || !strcasecmp(extension, ".xvid")
	     || !strcasecmp(extension, ".cmp")) {
    fprintf(stderr, "Creating Mp4Video track...\n");
    trackIds[0] = Mp4vCreator(mp4File, inFile, doEncrypt, allowVariableFrameRate);

  }  else {
    fprintf(stderr, 
	    "%s: unknown file extension in %s\n", "CreateMediaTracks", inputFileName);
    return NULL;
  }

  if (inFile) {
    fclose(inFile);
  }

  if (pTrackIds == NULL || pTrackIds[0] == MP4_INVALID_TRACK_ID) {
    return NULL;
  }

  return pTrackIds;
}


void ExtractTrack (MP4FileHandle mp4File, MP4TrackId trackId, 
		   const char* outputFileName)
{
  int openFlags = O_WRONLY | O_TRUNC | OPEN_CREAT;
  u_int8_t amrType = AMR_TYPE_NONE;
  int outFd = open(outputFileName, openFlags, 0644);


  if (outFd == -1) {
    fprintf(stderr, "%s: can't open %s: %s\n",
	    "extractor: ", outputFileName, strerror(errno));
    exit(-2);
  }

  // some track types have special needs
  // to properly recreate their raw ES file

  bool prependES = false;
  bool prependADTS = false;

  const char* trackType =
    MP4GetTrackType(mp4File, trackId);
  const char *media_data_name = 
    MP4GetTrackMediaDataName(mp4File, trackId);

  if (!strcmp(trackType, MP4_VIDEO_TRACK_TYPE)) {
    if (strcmp(media_data_name, "avc1") == 0) {
       fprintf(stderr, "Not implemented\n");
      //extract_h264_track(mp4File, trackId, outFd, outputFileName);
      return;
    }
    prependES = true;
  } else if (!strcmp(trackType, MP4_AUDIO_TRACK_TYPE)) {

    if (strcmp(media_data_name, "mp4a") == 0) {
      uint8_t type = 	
	MP4GetTrackEsdsObjectTypeId(mp4File, trackId);
      if (MP4_IS_AAC_AUDIO_TYPE(type) || 
	  type == MP4_MPEG4_INVALID_AUDIO_TYPE)
      prependADTS = true;
    } else if (strcmp(media_data_name, "sawb") == 0) {
      amrType = AMR_TYPE_AMRWB;
    } else if (strcmp(media_data_name, "samr") == 0) {
      amrType = AMR_TYPE_AMR;
    }
    switch (amrType) {
    case AMR_TYPE_AMR:
      if (write(outFd, AMR_MAGIC_AMR, AMR_MAGIC_LEN_AMR) != AMR_MAGIC_LEN_AMR) {
        fprintf(stderr, "%s: can't write to file: %s\n",
		"extractor: ", strerror(errno));
        return;
      }
      break;
    case AMR_TYPE_AMRWB:
      if (write(outFd, AMR_MAGIC_AMRWB, AMR_MAGIC_LEN_AMRWB) != AMR_MAGIC_LEN_AMRWB) {
        fprintf(stderr, "%s: can't write to file: %s\n",
		"extractor: ", strerror(errno));
        return;
      }
      break;
    default:
      break;
    }
  }

  MP4SampleId numSamples = 
#if 1
    MP4GetTrackNumberOfSamples(mp4File, trackId);
#else
  1;
#endif
  u_int8_t* pSample;
  u_int32_t sampleSize;
  MP4SampleId sampleId;
  // extraction loop
  for (sampleId = 1 ; sampleId <= numSamples; sampleId++) {
    int rc;

    // signal to ReadSample() 
    // that it should malloc a buffer for us
    pSample = NULL;
    sampleSize = 0;

    if (prependADTS) {
      // need some very specialized work for these
      MP4AV_AdtsMakeFrameFromMp4Sample(
				       mp4File,
				       trackId,
				       sampleId,
				       0, //aacProfileLevel
				       &pSample,
				       &sampleSize);
    } else {
      // read the sample
      rc = MP4ReadSample(
			 mp4File, 
			 trackId, 
			 sampleId, 
			 &pSample, 
			 &sampleSize,
			 NULL,
			 NULL,
			 NULL,
			 NULL);

      if (rc == 0) {
	fprintf(stderr, "%s: read sample %u for %s failed\n",
		"extractor: ", sampleId, outputFileName);
	exit(-2);
      }

      if (prependES && sampleId == 1) {
	u_int8_t* pConfig = NULL;
	u_int32_t configSize = 0;
	
	if (MP4GetTrackESConfiguration(mp4File, trackId, 
				       &pConfig, &configSize)) {
			
	  if (configSize != 0) {
	    write(outFd, pConfig, configSize);
	  }
	  CHECK_AND_FREE(pConfig);
	}

      }
    }

    rc = write(outFd, pSample, sampleSize);

    if (rc == -1 || (u_int32_t)rc != sampleSize) {
      fprintf(stderr, "%s: write to %s failed: %s\n",
	      "extractor: ", outputFileName, strerror(errno));
      exit(-2);
    }

    free(pSample);
  }

  // close ES file
  close(outFd);
}
