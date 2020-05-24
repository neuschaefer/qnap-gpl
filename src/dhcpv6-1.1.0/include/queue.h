/* from FreeBSD: src/sys/sys/queue.h,v 1.54 2002/08/05 05:18:43 alfred Exp */

/* Copyright (c) 1991, 1993
 * The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by the University of
 *    California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * @(#)queue.h 8.5 (Berkeley) 8/20/94
 * $FreeBSD: src/sys/sys/queue.h,v 1.54 2002/08/05 05:18:43 alfred Exp $
 */

#ifndef _QUEUE_H_
#define _QUEUE_H_

/*
 * Include queue.h in libc if available.
 */
#ifdef HAVE_SYS_QUEUE_H
#include <sys/queue.h>
#else

/*
 * Note: Even if queue.h is avaialble in libc, some macros may be not
 * available, so we need to check the availability of each macro.
 */

#if !defined(LIST_HEAD) || !defined(LIST_ENTRY) || \
    !defined(LIST_EMPTY) || !defined(LIST_FIRST) || !defined(LIST_FOREACH) || \
    !defined(LIST_INIT) || !defined(LIST_INSERT_AFTER) || \
    !defined(LIST_INSERT_BEFORE) || !defined(LIST_INSERT_HEAD) || \
    !defined(LIST_NEXT) || !defined(LIST_REMOVE)

/*
 * List declarations.
 */
#undef LIST_HEAD
#define LIST_HEAD(name, type) \
struct name { \
    struct type *lh_first; /* first element */ \
}

#undef LIST_ENTRY
#define LIST_ENTRY(type) \
struct { \
    struct type *le_next; /* next element */ \
    struct type **le_prev; /* address of previous next element */ \
}

/*
 * List functions.
 */
#undef LIST_EMPTY
#define LIST_EMPTY(head) ((head)->lh_first == NULL)

#undef LIST_FIRST
#define LIST_FIRST(head) ((head)->lh_first)

#undef LIST_FOREACH
#define LIST_FOREACH(var, head, field) \
    for ((var) = LIST_FIRST((head)); \
        (var); \
        (var) = LIST_NEXT((var), field))

#undef LIST_INIT
#define LIST_INIT(head) do { \
    LIST_FIRST((head)) = NULL; \
} while (0)

#undef LIST_INSERT_AFTER
#define LIST_INSERT_AFTER(listelm, elm, field) do { \
    if ((LIST_NEXT((elm), field) = LIST_NEXT((listelm), field)) != NULL) \
        LIST_NEXT((listelm), field)->field.le_prev = \
            &LIST_NEXT((elm), field); \
    LIST_NEXT((listelm), field) = (elm); \
    (elm)->field.le_prev = &LIST_NEXT((listelm), field); \
} while (0)

#undef LIST_INSERT_BEFORE
#define LIST_INSERT_BEFORE(listelm, elm, field) do { \
    (elm)->field.le_prev = (listelm)->field.le_prev; \
    LIST_NEXT((elm), field) = (listelm); \
    *(listelm)->field.le_prev = (elm); \
    (listelm)->field.le_prev = &LIST_NEXT((elm), field); \
} while (0)

#undef LIST_INSERT_HEAD
#define LIST_INSERT_HEAD(head, elm, field) do { \
    if ((LIST_NEXT((elm), field) = LIST_FIRST((head))) != NULL) \
        LIST_FIRST((head))->field.le_prev = &LIST_NEXT((elm), field); \
    LIST_FIRST((head)) = (elm); \
    (elm)->field.le_prev = &LIST_FIRST((head)); \
} while (0)

#undef LIST_NEXT
#define LIST_NEXT(elm, field) ((elm)->field.le_next)

#undef LIST_REMOVE
#define LIST_REMOVE(elm, field) do { \
    if (LIST_NEXT((elm), field) != NULL) \
        LIST_NEXT((elm), field)->field.le_prev = \
            (elm)->field.le_prev; \
    *(elm)->field.le_prev = LIST_NEXT((elm), field); \
} while (0)

#endif /* !defined(LIST_*) */

#if !defined(TAILQ_HEAD) || !defined(TAILQ_HEAD_INITIALIZER) || \
    !defined(TAILQ_ENTRY) || \
    !defined(TAILQ_CONCAT) || !defined(TAILQ_EMPTY) || \
    !defined(TAILQ_FIRST) || !defined(TAILQ_FOREACH) || \
    !defined(TAILQ_FOREACH_REVERSE) || !defined(TAILQ_INIT) || \
    !defined(TAILQ_INSERT_AFTER) || !defined(TAILQ_INSERT_BEFORE) || \
    !defined(TAILQ_INSERT_HEAD) || !defined(TAILQ_INSERT_TAIL) || \
    !defined(TAILQ_LAST) || !defined(TAILQ_NEXT) || !defined(TAILQ_PREV) || \
    !defined(TAILQ_REMOVE)

/*
 * Tail queue declarations.
 */
#undef TAILQ_HEAD
#define TAILQ_HEAD(name, type) \
struct name { \
    struct type *tqh_first; /* first element */ \
    struct type **tqh_last; /* addr of last next element */ \
}

#undef TAILQ_HEAD_INITIALIZER
#define TAILQ_HEAD_INITIALIZER(head) \
    { NULL, &(head).tqh_first }

#undef TAILQ_ENTRY
#define TAILQ_ENTRY(type) \
struct { \
    struct type *tqe_next; /* next element */ \
    struct type **tqe_prev; /* address of previous next element */ \
}

/*
 * Tail queue functions.
 */
#undef TAILQ_CONCAT
#define TAILQ_CONCAT(head1, head2, field) do { \
    if (!TAILQ_EMPTY(head2)) { \
        *(head1)->tqh_last = (head2)->tqh_first; \
        (head2)->tqh_first->field.tqe_prev = (head1)->tqh_last; \
        (head1)->tqh_last = (head2)->tqh_last; \
        TAILQ_INIT((head2)); \
    } \
} while (0)

#undef TAILQ_EMPTY
#define TAILQ_EMPTY(head) ((head)->tqh_first == NULL)

#undef TAILQ_FIRST
#define TAILQ_FIRST(head) ((head)->tqh_first)

#undef TAILQ_FOREACH
#define TAILQ_FOREACH(var, head, field) \
    for ((var) = TAILQ_FIRST((head)); \
        (var); \
        (var) = TAILQ_NEXT((var), field))

#undef TAILQ_FOREACH_REVERSE
#define TAILQ_FOREACH_REVERSE(var, head, headname, field) \
    for ((var) = TAILQ_LAST((head), headname); \
        (var); \
        (var) = TAILQ_PREV((var), headname, field))

#undef TAILQ_INIT
#define TAILQ_INIT(head) do { \
    TAILQ_FIRST((head)) = NULL; \
    (head)->tqh_last = &TAILQ_FIRST((head)); \
} while (0)

#undef TAILQ_INSERT_AFTER
#define TAILQ_INSERT_AFTER(head, listelm, elm, field) do { \
    if ((TAILQ_NEXT((elm), field) = TAILQ_NEXT((listelm), field)) != NULL) \
        TAILQ_NEXT((elm), field)->field.tqe_prev = \
            &TAILQ_NEXT((elm), field); \
    else { \
        (head)->tqh_last = &TAILQ_NEXT((elm), field); \
    } \
    TAILQ_NEXT((listelm), field) = (elm); \
    (elm)->field.tqe_prev = &TAILQ_NEXT((listelm), field); \
} while (0)

#undef TAILQ_INSERT_BEFORE
#define TAILQ_INSERT_BEFORE(listelm, elm, field) do { \
    (elm)->field.tqe_prev = (listelm)->field.tqe_prev; \
    TAILQ_NEXT((elm), field) = (listelm); \
    *(listelm)->field.tqe_prev = (elm); \
    (listelm)->field.tqe_prev = &TAILQ_NEXT((elm), field); \
} while (0)

#undef TAILQ_INSERT_HEAD
#define TAILQ_INSERT_HEAD(head, elm, field) do { \
    if ((TAILQ_NEXT((elm), field) = TAILQ_FIRST((head))) != NULL) \
        TAILQ_FIRST((head))->field.tqe_prev = \
            &TAILQ_NEXT((elm), field); \
    else \
        (head)->tqh_last = &TAILQ_NEXT((elm), field); \
    TAILQ_FIRST((head)) = (elm); \
    (elm)->field.tqe_prev = &TAILQ_FIRST((head)); \
} while (0)

#undef TAILQ_INSERT_TAIL
#define TAILQ_INSERT_TAIL(head, elm, field) do { \
    TAILQ_NEXT((elm), field) = NULL; \
    (elm)->field.tqe_prev = (head)->tqh_last; \
    *(head)->tqh_last = (elm); \
    (head)->tqh_last = &TAILQ_NEXT((elm), field); \
} while (0)

#undef TAILQ_LAST
#define TAILQ_LAST(head, headname) \
    (*(((struct headname *)((head)->tqh_last))->tqh_last))

#undef TAILQ_NEXT
#define TAILQ_NEXT(elm, field) ((elm)->field.tqe_next)

#undef TAILQ_PREV
#define TAILQ_PREV(elm, headname, field) \
    (*(((struct headname *)((elm)->field.tqe_prev))->tqh_last))

#undef TAILQ_REMOVE
#define TAILQ_REMOVE(head, elm, field) do { \
    if ((TAILQ_NEXT((elm), field)) != NULL) \
        TAILQ_NEXT((elm), field)->field.tqe_prev = \
            (elm)->field.tqe_prev; \
    else { \
        (head)->tqh_last = (elm)->field.tqe_prev; \
    } \
    *(elm)->field.tqe_prev = TAILQ_NEXT((elm), field); \
} while (0)

#endif /* !defined(TAILQ_*) */

#endif /* !defined(HAVE_SYS_QUEUE_H) */

#endif /* !defined(_QUEUE_H_) */
