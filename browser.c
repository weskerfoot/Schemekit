#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <stdlib.h>
#include <libguile.h>
#include <libguile/strings.h>

static void
load_modules(void) {
  scm_c_use_module("ice-9 threads");
  scm_c_use_module("ice-9 atomic");
}

struct QueueData {
  GAsyncQueue *queue;
  WebKitWebView *webView;
};

struct BrowserMessage {
  enum BrowserEvent {
    LOAD = 0,
    CLOSE = 1,
    EMPTY = 2
  } event;
  void *data;
};

static gboolean
eventCallback(void *data) {
  struct QueueData *qdata = data;

  struct BrowserMessage *msg = g_async_queue_timeout_pop(qdata->queue, 20);
  if (msg != NULL) {
    switch (msg->event) {
      case LOAD:
        printf("Got a load event\n");
        printf(msg->data);
        webkit_web_view_load_uri(qdata->webView, msg->data);
        break;
      case CLOSE:
        printf("Got a close event\n");
        break;
      default:
        printf("Got an unknown event\n");
        break;
    }
    free(msg);
  }
  return TRUE;
}

static SCM
qu_push(SCM scm_msg_type,
        SCM scm_message,
        SCM scm_qu) {

  struct BrowserMessage *msg = malloc( sizeof(*msg) );

  int msg_type = scm_to_int(scm_msg_type);

  msg->data = scm_to_locale_string(scm_message);
  switch (msg_type) {
    case LOAD:
      msg->event = LOAD;
      break;
    case CLOSE:
      msg->event = CLOSE;
      break;
    default:
      msg->event = EMPTY;
      break;
  }

  GAsyncQueue *g_queue = scm_to_pointer(scm_qu);
  g_async_queue_push(g_queue, msg);

  return SCM_BOOL_T;
}

static WebKitWebView*
make_webview() {
  WebKitSettings *settings = webkit_settings_new();

  WebKitHardwareAccelerationPolicy hw_policy = WEBKIT_HARDWARE_ACCELERATION_POLICY_ALWAYS;
  webkit_settings_set_hardware_acceleration_policy(settings,
                                                   hw_policy);

  webkit_settings_set_enable_webgl(settings, TRUE);
  webkit_settings_set_enable_accelerated_2d_canvas(settings, TRUE);
  webkit_settings_set_enable_write_console_messages_to_stdout(settings, TRUE);
  webkit_settings_set_media_playback_requires_user_gesture(settings, TRUE);
  webkit_settings_set_media_playback_requires_user_gesture(settings, TRUE);
  webkit_settings_set_enable_encrypted_media(settings, TRUE);
  webkit_settings_set_enable_media_capabilities(settings, TRUE);
  webkit_settings_set_enable_smooth_scrolling(settings, TRUE);
  webkit_settings_set_enable_dns_prefetching(settings, TRUE);

  webkit_settings_set_enable_hyperlink_auditing(settings, FALSE);
  webkit_settings_set_enable_java(settings, FALSE);

  return WEBKIT_WEB_VIEW(webkit_web_view_new_with_settings(settings));
}

/* GTK callbacks */

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
launch_webkit(SCM qu) {
  WebKitWebView *webView = make_webview();
  GAsyncQueue *g_queue = scm_to_pointer(qu);

  /* Get a default webkit context for modifying the cache policy */
  WebKitWebContext *webkit_ctx = webkit_web_context_get_default();

  webkit_web_context_set_cache_model(webkit_ctx,
                                     WEBKIT_CACHE_MODEL_DOCUMENT_VIEWER);

  /* Initialize GTK+ */
  gtk_init(0, NULL);

  struct QueueData queue_data;

  queue_data.queue = g_queue;
  queue_data.webView = webView;

  g_idle_add(eventCallback, &queue_data);

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

static void
run_repl(void *data, int argc, char **argv) {
  load_modules();

  GAsyncQueue *message_qu = g_async_queue_new();

  scm_c_define("message-qu", scm_from_pointer(message_qu, NULL));

  scm_c_define_gsubr("launch-webkit-blocking", 1, 0, 0, launch_webkit);
  scm_c_define_gsubr("qu-push", 3, 0, 0, qu_push);

  scm_c_primitive_load("./browser.scm");

  scm_shell(argc, argv);
}

int main(int argc, char *argv[]) {
  /* Set environment variables relevant to webgtk */
  setenv("LIBGL_DRI3_DISABLE", "1", -1);
  /* Initialize Guile */
  scm_boot_guile(argc, argv, run_repl, 0);
  return 0;
}
