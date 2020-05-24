#
# Makefile
#
# Version:	$Id: Makefile,v 1.1.1.1 2010/03/11 07:11:43 richardchang Exp $
#

TARGET      = rlm_eap_gtc
SRCS        = rlm_eap_gtc.c
RLM_CFLAGS  = $(INCLTDL) -I../.. -I../../libeap
HEADERS     =  ../../rlm_eap.h ../../eap.h
RLM_INSTALL =

RLM_DIR=../../
include ${RLM_DIR}../rules.mak

$(LT_OBJS): $(HEADERS)
