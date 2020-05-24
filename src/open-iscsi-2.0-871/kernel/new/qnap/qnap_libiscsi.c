/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/types.h>
#include <linux/slab.h>
#include <net/tcp.h>
#include <scsi/iscsi_proto.h>
#include <scsi/scsi_host.h>
#include <scsi/libiscsi.h>

#include "qnap_libiscsi.h"

struct task_struct *qnap_task = NULL;
spinlock_t qnap_event__lock;
struct list_head qnap_event_list;

int ksocket_send(struct socket *sock, struct sockaddr_in *addr, unsigned char *buf, int len)
{
    struct msghdr msg; 
    struct iovec iov;
    mm_segment_t oldfs;      
    int size = 0;
    if (sock->sk==NULL)
        return 0;
    iov.iov_base = buf;
    iov.iov_len = len;
    msg.msg_flags = 0;
    msg.msg_name = NULL;	//addr;
    msg.msg_namelen  = 0;	//sizeof(struct sockaddr_in);
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    msg.msg_control = NULL;
    iov_iter_init(&msg.msg_iter, WRITE, (struct iovec *)&iov, 1, len);

    oldfs = get_fs();
    set_fs(KERNEL_DS);
    size = sock_sendmsg(sock,&msg);
    set_fs(oldfs);
    return size;
}

static int NotifyQEvent(int event_type, const char *target_name)
{
	if (!target_name) target_name = "NULL";
	printk("%s(%d, %s)\n", __FUNCTION__, event_type, target_name);
        int retval;
        struct socket *sock;
        retval = sock_create(PF_UNIX, SOCK_STREAM, 0, &sock);
	printk("%s::sock_create ret %d\n", __FUNCTION__, retval);
        if (retval < 0)
                goto out;

        struct sockaddr_un addr_send;
        memset(&addr_send, 0, sizeof(addr_send));
        addr_send.sun_family = AF_UNIX;
	memcpy((char *) &addr_send.sun_path + 1, QNAP_NOTIFY_NAMESPACE, strlen(QNAP_NOTIFY_NAMESPACE));
	if (!sock->ops || !sock->ops->connect) {
		printk("sock ops or connect is NULL\n");
		goto out_release;
	}
        if ((retval = sock->ops->connect(sock, (struct sockaddr *)&addr_send, sizeof(addr_send), 0)) < 0) {
		printk("%s::connect failed %d\n", __FUNCTION__, retval);
                goto out_release;
        }
		
        char buf[512];
	snprintf(buf, sizeof(buf), "INIT_EVT:%d;%s", event_type, target_name);
	int rc = ksocket_send(sock, (struct sockaddr *)&addr_send, buf, strlen(buf)+1);
	printk("%s::ksocket_send(%s) ret %d\n", __FUNCTION__, buf, rc);

out_release:
        sock_release(sock);
        return retval;
out:
        return retval;
}

static QNAP_EVENT * get_ready_event(void)
{
	QNAP_EVENT *evnt = NULL;

	spin_lock(&qnap_event__lock);
	if (!list_empty(&qnap_event_list)) {
		evnt = list_entry(qnap_event_list.next, QNAP_EVENT, list);
		list_del_init(&evnt->list);
	}
	spin_unlock(&qnap_event__lock);

	return evnt;
}

static int qnap_event_thread(void *arg)
{
	QNAP_EVENT *event;
	__set_current_state(TASK_RUNNING);
	do {
		while (!list_empty(&qnap_event_list) &&
		       (event = get_ready_event())) {
		       NotifyQEvent(event->event, event->iqn);
			   kfree(event);
		}
		__set_current_state(TASK_INTERRUPTIBLE);
		if (list_empty(&qnap_event_list))
			schedule();

		__set_current_state(TASK_RUNNING);
	} while (!kthread_should_stop());
	return 0;
}

void add_qnap_event(int event, const char *iqn)
{
	QNAP_EVENT *pEvent = kmalloc(sizeof(QNAP_EVENT), GFP_ATOMIC);
	pEvent->event = event;
	strcpy(pEvent->iqn, iqn);
	spin_lock(&qnap_event__lock);
	list_add_tail(&pEvent->list, &qnap_event_list);
	spin_unlock(&qnap_event__lock);
	wake_up_process(qnap_task);
}
EXPORT_SYMBOL_GPL(add_qnap_event);

void init_qnap_event_thread(void)
{
	spin_lock_init(&qnap_event__lock);

	INIT_LIST_HEAD(&qnap_event_list);
	
	qnap_task = kthread_create(qnap_event_thread, NULL, "qnap_et");
	if (IS_ERR(qnap_task)) {
		return ;
	}

	wake_up_process(qnap_task);
}
EXPORT_SYMBOL_GPL(init_qnap_event_thread);

void stop_qnap_event_thread(void)
{
	if (qnap_task) {
		kthread_stop(qnap_task);
		qnap_task = NULL;
	}
}
EXPORT_SYMBOL_GPL(stop_qnap_event_thread);

int qnap_vjbod_is_vjbod_session(
	struct iscsi_session *session
	)
{
	if (session && session->targetname) {
		if (session->host && (session->host->skip_probe_sd == 1))
			return 0;
	}
	return -ENODEV;
}
EXPORT_SYMBOL_GPL(qnap_vjbod_is_vjbod_session);

/* this function will replace original one since we will retry ping-timeout event */
int qnap_vjbod_iscsi_has_ping_timed_out(struct iscsi_conn *conn)
{
	int val = (1 + conn->qnap_ping_retry);

	if (conn->ping_task &&
	    time_before_eq(conn->last_recv + (conn->recv_timeout * HZ) +
			   (conn->ping_timeout * HZ * val), jiffies))
				return 1;
	else
		return 0;
}
EXPORT_SYMBOL_GPL(qnap_vjbod_iscsi_has_ping_timed_out);
