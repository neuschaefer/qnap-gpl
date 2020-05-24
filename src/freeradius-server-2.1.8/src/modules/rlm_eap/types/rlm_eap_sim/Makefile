#
# Makefile
#
# Version:	$Id: Makefile,v 1.1.1.1 2010/03/11 07:11:43 richardchang Exp $
#

TARGET      = rlm_eap_sim
SRCS        = rlm_eap_sim.c
HEADERS     =
RLM_CFLAGS  = $(INCLTDL) -I../.. -I../../libeap
RLM_LIBS    =  ../../libeap/$(LIBPREFIX)freeradius-eap.la
RLM_INSTALL =

RLM_DIR=../../
include ${RLM_DIR}../rules.mak

$(LT_OBJS): $(HEADERS)
