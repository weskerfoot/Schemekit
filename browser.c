#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <stdlib.h>
#include <libguile.h>
#include <libguile/strings.h>

static void
destroyWindowCb(GtkWidget *widget, GtkWidget *window);

static gboolean
closeWebViewCb(WebKitWebView *webView, GtkWidget *window);

static void
destroyWindowCb(GtkWidget *widget, GtkWidget *window) {
    gtk_main_quit();
}

static gboolean
closeWebViewCb(WebKitWebView *webView,
               GtkWidget *window) {
    gtk_widget_destroy(window);
    return TRUE;
}

WebKitWebView *webView;

static SCM
start_browser(void) {
  /* Initialize GTK+ */
  gtk_init(0, NULL);

  /* Create an 800x600 window that will contain the browser instance */
  GtkWidget *main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  gtk_window_set_default_size(GTK_WINDOW(main_window), 800, 600);

  /* Create a browser instance */
  webView = WEBKIT_WEB_VIEW(webkit_web_view_new());

  /* Put the browser area into the main window */
  gtk_container_add(GTK_CONTAINER(main_window), GTK_WIDGET(webView));

  /* Set up callbacks so that if either the main window or the browser instance is */
  /* closed, the program will exit */
  g_signal_connect(main_window, "destroy", G_CALLBACK(destroyWindowCb), NULL);
  g_signal_connect(webView, "close", G_CALLBACK(closeWebViewCb), main_window);

  /* Load a web page into the browser instance */
  webkit_web_view_load_uri(webView, "http://google.com/");

  /* Make sure that when the browser area becomes visible, it will get mouse */
  /* and keyboard events */
  gtk_widget_grab_focus(GTK_WIDGET(webView));

  /* Make sure the main window and all its contents are visible */
  gtk_widget_show_all(main_window);

  /* Run the main GTK+ event loop */
  gtk_main();

  return SCM_BOOL_T;
}

static SCM
open_page(SCM mystring) {
  char *c_string = scm_to_locale_string(mystring);
  printf("Opening %s\n", c_string);
  webkit_web_view_load_uri(webView, c_string);
  return SCM_BOOL_T;
}

static void
inner_main(void *data, int argc, char **argv) {
  scm_c_define_gsubr("start-browser", 0, 0, 0, start_browser);
  scm_c_define_gsubr("open-page", 1, 0, 0, open_page);
  scm_shell(argc, argv);
}

int main(int argc, char *argv[]) {
    /* Initialize Guile */
    scm_boot_guile(argc, argv, inner_main, 0);
    return 0;
}
