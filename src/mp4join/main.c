#include <stdio.h>

#include "interface.h"

#define GTK 1

#ifdef GTK
    #include <gtk/gtk.h>
#endif

int main( int   argc, char *argv[] )
{
    int res;

#ifdef GTK    
    message = &gtk_message;
    gtk_init(&argc, &argv);
    res = run_interface();
    return 0;
#endif
    
    message = &std_message;
    return 0;
}
