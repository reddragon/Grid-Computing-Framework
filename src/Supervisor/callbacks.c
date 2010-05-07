#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"


gchar * text_of_active_client ;
char xml_path[100];
int recover_tasks;

void get_xml_path(char x[])
{
	strncpy(x,xml_path,100);	
}

int get_status_recover()
{
	return recover_tasks;	
}

gboolean
on_window1_delete_event                (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  gtk_widget_destroy(widget);	
  gtk_main_quit();
  exit(0);
  return FALSE;
}


void
on_recover_old_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  delete_window1_show2(); 
  recover_tasks = 1;
}


void
on_start_new_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
  delete_window1_show2();
  recover_tasks = 0;
}


gboolean
on_window2_delete_event                (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  gtk_widget_destroy(widget);	
  gtk_main_quit();
  exit(0);
  return FALSE;
}


void
on_filechooserwidget1_file_activated   (GtkFileChooser  *filechooser,
                                        gpointer         user_data)
{

 GFile *fp = gtk_file_chooser_get_file(filechooser); 
 strcpy(xml_path,g_file_get_path(fp));
}

void on_stop_execution_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{ }

void
on_upload_file_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
 delete_window2_show3();
 xml_signal();
}


gboolean
on_window3_delete_event                (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  gtk_widget_destroy(widget);	
  gtk_main_quit();
  exit(0);
  return FALSE;
}


void on_combobox1_changed                   (GtkComboBox     *combobox,
                                        gpointer         user_data)
{
text_of_active_client = gtk_combo_box_get_active_text(combobox);
}


void on_display_client_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
update_status(text_of_active_client);
}



