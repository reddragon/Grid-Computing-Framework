#include <gtk/gtk.h>


gboolean
on_window1_destroy_event               (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_filechooserwidget1_file_activated   (GtkFileChooser  *filechooser,
                                        gpointer         user_data);

void
on_check_button_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_go_button_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_window2_delete_event                (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_back_clicked                        (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_window3_destroy_event               (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_combobox1_changed                   (GtkComboBox     *combobox,
                                        gpointer         user_data);

void
on_refresh_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_window1_delete_event                (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);
