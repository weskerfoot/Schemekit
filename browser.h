struct QueueData {
  GAsyncQueue *gtk_qu;
  GAsyncQueue *guile_qu;
  WebKitWebView *webView;
};

enum BrowserEvent {
  LOAD = 0,
  CLOSE = 1,
  EMPTY = 2
};

struct BrowserMessage {
  enum BrowserEvent event;
  void *data;
};



static SCM scm_ref(const char *);
static int read_config_val(char * const);
static SCM qu_push(enum BrowserEvent, char *, GAsyncQueue *);


