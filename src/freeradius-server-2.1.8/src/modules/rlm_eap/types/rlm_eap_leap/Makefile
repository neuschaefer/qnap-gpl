#
# Makefile
#
# Version:	$Id: Makefile,v 1.1.1.1 2010/03/11 07:11:43 richardchang Exp $
#

TARGET      = rlm_eap_leap
SRCS        = rlm_eap_leap.c eap_leap.c smbdes.c
RLM_CFLAGS  = $(INCLTDL) -I../.. -I../../libeap
HEADERS     = eap_leap.h  ../../eap.h ../../rlm_eap.h
RLM_INSTALL =

RLM_DIR=../../
include ${RLM_DIR}../rules.mak

$(LT_OBJS): $(HEADERS)
