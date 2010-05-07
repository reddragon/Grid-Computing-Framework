#include <gtk/gtk.h>

gboolean on_window1_delete_event                (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void on_recover_old_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void on_start_new_clicked               (GtkButton       *button,
                                        gpointer         user_data);

gboolean on_window2_delete_event        (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void on_filechooserwidget1_file_activated   (GtkFileChooser  *filechooser,
                                        gpointer         user_data);

void on_upload_file_clicked             (GtkButton       *button,
                                        gpointer         user_data);

gboolean on_window3_delete_event        (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void on_stop_execution_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void on_combobox1_changed               (GtkComboBox     *combobox,
                                        gpointer         user_data);

void on_display_client_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void update_status();

void get_xml_path(char []);

int get_status_recover();

