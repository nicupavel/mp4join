# mp4join



## Description
    - mp4join - joins 2 mp4 files with similar video/audio tracks encoding
    - mp4join outputs a new file which is ISO/IEC 14496-1 compliant.
    - mp4join is based on libmp4v2 and tools from mpeg4ip project

## Assumptions
    - mp4join requires 2 identical files in encoding parameters. Most important at this moment
      are: FrameRate, VideoSize, Codec, Sample Rate for audio.
    - Input mp4 files should have the video track as the first track.
    - mp4join doesn't work with broken mp4 files. It needs basic atom descriptors.
    
## Limitations
    - Supported video codecs: The video track creator only supports: xvid, m4v/mp4v, divx, cmp. 
      Profile variations doesn't matter.
    - H.264 is NOT SUPPORTED.
    - Supported audio codecs: The audio track creator (not finished) only supports AAC. 
      Profile variations doesn't matter (low complexity or main are ok)
    - Encryption of the tracks is not supported
    - There is no hinting of tracks (ie tracks won't be "streamable" thru Darwin Streaming Server)
    - Only Video and Audio tracks are supported. Scene descriptors (BIFS) and Object Descriptors (OD) are 
      not supported.
    - MP4 Metadata (author, title, genre) is partial supported (ie file has support for metadata but only
      TOOL field is filled with "getica mpeg4 joiner 0.1.5")
    - FrameRate is HARDCODED at 30. Variable Frame Rate is NOT SUPPORTED.
    - Audio and Video tracks should have the SAME time length there is no padding/trimming done on joining.
      Small differences <5s are "OK".
    - VIDEO track should be the FIRST track (1) of the file. This is a limitation of extract_track function.
    - AUDIO track should be the second track (2) of the file. This is a limitation of extract_track function.
    - AUDIO track is not muxed back into the new file the AACCreator code is not finished.
    - No 64bit support on files. Resulting file and extracted files should be <2Gb
    - Track TimeScale is HARDCODED.
    - Video Profile and Audio Profile are read from the VOL/VO header. 
      There is no way to specify a video/audio profile now.
    - For AAC only ADTS header is supported (older ADIF is not supported).

## Usage
Use run.sh to run the utility for console version or add mp4join.desktop to your desktop for GUI version.
      