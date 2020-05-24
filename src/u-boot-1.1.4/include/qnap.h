#ifndef __QNAP_H__
#define __QNAP_H__

/******************************************************
 * Richard Chen 20081217, 
 * QNAP for Recovery Button
 *****************************************************/
uint32_t QNAP_recovery_detect(void);
int      QNAP_recovery_init(void);
int      QNAP_do_recovery(void);
#endif /* __QNAP_H__ */
