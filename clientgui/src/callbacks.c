#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"


void update_progress()
{ 	int percent=60;
	char temp[1000]="Progress: ";
	char number[100] ;
	sprintf(number,"%d",percent);
	strcat(temp,number);
	strcat(temp," %");	
	gtk_entry_set_text (GTK_ENTRY (entry1), _(temp));
	gtk_widget_show(entry1);
}
