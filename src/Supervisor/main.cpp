/*
 * Initial main.c file generated by Glade. Edit as required.
 * Glade will not overwrite this file.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <pthread.h>
#include "supervisor.cpp"

#include "interface.h"
#include "support.h"

int
main (int argc, char *argv[])
{
  GtkWidget * window;
  int window_choice = 10;
  

#ifdef ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif
    GThread   *thread;
    GError    *error = NULL;
	pthread_t worker;
	   
  
    if( ! g_thread_supported() )
        g_thread_init( NULL );
    
    /* Secure gtk */
    gdk_threads_init();
 
    /* Obtain gtk's global lock */
    gdk_threads_enter();
 
    /* Do stuff as usual */
    gtk_init( &argc, &argv );
    
      
    
    /* Create new thread */
    thread = g_thread_create( thread_func, (gpointer)NULL,
                              FALSE, &error );
    if( ! thread )
    {
        g_print( "Error: %s\n", error->message );
        return( -1 );
    }
  
  //add_pixmap_directory (PACKAGE_DATA_DIR "/" PACKAGE "/pixmaps");

  /*
   * The following code was added by Glade to create one of each component
   * (except popup menus), just so that you see something after building
   * the project. Delete any components that you don't want shown initially.
   */
  window = create_window1 ();
  gtk_widget_show (window);
  //window2 = create_window2 ();
  //gtk_widget_show (window2);
  //window3 = create_window3 ();
  //gtk_widget_show (window3);
  
  pthread_t supervisor_starter_thread;
  pthread_create(&supervisor_starter_thread, NULL, &(supervisor_starter), &window_choice);
  pthread_create(&worker,NULL,&updater,NULL);
  gtk_main ();
    
  pthread_join(worker,NULL);
  pthread_join(supervisor_starter_thread, NULL);   
  return 0;
}

