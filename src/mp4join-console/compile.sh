#!/bin/bash
gcc -Wall -I./include -I./ -L./libs fappend.c main.c config.c aac.c mp4v.c mp4ops.c -o mp4join -lmpeg4ip_gnu -lmp4av -lmp4v2
