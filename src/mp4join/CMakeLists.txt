PROJECT(mp4join)

#if you don't want the full compiler output, remove the following line
SET(CMAKE_VERBOSE_MAKEFILE OFF)

SET(GTK_CFLAGS "`pkg-config --cflags gtk+-2.0`")
SET(GTK_LIBS "`pkg-config --libs gtk+-2.0`")
SET(MP4_LIBS "-L/usr/local/lib -lmpeg4ip_gnu -lmp4v2 -lmp4av ")

#add definitions, compiler switches, etc.   
ADD_DEFINITIONS(-Wall -O2 ${GTK_CFLAGS})
#list all source files here   
ADD_EXECUTABLE(mp4join main.c interface.c mp4fileops.c mp4ops.c mp4v.c aac.c fileops.c)

TARGET_LINK_LIBRARIES(mp4join ${GTK_LIBS} ${MP4_LIBS})

