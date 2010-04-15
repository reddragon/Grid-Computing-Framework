#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

void update_status()
{
	char temp[1000]="\tIP address:\n\tTask Id:\n\tTask Priority:\n\tStatus: Compiling/Executing/Error\n\tResult: \n\tTime of Exceution: \n\tError(if any): \n new ";
	char number[100] ;
	sprintf(number,"%d",index_of_active_client);
	strcat(temp,number);	
	gtk_text_buffer_set_text (gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview1)), (temp), -1);
	gtk_widget_show(textview1);
}


gboolean
on_window1_destroy_event               (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{

  return FALSE;
}


void
on_filechooserwidget1_file_activated   (GtkFileChooser  *filechooser,
                                        gpointer         user_data)
{

}


void
on_check_button_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_go_button_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{

}


gboolean
on_window2_delete_event                (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{

  return FALSE;
}


void
on_button7_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_combobox1_changed                   (GtkComboBox     *combobox,
                                        gpointer         user_data)
{
index_of_active_client = gtk_combo_box_get_active(combobox);
update_status();

}


void
on_update_button_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
update_status();

}

