#include "mp4fileops.h"
#include "interface.h"


/* Main Application window */
static GtkWidget *window = NULL;

static struct {
    char *file;
    GtkWidget *widget;
    GtkWidget *label;
    const char *mp4info;
} file_indexes[] = {
    { NULL, NULL, NULL, "No file loaded" },
    { NULL, NULL, NULL, "No file loaded" },
    { NULL, NULL, NULL, "No file created" },
};

/* progress bar data struct passed to progress bar callback -- not used yet*/
typedef struct _pbar_data_s pbar_data;
struct _pbar_data_s {
    guint id;
    GtkWidget *progress;
    char *message;
    int percent;
} * pdata;

/* progress bar timer function */
static gboolean pbar_cb ( gpointer data )
{    
    pbar_data *pdata = (pbar_data *) data;
    
    gtk_progress_bar_set_text (GTK_PROGRESS_BAR(pdata->progress), pdata->message);
    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(pdata->progress), (double)pdata->percent/100);

    return TRUE;
}

void gtk_message(char *string, int p)
{
  pdata->message = string;
  fprintf (stderr, "%s\n", pdata->message);
  pdata->percent = p;
}

void std_message(char *string, int progress)
{
    fprintf (stderr, "%s\n", string);
}

static void gtk_do_join(void)
{
    /* For future extensions */
    do_join(file_indexes[0].file, file_indexes[1].file, file_indexes[2].file);
}

static void select_file_cb (GtkWidget * widget, gpointer data)
{
    GtkWidget *fs;
    GtkFileFilter *filter;
    int response;
    
    unsigned int index = GPOINTER_TO_INT(data);
    
    fs = gtk_file_chooser_dialog_new ("MPEG 4 File to join ", NULL,
                                      GTK_FILE_CHOOSER_ACTION_OPEN,
                                      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                      GTK_STOCK_OK,     GTK_RESPONSE_OK,
                                      NULL);
    
    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, "All files");
    gtk_file_filter_add_pattern (filter, "*");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(fs), filter);
    
    filter = gtk_file_filter_new();
    
    gtk_file_filter_set_name (filter, "MPEG 4 files");
    gtk_file_filter_add_pattern (filter, "*.mp4");
    gtk_file_filter_add_pattern (filter, "*.mpeg4");
    gtk_file_filter_add_pattern (filter, "*.mpg");
    gtk_file_filter_add_pattern (filter, "*.MP4");
    gtk_file_filter_add_pattern (filter, "*.MPG");
    
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(fs), filter);
    gtk_file_chooser_set_filter (GTK_FILE_CHOOSER(fs), filter);
    
    response = gtk_dialog_run (GTK_DIALOG (fs));
    gtk_widget_hide (fs);

    if (response == GTK_RESPONSE_OK) {
        file_indexes[index].file = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fs));
        gtk_entry_set_text (GTK_ENTRY(file_indexes[index].widget), file_indexes[index].file);
        
        file_indexes[index].mp4info = MP4FileInfo(file_indexes[index].file,MP4_INVALID_TRACK_ID);
        gtk_label_set_text (GTK_LABEL(file_indexes[index].label), file_indexes[index].mp4info);
    }

    gtk_widget_destroy (fs);
}

static void save_file_dialog (GtkWidget * widget, gpointer data)
{
    GtkWidget *fs;
    int response;
    unsigned int index = GPOINTER_TO_INT(data);
    
    file_indexes[index].file = (char *) gtk_entry_get_text (GTK_ENTRY(file_indexes[index].widget));
    
    fs = gtk_file_chooser_dialog_new ("Select file to save", NULL,
                                      GTK_FILE_CHOOSER_ACTION_SAVE,
                                      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                      GTK_STOCK_OK,     GTK_RESPONSE_OK,
                                      NULL);
    
    if (file_indexes[index].file) {
        gtk_file_chooser_set_filename (GTK_FILE_CHOOSER(fs), file_indexes[index].file);
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(fs), file_indexes[index].file);
    }
    
    response = gtk_dialog_run (GTK_DIALOG (fs));
    gtk_widget_hide (fs);

    if (response == GTK_RESPONSE_OK) {
        file_indexes[index].file = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fs));
        gtk_entry_set_text (GTK_ENTRY(file_indexes[index].widget), file_indexes[index].file);
    }
    gtk_widget_destroy (fs);
    
}

static void save_file_cb(GtkWidget * widget, gpointer data)
{
    unsigned int index = GPOINTER_TO_INT(data);
    
    file_indexes[index].file = (char *) gtk_entry_get_text (GTK_ENTRY(file_indexes[index].widget));
    
    if (g_fopen(file_indexes[index].file, "w") == NULL)
        save_file_dialog(widget, data);
    
    if (g_fopen(file_indexes[index].file, "w") == NULL)
        return;
    
    gtk_do_join();
}

static void about_cb( GtkWidget *widget,
                   gpointer   data )
{
    GtkWidget *dialog;
    
    g_print ("Mp4Join (c) 2007 Nicu Pavel\n");

    dialog = gtk_about_dialog_new();
    
    gtk_about_dialog_set_name(GTK_ABOUT_DIALOG (dialog),"MP4 Join");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG (dialog), "0.2");
    gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG (dialog),"Copyright 2007 Nicu Pavel");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG (dialog),"A tool for joining 2 mpeg4 files");
    gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog),"http://www.linuxconsulting.ro/mp4join");

    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}

static gboolean delete_event_cb( GtkWidget *widget,
                              GdkEvent  *event,
                              gpointer   data )
{
    return FALSE;
}

static void destroy_cb( GtkWidget *widget,
                     gpointer   data )
{
    g_free(pdata);
    gtk_main_quit ();
}

int run_interface( void )
{
    /* GtkWidget is the storage type for widgets */
    GtkWidget *about_button, *close_button, *loadfile1, *loadfile2, *savefile;
    GtkWidget *vbox, *hbox;
    GtkWidget *label;
    GtkWidget *expander1, *expander2;
    GtkWidget *separator;
    GtkWidget *temp;
        
    /* initialize progress bar data */
    pdata = g_malloc (sizeof(pbar_data));
    pdata->message = "Select files to join and press SAVE";
    pdata->percent = 0;
    pdata->progress = NULL;
    
    /* create a new window */
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size (GTK_WINDOW(window), 600, 400);
    gtk_window_set_resizable (GTK_WINDOW (window), TRUE);
    gtk_window_set_title(GTK_WINDOW(window), "MP4Join");
    
    g_signal_connect (G_OBJECT (window), "delete_event",
                      G_CALLBACK (delete_event_cb), NULL);

    g_signal_connect (G_OBJECT (window), "destroy",
                      G_CALLBACK (destroy_cb), NULL);
    
    /* Sets the border width of the window. */
    //gtk_container_set_border_width (GTK_CONTAINER (window), 10);

    vbox = gtk_vbox_new (FALSE, 10);
    gtk_container_add (GTK_CONTAINER(window), vbox);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
    
    label = gtk_label_new ("MP4Join - joins 2 mp4(mpeg4) files");
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
    
    /* Create a new horizontal box */
    hbox = gtk_hbox_new (FALSE, 10);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 8);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    
    /* Creates a new entry field */
    file_indexes[0].widget = gtk_entry_new ();
    gtk_box_pack_start (GTK_BOX (hbox), file_indexes[0].widget, TRUE, TRUE, 0);
    
    /* Creates a new button */
    loadfile1 = gtk_button_new_from_stock (GTK_STOCK_OPEN);
    gtk_box_pack_start (GTK_BOX (hbox), loadfile1, FALSE, FALSE, 0);
    
    /* Create the expander */
    expander1 = gtk_expander_new ("File Details");
    gtk_box_pack_start (GTK_BOX (vbox), expander1, FALSE, FALSE, 0);

    file_indexes[0].label = gtk_label_new (file_indexes[0].mp4info);
    gtk_container_add (GTK_CONTAINER (expander1), file_indexes[0].label);
    
    /* Create a new horizontal box */
    hbox = gtk_hbox_new (FALSE, 10);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 8);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
        
    /* Creates a new entry field */
    file_indexes[1].widget = gtk_entry_new ();
    gtk_box_pack_start (GTK_BOX (hbox), file_indexes[1].widget, TRUE, TRUE, 0);
    
    /* Creates a new button */
    loadfile2 = gtk_button_new_from_stock (GTK_STOCK_OPEN);
    gtk_box_pack_start (GTK_BOX (hbox), loadfile2, FALSE, FALSE, 0);
    
    /* Create the expander to show the media informations */
    expander2 = gtk_expander_new ("File Details");
    gtk_box_pack_start (GTK_BOX (vbox), expander2, FALSE, FALSE, 0);

    file_indexes[1].label = gtk_label_new (file_indexes[0].mp4info);
    gtk_container_add (GTK_CONTAINER (expander2), file_indexes[1].label);
    
    separator = gtk_hseparator_new();
    gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, FALSE, 0);
    
    
    /* New horizontal box */
    hbox = gtk_hbox_new (FALSE, 10);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 8);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    
    /* Entry field Widget MP4 Save File*/
    file_indexes[2].widget = gtk_entry_new ();
    gtk_box_pack_start (GTK_BOX (hbox), file_indexes[2].widget, TRUE, TRUE, 0);
    
    /* Entry field Widget MP4 Target file*/
    savefile = gtk_button_new_from_stock (GTK_STOCK_SAVE);
    gtk_box_pack_start (GTK_BOX (hbox), savefile, FALSE, FALSE, 0);
    
    /* Create the expander to show the media informations */
    expander1 = gtk_expander_new ("Settings");
    gtk_box_pack_start (GTK_BOX (vbox), expander1, FALSE, FALSE, 0);

    label = gtk_label_new ("Settings not available yet");
    gtk_container_add (GTK_CONTAINER (expander1), label);
    
    temp = gtk_hseparator_new();
    gtk_box_pack_start (GTK_BOX (vbox), temp, FALSE, FALSE, 0);
    
    /* Align the label */
    /* No longer used text written directly into progress bar */
    /*
    temp = gtk_alignment_new(0, 0, 0, 0);
    gtk_box_pack_start (GTK_BOX(vbox), temp, FALSE, FALSE, 0);
    status = gtk_label_new("Status: Idle");
    gtk_container_add (GTK_CONTAINER(temp), status);
    */
    
    /* Progress bar widget */
    /* disabled atm */
    /*
    pdata->progress = gtk_progress_bar_new();
    gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(progress),  GTK_PROGRESS_LEFT_TO_RIGHT);
    gtk_box_pack_start (GTK_BOX (vbox), progress, FALSE, FALSE, 0);
    pdata->id = g_timeout_add (100, pbar_cb, pdata);
    */
    
    /* Create a new horizontal box used for a little spacing*/
    hbox = gtk_hbox_new (FALSE, 10);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
        
    temp = gtk_hseparator_new();
    gtk_box_pack_start (GTK_BOX (hbox), temp, FALSE, FALSE, 0);
    
    /* Alignment widget for the button to have fixed size */
    /*
    temp = gtk_alignment_new(0, 0, 0, 0);
    gtk_box_pack_start (GTK_BOX (hbox), temp, FALSE, FALSE, 0);
    */
    
    /* Creates a new button */
    about_button = gtk_button_new_from_stock (GTK_STOCK_ABOUT);
    close_button = gtk_button_new_from_stock (GTK_STOCK_QUIT);
    gtk_box_pack_start (GTK_BOX (hbox), about_button, TRUE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), close_button, TRUE, FALSE, 0);
    

    g_signal_connect (G_OBJECT (about_button), "clicked",
                      G_CALLBACK (about_cb), NULL);

    g_signal_connect_swapped (G_OBJECT (close_button), "clicked",
                              G_CALLBACK (gtk_widget_destroy),
                                          G_OBJECT (window));
    
    /* MP4 loading and saving buttons  events*/
    g_signal_connect (G_OBJECT (loadfile1), "clicked",
                      G_CALLBACK (select_file_cb), GINT_TO_POINTER(0));
        
    g_signal_connect (G_OBJECT (loadfile2), "clicked",
                      G_CALLBACK (select_file_cb), GINT_TO_POINTER(1));
    
    g_signal_connect (G_OBJECT (savefile), "clicked",
                      G_CALLBACK (save_file_cb), GINT_TO_POINTER(2));
    
    if (!GTK_WIDGET_VISIBLE (window))
        gtk_widget_show_all (window);
    else
        gtk_widget_destroy (window);
    
    gtk_main ();

    return 0;
}
