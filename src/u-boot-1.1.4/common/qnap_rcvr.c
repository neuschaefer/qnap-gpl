#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <qnap.h>

#define mdelay(n) udelay((n) * 1000)

#define ST_RCVR_MODE        0x01
#define ST_DHCP_FAIL        0x02
#define ST_TFTP_FAIL        0x04
#define ST_RCVR_FAIL        0x08
#define ST_RCVR_SUCCESS     0x10

static void rcvr_status_warn(int status)
{
    if(status & (ST_RCVR_MODE | ST_RCVR_SUCCESS)) {
		/* set status red on */
		mvUartPutc(1,0x57);
		/* beep two short buzzer */
		mvUartPutc(1,0x50);
		mdelay(1000);
		mvUartPutc(1,0x50);
        printf("=> %s\n", 
               status & ST_RCVR_MODE ? "Enter Recovery Mode" : 
                                       "Going to restart ...");
    }

    if(status & (ST_DHCP_FAIL | ST_TFTP_FAIL | ST_RCVR_FAIL)) {
        /* set status red flash */
		mvUartPutc(1,0x54);
        /* beep one long buzzer */
		mvUartPutc(1,0x51);
		mdelay(1000);
        if(status & ST_TFTP_FAIL) {
            /* beep one long buzzer */
		    mvUartPutc(1,0x51);
		    mdelay(1000);
        }
        if(status & ST_TFTP_FAIL) {
            /* beep one long buzzer */
		    mvUartPutc(1,0x51);
		    mdelay(1000);
        }
    }
}

int QNAP_do_recovery()
{
    char *s = NULL,
         *tftpcmd = NULL,
         *updcmd = NULL;
    int rc = 0;

    rcvr_status_warn(ST_RCVR_MODE);

    s = "dhcp";
    rc = run_command(s, 0);

    if(rc > 0) {
        tftpcmd = "tftpboot 0x800000 ${bootfile}";
        if((rc = run_command(tftpcmd, 0)) > 0) {
            updcmd = "protect off bank 1; erase 0xf8200000 0xf8ffffff; cp.b 0xa00000 0xf8200000 e00000; protect on bank 1";
            rc = run_command(updcmd, 0);
            if(rc > 0) {
                printf("Recovery done successfully: %d\n", rc);
                rcvr_status_warn(ST_RCVR_SUCCESS);
                return 0;
            } else {
                printf("Recovery done error: %d\n", rc);
                rcvr_status_warn(ST_RCVR_FAIL);
                return -1;
            }
        } else {
            printf("Tftp loading failed: %d\n", rc);
            rcvr_status_warn(ST_TFTP_FAIL);
            return -1;
        }
    } else {
        printf("DHCP/BOOTP Failed, rc: %d\n", rc);
        rcvr_status_warn(ST_DHCP_FAIL);
        return -1;
    }
    return -1;
}

