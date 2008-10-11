#include <mpeg4ip.h>
#include <mp4.h>
#include <mp4av.h>


#define AMR_TYPE_NONE 0
#define AMR_TYPE_AMR 1
#define AMR_TYPE_AMRWB 2

#define AMR_MAGIC_LEN_AMR 6
#define AMR_MAGIC_AMR "#!AMR\n"

#define AMR_MAGIC_LEN_AMRWB 9
#define AMR_MAGIC_AMRWB "#!AMR-WB\n"

MP4TrackId* CreateMediaTracks(MP4FileHandle mp4File, const char* inputFileName, bool doEncrypt);
void ExtractTrack (MP4FileHandle mp4File, MP4TrackId trackId, const char* outputFileName);

