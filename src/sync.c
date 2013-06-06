#include "sync.h"
#include "http.h"
	
void request_phone_state() {
  Tuplet value = TupletInteger(REQUEST_DATA_KEY, REQUEST_UNREAD | REQUEST_MISSED);
  DictionaryIterator *iter;
  app_message_out_get(&iter);
  if (iter == NULL) return;

  dict_write_tuplet(iter, &value);
  dict_write_end(iter);
  
  app_message_out_send();
  app_message_out_release();
}
