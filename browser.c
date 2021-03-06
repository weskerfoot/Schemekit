#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <stdlib.h>
#include <libguile.h>
#include <libguile/strings.h>

/* Our includes */
#include "browser.h"
#include "scheme_functions.h"

static void
load_modules(void) {
  scm_c_use_module("ice-9 threads");
  scm_c_use_module("ice-9 atomic");
  scm_c_use_module("ice-9 hash-table");
}

static gboolean
messageEvent(void *data) {
  struct QueueData *qdata = data;
  struct BrowserMessage *msg = g_async_queue_timeout_pop(qdata->gtk_qu, 100);

  if (msg != NULL) {
    printf("%d\n", msg->event);
    printf("Got an event\n");
    switch (msg->event) {
      case LOAD:
        printf("Loading %s\n", (char*)msg->data);
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

static int
qu_push(enum BrowserEvent msg_type,
        char *message,
        GAsyncQueue *g_queue) {

  struct BrowserMessage *msg = malloc( sizeof (*msg) );

  msg->data = message;

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
  g_async_queue_push(g_queue, msg);
  return 1;
}

static int
conf_val(char * const key) {
  /* Lookup a key value in a Scheme hash-table */
  SCM config = scm_ref("config");
  SCM scm_key = scm_from_locale_string(key);
  SCM result = scm_hash_ref(config, scm_key, NULL);

  if (result != NULL) {
    return scm_to_int(result);
  }
  else {
    /* TODO check the key and then do an intelligent default? */
    /* For now, default to 1 = ON */
    return 1;
  }
}

static WebKitWebView*
make_webview() {
  WebKitSettings *settings = webkit_settings_new();
  WebKitHardwareAccelerationPolicy hw_policy;

  /* Disable hardware acceleration by default */
  /* It seems to be causing issues */
  if (conf_val("hw-acceleration")) {
    hw_policy = WEBKIT_HARDWARE_ACCELERATION_POLICY_ALWAYS;
  }
  else {
    hw_policy = WEBKIT_HARDWARE_ACCELERATION_POLICY_NEVER;
  }

  webkit_settings_set_hardware_acceleration_policy(settings, hw_policy);

  webkit_settings_set_enable_webgl(settings, conf_val("webgl"));
  webkit_settings_set_enable_accelerated_2d_canvas(settings, conf_val("2d-canvas"));
  webkit_settings_set_enable_write_console_messages_to_stdout(settings, conf_val("console-log"));
  webkit_settings_set_media_playback_requires_user_gesture(settings, conf_val("media-gestures"));

  webkit_settings_set_enable_media_stream(settings, conf_val("media-stream"));
  webkit_settings_set_enable_media_capabilities(settings, conf_val("media-capabilities"));
  webkit_settings_set_enable_mediasource(settings, conf_val("media-source"));

  webkit_settings_set_enable_dns_prefetching(settings, conf_val("dns-prefetching"));
  webkit_settings_set_enable_javascript(settings, conf_val("javascript"));
  webkit_settings_set_enable_page_cache(settings, conf_val("page-cache"));
  webkit_settings_set_enable_offline_web_application_cache(settings, conf_val("offline-web-app-cache"));
  webkit_settings_set_enable_developer_extras(settings, conf_val("dev-extras"));
  webkit_settings_set_draw_compositing_indicators(settings, conf_val("compositing-indicators"));

  webkit_settings_set_enable_smooth_scrolling(settings, conf_val("smooth-scrolling"));
  webkit_settings_set_enable_hyperlink_auditing(settings, conf_val("hyperlink-auditing"));
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

static void
webViewChanged(WebKitWebView *web_view,
               WebKitLoadEvent load_event,
               struct QueueData *qdata) {
    switch (load_event) {
      case WEBKIT_LOAD_STARTED:
      case WEBKIT_LOAD_REDIRECTED:
      case WEBKIT_LOAD_COMMITTED:
        /* The load is being performed. Current URI is
         * the final one and it won't change unless a new
         * load is requested or a navigation within the
         * same page is performed */
        break;
      case WEBKIT_LOAD_FINISHED:
        qu_push(LOAD, (char*)webkit_web_view_get_uri(web_view), qdata->guile_qu);
        break;
    }
}

SCM
launch_webkit(SCM scm_gtk_qu,
              SCM scm_guile_qu) {

  WebKitWebView *webView = make_webview();
  GAsyncQueue *gtk_qu = scm_to_pointer(scm_gtk_qu);
  GAsyncQueue *guile_qu = scm_to_pointer(scm_guile_qu);

  /* Get a default webkit context for modifying the cache policy */
  WebKitWebContext *webkit_ctx = webkit_web_context_get_default();

  WebKitProcessModel process_model = WEBKIT_PROCESS_MODEL_MULTIPLE_SECONDARY_PROCESSES;

  webkit_web_context_set_process_model(webkit_ctx,
                                       process_model);

  webkit_web_context_set_cache_model(webkit_ctx,
                                     WEBKIT_CACHE_MODEL_WEB_BROWSER);

  /* Initialize GTK+ */
  gtk_init(0, NULL);

  struct QueueData queue_data;

  /* Initialize the queue data */
  queue_data.gtk_qu = gtk_qu;
  queue_data.guile_qu = guile_qu;
  queue_data.webView = webView;

  g_idle_add(messageEvent, &queue_data);

  /* Create an 800x600 window that will contain the browser instance */
  GtkWidget *main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  gtk_window_set_default_size(GTK_WINDOW(main_window), 800, 600);

  /* Put the browser area into the main window */
  gtk_container_add(GTK_CONTAINER(main_window), GTK_WIDGET(webView));

  /* Set up callbacks so that if either the main window or the browser instance is */
  /* closed, the program will exit */
  g_signal_connect(main_window, "destroy", G_CALLBACK(destroyWindowCb), NULL);
  g_signal_connect(webView, "close", G_CALLBACK(closeWebViewCb), main_window);
  g_signal_connect(webView, "load-changed", G_CALLBACK(webViewChanged), &queue_data);

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
  GAsyncQueue *browser_event_qu = g_async_queue_new();
  GAsyncQueue *guile_event_qu = g_async_queue_new();

  /* Used for passing messages to the GTK thread */
  scm_c_define("gtk-qu", scm_from_pointer(browser_event_qu, NULL));

  /* Used for passing messages back to the Guile thread */
  scm_c_define("guile-qu", scm_from_pointer(guile_event_qu, NULL));

  scm_c_define_gsubr("launch-webkit-blocking", 2, 0, 0, launch_webkit);
  scm_c_define_gsubr("qu-push", 3, 0, 0, scm_qu_push);
  scm_c_define_gsubr("qu-pop", 1, 0, 0, scm_qu_pop);

  scm_c_primitive_load("./schemekit.scm");

  scm_shell(argc, argv);
}

int main(int argc, char *argv[]) {
  /* Set environment variables relevant to webgtk */
  /* Initialize Guile */
  scm_boot_guile(argc, argv, run_repl, 0);
  return 0;
}
