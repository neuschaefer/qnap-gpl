#ifndef _QNAP_LIBISCSI_H
#define _QNAP_LIBISCSI_H

#include <linux/un.h>
#include <linux/kthread.h>

#include "../iscsi_tcp.h"

extern struct task_struct *qnap_task;
extern spinlock_t qnap_event__lock;
extern struct list_head qnap_event_list;

typedef struct {
	struct list_head list;
	int event;
	char iqn[256];
} QNAP_EVENT;

#define INADDR_SEND INADDR_LOOPBACK
#define QNAP_NOTIFY_NAMESPACE "QNAP_NOTIFY_NAMESPACE"

int qnap_vjbod_is_vjbod_session(struct iscsi_session *session);
int qnap_vjbod_iscsi_has_ping_timed_out(struct iscsi_conn *conn);

void add_qnap_event(int event, const char *iqn);
void init_qnap_event_thread(void);
void stop_qnap_event_thread(void);

#endif /* _QNAP_LIBISCSI_H */
