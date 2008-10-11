/* maximum message length which is passed to gtk_message or std_message */
#define MAX_MSG_LEN 2048

/* How many tracks to join */
#define MAX_TRACKS 2

#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gstdio.h>

/* Pointer to function to write to GTK interface or STDout interface */
void (*message) (char *string, int progress);
/* GTK message function */
void gtk_message(char *string, int progress);

/* STDIO message function */
void std_message(char *string, int progress);

/* main GTK interface function */
int run_interface(void);
