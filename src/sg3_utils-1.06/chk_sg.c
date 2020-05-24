#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "sg_include.h"
#include "sg_err.h"

/* A utility program for the Linux OS SCSI generic ("sg") device driver.
 * This program should be used with the sg3_utils package.
 * Author: Andy Wu
*/


/* #define SG_DEBUG */

#define SENSE_BUFF_LEN 32       /* Arbitrary, could be larger */
#define DEF_TIMEOUT 60000       /* 60,000 millisecs == 60 seconds */

#define INQUIRY_CMD     0x12
#define INQUIRY_CMDLEN  6
#define MX_ALLOC_LEN 255

#define EBUFF_SZ 256

#ifndef SCSI_IOCTL_GET_PCI
#define SCSI_IOCTL_GET_PCI 0x5387
#endif



#define ME "chk_sg: "

#define READCAP_TIMEOUT 60000   /* 60,000 milliseconds == 1 minute */
#define SENSE_BUFF_SZ 32
#define RCAP_REPLY_LEN 8
#define RCAP16_REPLY_LEN 12

#ifndef SERVICE_ACTION_IN
#define SERVICE_ACTION_IN     0x9e
#endif
#ifndef SAI_READ_CAPACITY_16
#define SAI_READ_CAPACITY_16  0x10
#endif

#define EBUFF_SZ 256



/* Returns 0 when successful, else -1 */
static int do_inq(int sg_fd, int cmddt, int evpd, unsigned int pg_op, 
                  void * resp, int mx_resp_len, int noisy)
{
    int res;
    unsigned char inqCmdBlk[INQUIRY_CMDLEN] = {INQUIRY_CMD, 0, 0, 0, 0, 0};
    unsigned char sense_b[SENSE_BUFF_LEN];
    struct sg_io_hdr io_hdr;

    if (cmddt)
        inqCmdBlk[1] |= 2;
    if (evpd)
        inqCmdBlk[1] |= 1;
    inqCmdBlk[2] = (unsigned char)pg_op;
    inqCmdBlk[4] = (unsigned char)mx_resp_len;
    memset(&io_hdr, 0, sizeof(struct sg_io_hdr));
    io_hdr.interface_id = 'S';
    io_hdr.cmd_len = sizeof(inqCmdBlk);
    io_hdr.mx_sb_len = sizeof(sense_b);
    io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;
    io_hdr.dxfer_len = mx_resp_len;
    io_hdr.dxferp = resp;
    io_hdr.cmdp = inqCmdBlk;
    io_hdr.sbp = sense_b;
    io_hdr.timeout = DEF_TIMEOUT;

    if (ioctl(sg_fd, SG_IO, &io_hdr) < 0) {
        perror("SG_IO (inquiry) error");
        return -1;
    }
    res = sg_err_category3(&io_hdr);
    switch (res) {
    case SG_ERR_CAT_CLEAN:
    case SG_ERR_CAT_RECOVERED:
        return 0;
    default:
        if (noisy) {
            char ebuff[EBUFF_SZ];
            snprintf(ebuff, EBUFF_SZ, "Inquiry error, CmdDt=%d, "
                     "EVPD=%d, page_opcode=%x ", cmddt, evpd, pg_op);
            sg_chk_n_print3(ebuff, &io_hdr);
        }
        return -1;
    }
}

static void dStrRaw(const char* str, int len)
{
    int i;
    
    for (i = 0 ; i < len; i++) {
        printf("%c", str[i]);
    }
}

static void dStrHex(const char* str, int len)
{
    const char* p = str;
    unsigned char c;
    char buff[82];
    int a = 0;
    const int bpstart = 5;
    const int cpstart = 60;
    int cpos = cpstart;
    int bpos = bpstart;
    int i, k;
    
    if (len <= 0) return;
    memset(buff,' ',80);
    buff[80]='\0';
    k = sprintf(buff + 1, "%.2x", a);
    buff[k + 1] = ' ';
    if (bpos >= ((bpstart + (9 * 3))))
        bpos++;

    for(i = 0; i < len; i++)
    {
        c = *p++;
        bpos += 3;
        if (bpos == (bpstart + (9 * 3)))
            bpos++;
        sprintf(&buff[bpos], "%.2x", (int)(unsigned char)c);
        buff[bpos + 2] = ' ';
        if ((c < ' ') || (c >= 0x7f))
            c='.';
        buff[cpos++] = c;
        if (cpos > (cpstart+15))
        {
            printf("%s\n", buff);
            bpos = bpstart;
            cpos = cpstart;
            a += 16;
            memset(buff,' ',80);
            k = sprintf(buff + 1, "%.2x", a);
            buff[k + 1] = ' ';
        }
    }
    if (cpos > cpstart)
    {
        printf("%s\n", buff);
    }
}

static const char * scsi_ptype_strs[] = {
    /* 0 */ "disk",
    "tape",
    "printer",
    "processor",
    "write once optical disk",
    /* 5 */ "cd/dvd",
    "scanner",
    "optical memory device",
    "medium changer",
    "communications",
    /* 0xa */ "graphics",
    "graphics",
    "storage array controller",
    "enclosure services device",
    "simplified direct access device",
    "optical card reader/writer device",
    /* 0x10 */ "bridging expander",
    "object based storage",
    "automation/driver interface",
};

const char * get_ptype_str(int scsi_ptype)
{
    int num = sizeof(scsi_ptype_strs) / sizeof(scsi_ptype_strs[0]);

    if (0x1f == scsi_ptype)
	return "no physical device on this lu";
    else if (0x1e == scsi_ptype)
	return "well known logical unit";
    else
        return (scsi_ptype < num) ? scsi_ptype_strs[scsi_ptype] : "";
}



int sg_inq(char *file_name)
{
    int sg_fd, k, j, num, len, act_len;
    int support_num;
    char ebuff[EBUFF_SZ];
    char buff[MX_ALLOC_LEN + 1];
    unsigned char rsp_buff[MX_ALLOC_LEN + 1];
    unsigned int num_opcode = 0;
    int do_evpd = 0;
    int do_cmddt = 0;
    int do_cmdlst = 0;
    int do_hex = 0;
    int do_raw = 0;
    int do_pci = 0;
    int do_36 = 0;
    int oflags = O_RDONLY | O_NONBLOCK;
    int ansi_version = 0;
    int ret = 0;

    if (do_raw && do_hex) {
        fprintf(stderr, "Can't do hex and raw at the same time\n");
        file_name = 0;
    }
    
    if (do_pci)
        oflags = O_RDWR | O_NONBLOCK;
    if ((sg_fd = open(file_name, oflags)) < 0) {
        snprintf(ebuff, EBUFF_SZ, "sg_inq: error opening file: %s", file_name);
        perror(ebuff);
        return 1;
    }
    /* Just to be safe, check we have a new sg device by trying an ioctl */
    if ((ioctl(sg_fd, SG_GET_VERSION_NUM, &k) < 0) || (k < 30000)) {
        fprintf(stderr,
                "sg_inq: %s doesn't seem to be a version 3 sg device\n",
                file_name);
        close(sg_fd);
        return 1;
    }
    memset(rsp_buff, 0, MX_ALLOC_LEN + 1);

    if (! (do_cmddt || do_evpd)) {
        /*
	if (!do_raw)
            printf("standard INQUIRY:\n");
        if (num_opcode > 0)
            printf(" <<given opcode or page_code is being ignored>>\n");
        */
        if (0 == do_inq(sg_fd, 0, 0, 0, rsp_buff, 36, 1)) {
            len = rsp_buff[4] + 5;
            ansi_version = rsp_buff[2] & 0x7;
            if ((len > 36) && (len < 256) && (! do_36)) {
                if (do_inq(sg_fd, 0, 0, 0, rsp_buff, len, 1)) {
                    fprintf(stderr, "second INQUIRY (%d byte) failed\n", len);
                    return 1;
                }
                if (len != (rsp_buff[4] + 5)) {
                    fprintf(stderr,
                            "strange, twin INQUIRYs yield different "
                            "'additional length'\n");
                    ret = 2;
                }
            }
            if (do_36) {
                act_len = len;
                len = 36;
            }
            else
                act_len = len;
            if (do_hex)
                dStrHex((const char *)rsp_buff, len);
            else if (do_raw)
                dStrRaw((const char *)rsp_buff, len);
            else {
		/*
                printf("  PQual=%d, Device_type=%d, RMB=%d, [ANSI version=%d], ",
                       (rsp_buff[0] & 0xe0) >> 5, rsp_buff[0] & 0x1f,
                       !!(rsp_buff[1] & 0x80), ansi_version);
                printf("version=0x%02x\n", (unsigned int)rsp_buff[2]);
                printf("  [AERC=%d], [TrmTsk=%d], NormACA=%d, HiSUP=%d, "
                       "Resp data format=%d\n  SCCS=%d, ",
                       !!(rsp_buff[3] & 0x80), !!(rsp_buff[3] & 0x40),
                       !!(rsp_buff[3] & 0x20), !!(rsp_buff[3] & 0x10),
                       rsp_buff[3] & 0x0f, !!(rsp_buff[5] & 0x80));
                printf("ACC=%d, ALUA=%d, 3PC=%d\n",
                       !!(rsp_buff[5] & 0x40), ((rsp_buff[5] & 0x30) >> 4),
		       !!(rsp_buff[5] & 0x08));
                printf("  BQue=%d, EncServ=%d, MultiP=%d, MChngr=%d, "
                       "[ACKREQQ=%d], ",
                       !!(rsp_buff[6] & 0x80), !!(rsp_buff[6] & 0x40), 
                       !!(rsp_buff[6] & 0x10), !!(rsp_buff[6] & 0x08), 
                       !!(rsp_buff[6] & 0x04));
                printf("Addr16=%d\n  RelAdr=%d, ",
                       !!(rsp_buff[6] & 0x01),
                       !!(rsp_buff[7] & 0x80));
                printf("WBus16=%d, Sync=%d, Linked=%d, [TranDis=%d], ",
                       !!(rsp_buff[7] & 0x20), !!(rsp_buff[7] & 0x10),
                       !!(rsp_buff[7] & 0x08), !!(rsp_buff[7] & 0x04));
                printf("CmdQue=%d\n", !!(rsp_buff[7] & 0x02));
                if (len > 56)
                    printf("  Clocking=0x%x, QAS=%d, IUS=%d\n",
                           (rsp_buff[56] & 0x0c) >> 2, !!(rsp_buff[56] & 0x2),
                           !!(rsp_buff[56] & 0x1));
                if (act_len == len)
                    printf("    length=%d (0x%x)", len, len);
                else
                    printf("    length=%d (0x%x), but only read 36 bytes", 
                           len, len);
                if ((ansi_version >= 2) && (len < 36))
                    printf("  [for SCSI>=2, len>=36 is expected]");
                cp = get_ptype_str(rsp_buff[0] & 0x1f);
                printf("   Peripheral device type: %s\n", cp);
		*/

                if (len <= 8)
                    ;/*
			printf(" Inquiry response length=%d\n, no vendor, "
                           "product or revision data\n", len);
			   */
                else {
                    if (len < 36)
                        rsp_buff[len] = '\0';
                    memcpy(buff, &rsp_buff[8], 8);
                    buff[8] = '\0';
		    
                    //printf(" Vendor identification: %s\n", buff);
                    printf( "Vendor name : %s", buff);

                    if (len <= 16)
                        ;//printf(" Product identification: <none>\n");
                    else {
                        memcpy(buff, &rsp_buff[16], 16);
                        buff[16] = '\0';
                        //printf(" Product identification: %s\n", buff);
                        printf(" %s", buff);
                    }
                    if (len <= 32)
                        ;//printf(" Product revision level: <none>\n");
                    else {
                        memcpy(buff, &rsp_buff[32], 4);
                        buff[4] = '\0';
                        printf(" %s", buff);
                    }
                }
            }
            /*if (!do_raw &&
                (0 == do_inq(sg_fd, 0, 1, 0x80, rsp_buff, MX_ALLOC_LEN, 0))) {
                len = rsp_buff[3];
                if (len > 0) {
                    memcpy(buff, rsp_buff + 4, len);
                    buff[len] = '\0';
                    //printf(" Product serial number: %s\n", buff);
                    printf(" ##%s##", buff);
                }
            }*/
            printf("\n");
        }
        else {
            printf("36 byte INQUIRY failed\n");
            return 1;
        }
    }
    else if (do_cmddt) {
        int reserved_cmddt;
        char op_name[128];

        if (do_cmdlst) {
            printf("Supported command list:\n");
            for (k = 0; k < 256; ++k) {
                if (0 == do_inq(sg_fd, 1, 0, k, rsp_buff, MX_ALLOC_LEN, 1)) {
                    support_num = rsp_buff[1] & 7;
                    reserved_cmddt = rsp_buff[4];
                    if ((3 == support_num) || (5 == support_num)) {
                        num = rsp_buff[5];
                        for (j = 0; j < num; ++j)
                            printf(" %.2x", (int)rsp_buff[6 + j]);
                        if (5 == support_num)
                            printf("  [vendor specific manner (5)]");
                        sg_get_command_name((unsigned char)k, 
                                            sizeof(op_name) - 1, op_name);
                        op_name[sizeof(op_name) - 1] = '\0';
                        printf("  %s\n", op_name);
                    }
                    else if ((4 == support_num) || (6 == support_num))
                        printf("  opcode=0x%.2x vendor specific (%d)\n",
                               k, support_num);
                    else if ((0 == support_num) && (reserved_cmddt > 0)) {
                        printf("  opcode=0x%.2x ignored cmddt bit, "
                               "given standard INQUIRY response, stop\n", k);
                        break;
                    }
                }
                else {
                    fprintf(stderr,
                            "CmdDt INQUIRY on opcode=0x%.2x: failed\n", k);
                    break;
                }
            }
        }
        else {
            if (! do_raw) {
                printf("CmdDt INQUIRY, opcode=0x%.2x:  [", num_opcode);
                sg_get_command_name((unsigned char)num_opcode, 
                                    sizeof(op_name) - 1, op_name);
                op_name[sizeof(op_name) - 1] = '\0';
                printf("%s]\n", op_name);
            }
            if (0 == do_inq(sg_fd, 1, 0, num_opcode, rsp_buff, 
                            MX_ALLOC_LEN, 1)) {
                len = rsp_buff[5] + 6;
                reserved_cmddt = rsp_buff[4];
                if (do_hex)
                    dStrHex((const char *)rsp_buff, len);
                else if (do_raw)
                    dStrRaw((const char *)rsp_buff, len);
                else {
                    const char * desc_p;
                    int prnt_cmd = 0;

                    support_num = rsp_buff[1] & 7;
                    num = rsp_buff[5];
                    switch (support_num) {
                    case 0: 
                        if (0 == reserved_cmddt)
                            desc_p = "no data available"; 
                        else
                            desc_p = "ignored cmddt bit, standard INQUIRY "
                                     "response";
                        break;
                    case 1: desc_p = "not supported"; break;
                    case 2: desc_p = "reserved (2)"; break;
                    case 3: desc_p = "supported as per standard"; 
                            prnt_cmd = 1;
                            break;
                    case 4: desc_p = "vendor specific (4)"; break;
                    case 5: desc_p = "supported in vendor specific way";
                            prnt_cmd = 1; 
                            break;
                    case 6: desc_p = "vendor specific (6)"; break;
                    case 7: desc_p = "reserved (7)"; break;
                    default: desc_p = "impossible value > 7"; break;
                    }
                    if (prnt_cmd) {
                        printf("  Support field: %s [", desc_p);
                        for (j = 0; j < num; ++j)
                            printf(" %.2x", (int)rsp_buff[6 + j]);
                        printf(" ]\n");
                    } else
                        printf("  Support field: %s\n", desc_p);
                }
            }
            else {
                fprintf(stderr,
                        "CmdDt INQUIRY on opcode=0x%.2x: failed\n",
                        num_opcode);
                return 1;
            }

        }
    }
    else if (do_evpd) {
        if (!do_raw)
            printf("EVPD INQUIRY, page code=0x%.2x:\n", num_opcode);
        if (0 == do_inq(sg_fd, 0, 1, num_opcode, rsp_buff, MX_ALLOC_LEN, 1)) {
            len = rsp_buff[3] + 4;
            if (num_opcode != rsp_buff[1])
                printf("non evpd respone; probably a STANDARD INQUIRY "
                       "response\n");
            else if (do_raw)
                dStrRaw((const char *)rsp_buff, len);
            else {
                if (! do_hex)
                    printf(" Only hex output supported\n");
                dStrHex((const char *)rsp_buff, len);
            }
        }
        else {
            fprintf(stderr,
                    "EVPD INQUIRY, page code=0x%.2x: failed\n", num_opcode);
            return 1;
        }
    }

    if (do_pci) {
        unsigned char slot_name[16];

        printf("\n");
        memset(slot_name, '\0', sizeof(slot_name));
        if (ioctl(sg_fd, SCSI_IOCTL_GET_PCI, slot_name) < 0) {
            if (EINVAL == errno)
                printf("ioctl(SCSI_IOCTL_GET_PCI) not supported by this "
                       "kernel\n");
            else if (ENXIO == errno)
                printf("associated adapter not a PCI device?\n");
            else
                perror("ioctl(SCSI_IOCTL_GET_PCI) failed");
        }
        else
            printf("PCI:slot_name: %s\n", slot_name);
    }

    close(sg_fd);
    return ret;
}


/* Performs a 16 byte READ CAPACITY command and fetches response.
 * Return of 0 -> success, -1 -> failure */
int do_readcap_16(int sg_fd, int pmi, unsigned long long llba, 
                  unsigned long long * llast_sect, unsigned int * sect_sz)
{
    int k, res;
    unsigned char rcCmdBlk[16] = {SERVICE_ACTION_IN, SAI_READ_CAPACITY_16, 
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    unsigned char rcBuff[RCAP16_REPLY_LEN];
    unsigned char sense_b[SENSE_BUFF_SZ];
    struct sg_io_hdr io_hdr;
    unsigned long long ls;

    if (pmi) { /* lbs only valid when pmi set */
        rcCmdBlk[14] |= 1;
        rcCmdBlk[2] = (llba >> 56) & 0xff;
        rcCmdBlk[3] = (llba >> 48) & 0xff;
        rcCmdBlk[4] = (llba >> 40) & 0xff;
        rcCmdBlk[5] = (llba >> 32) & 0xff;
        rcCmdBlk[6] = (llba >> 24) & 0xff;
        rcCmdBlk[7] = (llba >> 16) & 0xff;
        rcCmdBlk[8] = (llba >> 8) & 0xff;
        rcCmdBlk[9] = llba & 0xff;
    }
    rcCmdBlk[13] = 12;  /* Allocation length */
    memset(&io_hdr, 0, sizeof(struct sg_io_hdr));
    io_hdr.interface_id = 'S';
    io_hdr.cmd_len = sizeof(rcCmdBlk);
    io_hdr.mx_sb_len = sizeof(sense_b);
    io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;
    io_hdr.dxfer_len = sizeof(rcBuff);
    io_hdr.dxferp = rcBuff;
    io_hdr.cmdp = rcCmdBlk;
    io_hdr.sbp = sense_b;
    io_hdr.timeout = 60000;

    while (1) {
        if (ioctl(sg_fd, SG_IO, &io_hdr) < 0) {
            perror("read_capacity16 (SG_IO) error");
            return -1;
        }
        res = sg_err_category3(&io_hdr);
        if (SG_ERR_CAT_MEDIA_CHANGED == res)
            continue;
        else if (SG_ERR_CAT_CLEAN != res) {
            sg_chk_n_print3("READ CAPACITY 16 command error", &io_hdr);
            return -1;
        }
        else
            break;
    }
    for (k = 0, ls = 0; k < 8; ++k) {
        ls <<= 8;
        ls |= rcBuff[k];
    }
    *llast_sect = ls;
    *sect_sz = (rcBuff[8] << 24) | (rcBuff[9] << 16) |
               (rcBuff[10] << 8) | rcBuff[11];
    return 0;
}


/* Performs a 10 byte READ CAPACITY command and fetches response.
 * Return of 0 -> success, -1 -> failure */
int do_readcap_10(int sg_fd, int pmi, unsigned int lba, 
                  unsigned int * last_sect, unsigned int * sect_sz)
{
    int res;
    unsigned char rcCmdBlk[10] = {READ_CAPACITY, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    unsigned char rcBuff[RCAP_REPLY_LEN];
    unsigned char sense_b[SENSE_BUFF_SZ];
    struct sg_io_hdr io_hdr;

    if (pmi) { /* lbs only valid when pmi set */
        rcCmdBlk[8] |= 1;
        rcCmdBlk[2] = (lba >> 24) & 0xff;
        rcCmdBlk[3] = (lba >> 16) & 0xff;
        rcCmdBlk[4] = (lba >> 8) & 0xff;
        rcCmdBlk[5] = lba & 0xff;
    }
    memset(&io_hdr, 0, sizeof(struct sg_io_hdr));
    io_hdr.interface_id = 'S';
    io_hdr.cmd_len = sizeof(rcCmdBlk);
    io_hdr.mx_sb_len = sizeof(sense_b);
    io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;
    io_hdr.dxfer_len = sizeof(rcBuff);
    io_hdr.dxferp = rcBuff;
    io_hdr.cmdp = rcCmdBlk;
    io_hdr.sbp = sense_b;
    io_hdr.timeout = 60000;

    while (1) {
        if (ioctl(sg_fd, SG_IO, &io_hdr) < 0) {
            perror("read_capacity (SG_IO) error");
            return -1;
        }
        res = sg_err_category3(&io_hdr);
        if (SG_ERR_CAT_MEDIA_CHANGED == res)
            continue;
        else if (SG_ERR_CAT_CLEAN != res) {
            sg_chk_n_print3("READ CAPACITY command error", &io_hdr);
            return -1;
        }
        else
            break;
    }
    *last_sect = ((rcBuff[0] << 24) | (rcBuff[1] << 16) |
                 (rcBuff[2] << 8) | rcBuff[3]);
    *sect_sz = (rcBuff[4] << 24) | (rcBuff[5] << 16) |
               (rcBuff[6] << 8) | rcBuff[7];
    return 0;
}


int sg_readcap(char *file_name)
{
    int sg_fd, k, res;
    unsigned int lba = 0;
    int pmi = 0;
    int do16 = 0;
    unsigned int last_blk_addr, block_size;
    char ebuff[EBUFF_SZ];


    if ((0 == pmi) && (lba > 0)) {
        fprintf(stderr, ME "lba can only be non-zero when pmi is set\n");
        return 1;
    }
    if ((sg_fd = open(file_name, do16 ? O_RDWR : O_RDONLY)) < 0) {
        snprintf(ebuff, EBUFF_SZ, ME "error opening file: %s", file_name);
        perror(ebuff);
        return 1;
    }
    /* Just to be safe, check we have a new sg device by trying an ioctl */
    if ((ioctl(sg_fd, SG_GET_VERSION_NUM, &k) < 0) || (k < 30000)) {
        printf( ME "%s doesn't seem to be a version 3 sg device\n",
               file_name);
        close(sg_fd);
        return 1;
    }
    if (! do16) {
        res = do_readcap_10(sg_fd, pmi, lba, &last_blk_addr, &block_size);
        if ((0 == res) && (0xffffffff != last_blk_addr)) {
            //printf("Read Capacity results:\n");
            /*
            if (pmi) {
                printf("   PMI mode: given lba=0x%x, last block before "
                       "delay=0x%x\n", lba, last_blk_addr);
            } else {
                printf("   Last block address=%u (0x%x), Number of blocks=%u\n",
                       last_blk_addr, last_blk_addr, last_blk_addr + 1);
            }
            printf("   Block size = %u bytes\n", block_size);
            */
            printf( "LBA = %08x\n", last_blk_addr + 1);
            printf( "BLK = %08x\n", block_size);
            printf( "lba_hex = %u\n", last_blk_addr + 1);
            printf( "blk_hex = %u bytes\n", block_size);
            if (! pmi) {
                unsigned long long total_sz = last_blk_addr + 1;
                double sz_mb, sz_gb;

                total_sz *= block_size;
                sz_mb = ((double)(last_blk_addr + 1) * block_size) / 
                        (double)(1048576);
                sz_gb = ((double)(last_blk_addr + 1) * block_size) / 
                        (double)(1000000000L);
                //printf("Hence:\n");
                //printf("   Device size: %llu bytes, %.1f MB, %.2f GB\n",
                //       total_sz, sz_mb, sz_gb);
                printf( "HD size = %lld bytes\n", total_sz);
            }
        }
        else if (0 == res) {
            do16 = 1;
            close(sg_fd);
            if ((sg_fd = open(file_name, O_RDWR)) < 0) {
                snprintf(ebuff, EBUFF_SZ, ME "error re-opening file: %s "
                         "RDWR", file_name);
                perror(ebuff);
                return 1;
            }
        }
    }
/*    if (do16) {
        res = do_readcap_16(sg_fd, pmi, llba, &llast_blk_addr, &block_size);
        if (0 == res) {
            printf("Read Capacity results:\n");
            if (pmi) {
                printf("   PMI mode: given lba=0x%llx, last block before "
                       "delay=0x%llx\n", llba, llast_blk_addr);
            } else {
                printf("   Last block address=%llu (0x%llx), Number of "
                       "blocks=%llu\n", llast_blk_addr, llast_blk_addr, 
                       llast_blk_addr + 1);
            }
            printf("   Block size = %u bytes\n", block_size);
            if (! pmi) {
                unsigned long long total_sz = llast_blk_addr + 1;
                double sz_mb, sz_gb;

                total_sz *= block_size;
                sz_mb = ((double)(llast_blk_addr + 1) * block_size) / 
                        (double)(1048576);
                sz_gb = ((double)(llast_blk_addr + 1) * block_size) / 
                        (double)(1000000000L);
                printf("Hence:\n");
                printf("   Device size: %llu bytes, %.1f MB, %.2f GB\n",
                       total_sz, sz_mb, sz_gb);
            }
        }
    }
*/
    close(sg_fd);
    return 0;
}

int main(int argc, char **argv)
{
	int	i = 0;
	int	ret = 0;

	if ( argc < 2 ) {
		fprintf(stderr, "usage: %s /dev/sga [/dev/sgb] [/dev/sgx]\n", argv[0]);
		return -1;
	}

	for ( i = 1; i < argc; i++) {
		printf("********** start of %s **********\n", argv[i]);
		ret = sg_inq(argv[i]);
		ret = sg_readcap(argv[i]);
		printf("********** end of %s **********\n\n", argv[i]);
	}
	return 0;
}
