/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include <memory.h> /* for memset */
#include "sm_inter.h"
#include <string.h>
#define SM_INTER_X

/* Default timeout can be changed using clnt_control() */
static struct timeval TIMEOUT = { 25, 0 };

struct sm_stat_res *
sm_stat_1(struct sm_name *argp, CLIENT *clnt)
{
	static struct sm_stat_res clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, SM_STAT,
		(xdrproc_t) xdr_sm_name, (caddr_t) argp,
		(xdrproc_t) xdr_sm_stat_res, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

struct sm_stat_res *
sm_mon_1(struct mon *argp, CLIENT *clnt)
{
	static struct sm_stat_res clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, SM_MON,
		(xdrproc_t) xdr_mon, (caddr_t) argp,
		(xdrproc_t) xdr_sm_stat_res, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

struct sm_stat *
sm_unmon_1(struct mon_id *argp, CLIENT *clnt)
{
	static struct sm_stat clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, SM_UNMON,
		(xdrproc_t) xdr_mon_id, (caddr_t) argp,
		(xdrproc_t) xdr_sm_stat, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

struct sm_stat *
sm_unmon_all_1(struct my_id *argp, CLIENT *clnt)
{
	static struct sm_stat clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, SM_UNMON_ALL,
		(xdrproc_t) xdr_my_id, (caddr_t) argp,
		(xdrproc_t) xdr_sm_stat, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

void *
sm_simu_crash_1(void *argp, CLIENT *clnt)
{
	static char clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, SM_SIMU_CRASH,
		(xdrproc_t) xdr_void, (caddr_t) argp,
		(xdrproc_t) xdr_void, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return ((void *)&clnt_res);
}

void *
sm_notify_1(struct stat_chge *argp, CLIENT *clnt)
{
	static char clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, SM_NOTIFY,
		(xdrproc_t) xdr_stat_chge, (caddr_t) argp,
		(xdrproc_t) xdr_void, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return ((void *)&clnt_res);
}