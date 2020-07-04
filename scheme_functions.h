static SCM
scm_qu_pop(SCM scm_qu) {
  GAsyncQueue *g_queue = scm_to_pointer(scm_qu);

  struct BrowserMessage *msg = g_async_queue_timeout_pop(g_queue, 10);

  if (msg != NULL) {
    return scm_from_locale_string(msg->data);
  }
  else {
    return SCM_BOOL_F;
  }
}

static SCM
scm_qu_push(SCM scm_msg_type,
            SCM scm_message,
            SCM scm_qu) {

  enum BrowserEvent msg_type = scm_to_int(scm_msg_type);
  char *data = scm_to_locale_string(scm_message);
  GAsyncQueue *g_queue = scm_to_pointer(scm_qu);

  qu_push(msg_type, data, g_queue);

  return SCM_BOOL_T;
}

static SCM
scm_ref(const char *var_name) {
  /* Lookup and de-reference a Scheme value */
  return scm_variable_ref(scm_c_lookup(var_name));
}
