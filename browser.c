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

SCM
launch_webkit(SCM webview) {
  WebKitWebView *webView = scm_to_pointer(webview);
  /* Initialize GTK+ */
  gtk_init(0, NULL);

  /* Create an 800x600 window that will contain the browser instance */
  GtkWidget *main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  gtk_window_set_default_size(GTK_WINDOW(main_window), 800, 600);

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
open_page(SCM webview, SCM scm_url) {
  char *url = scm_to_locale_string(scm_url);
  printf("Opening %s\n", url);
  WebKitWebView *webView = scm_to_pointer(webview);
  webkit_web_view_load_uri(webView, url);
  return SCM_BOOL_T;
}

static SCM
make_webview() {
  WebKitWebView *webView = WEBKIT_WEB_VIEW(webkit_web_view_new());
  return scm_from_pointer(webView, NULL);
}

static void
load_modules(void) {
  scm_c_use_module("ice-9 threads");
  scm_c_use_module("ice-9 atomic");
}

static void
run_repl(void *data, int argc, char **argv) {
  load_modules();

  GAsyncQueue *message_qu = g_async_queue_new();

  scm_c_define_gsubr("launch-webkit-blocking", 1, 0, 0, launch_webkit);
  scm_c_define_gsubr("open-page-with-webview", 2, 0, 0, open_page);
  scm_c_define_gsubr("make-webview", 0, 0, 0, make_webview);

  scm_c_primitive_load("./browser.scm");

  scm_shell(argc, argv);
}

int main(int argc, char *argv[]) {
  /* Initialize Guile */
  scm_boot_guile(argc, argv, run_repl, 0);
  return 0;
}
