#include <assert.h>
#include <stdio.h>

#include "core.h"

typedef struct MailboxEntry {
  Message value;
  struct MailboxEntry *next;
} MailboxEntry;


static MailboxEntry *alloc_entry(Message message) {
  MailboxEntry *entry = calloc(1, sizeof(MailboxEntry));
  entry->value = message;
  return entry;
}

void free_entry(MailboxEntry *entry) {
  free(entry);
}

void mailbox_push(Mailbox *mailbox, Message message) {
  assert(pthread_mutex_lock(&mailbox->lock) == 0);

  if (mailbox->front) {
    mailbox->back->next = alloc_entry(message);
    mailbox->back = mailbox->back->next;
  } else {
    mailbox->front = alloc_entry(message);
    mailbox->back = mailbox->front;
  }

  assert(pthread_mutex_unlock(&mailbox->lock) == 0);
}

bool mailbox_pop(Mailbox *mailbox, Message *out) {
  assert(pthread_mutex_lock(&mailbox->lock) == 0);

  if (!mailbox->front) {
    return false;
  }

  MailboxEntry *old_front = mailbox->front;
  *out = old_front->value;
  mailbox->front = mailbox->front->next;

  assert(pthread_mutex_unlock(&mailbox->lock) == 0);

  free_entry(old_front);
  return true;
}

void mailbox_init(Mailbox *mailbox) {
  if (pthread_mutex_init(&mailbox->lock, NULL) != 0) {
    perror("pthread_mutex_init");
    exit(1);
  }
  mailbox->front = NULL;
  mailbox->back = NULL;
}
