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
*  Copyright (C) 2000-2003 D. Gilbert
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2, or (at your option)
*  any later version.

   This program outputs information provided by a SCSI INQUIRY command.
   It is mainly based on the SCSI-3 SPC-2 document.

   Acknowledgment:
      - Martin Schwenke <martin@meltin.net> added the raw switch and other
        improvements [20020814]

From SPC-3 revision 16 the CmdDt bit in an INQUIRY is obsolete. There is
now a REPORT SUPPORTED OPERATION CODES command that yields similar
information [MAINTENANCE IN, service action = 0xc]. Support will be added
in the future.
   
*/

static char * version_str = "0.27 20031215";


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

static void usage()
{
    fprintf(stderr,
            "Usage: 'sg_inq [-c] [-cl] [-e] [-h|-r] [-o=<opcode_page>] [-p]"
            " [-V] [-36]\n               [-?] <sg_device>'\n"
            " where -c   set CmdDt mode (use -o for opcode) [obsolete]\n"
            "       -cl  list supported commands using CmdDt mode [obsolete]\n"
            "       -e   set EVPD mode (use -o for page code)\n"
            "       -h   output in hex (ASCII to the right)\n"
            "       -o=<opcode_page> opcode or page code in hex\n"
            "       -p   output SCSI adapter PCI information\n"
            "       -r   output raw binary data\n"
            "       -V   output version string\n"
            "       -36  only perform a 36 byte INQUIRY\n"
            "       -?   output this usage message\n"
            " If no optional switches given (or '-h') then does"
            " a standard INQUIRY\n");
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



int main(int argc, char * argv[])
{
    int sg_fd, k, j, num, len, act_len;
    int support_num;
    char * file_name = 0;
    char ebuff[EBUFF_SZ];
    char buff[MX_ALLOC_LEN + 1];
    const char * cp;
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

    for (k = 1; k < argc; ++k) {
        if (0 == strncmp("-o=", argv[k], 3)) {
            num = sscanf(argv[k] + 3, "%x", &num_opcode);
            if ((1 != num) || (num_opcode > 255)) {
                fprintf(stderr, "Bad number after '-o' switch\n");
                file_name = 0;
                break;
            }
        }
        else if (0 == strcmp("-e", argv[k]))
            do_evpd = 1;
        else if (0 == strcmp("-h", argv[k]))
            do_hex = 1;
        else if (0 == strcmp("-r", argv[k]))
            do_raw = 1;
        else if (0 == strcmp("-cl", argv[k])) {
            do_cmdlst = 1;
            do_cmddt = 1;
        }
        else if (0 == strcmp("-c", argv[k]))
            do_cmddt = 1;
        else if (0 == strcmp("-p", argv[k]))
            do_pci = 1;
        else if (0 == strcmp("-36", argv[k]))
            do_36 = 1;
        else if (0 == strcmp("-?", argv[k])) {
            file_name = 0;
            break;
        }
        else if (0 == strcmp("-V", argv[k])) {
            fprintf(stderr, "Version string: %s\n", version_str);
            exit(0);
        }
        else if (*argv[k] == '-') {
            fprintf(stderr, "Unrecognized switch: %s\n", argv[k]);
            file_name = 0;
            break;
        }
        else if (0 == file_name)
            file_name = argv[k];
        else {
            fprintf(stderr, "too many arguments\n");
            file_name = 0;
            break;
        }
    }
    
    if (do_raw && do_hex) {
        fprintf(stderr, "Can't do hex and raw at the same time\n");
        file_name = 0;
    }
    
    if (0 == file_name) {
        usage();
        return 1;
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
        if (!do_raw)
            printf("standard INQUIRY:\n");
        if (num_opcode > 0)
            printf(" <<given opcode or page_code is being ignored>>\n");
        
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
                printf("  PQual=%d  Device_type=%d  RMB=%d  [ANSI_version=%d] ",
                       (rsp_buff[0] & 0xe0) >> 5, rsp_buff[0] & 0x1f,
                       !!(rsp_buff[1] & 0x80), ansi_version);
                printf(" version=0x%02x\n", (unsigned int)rsp_buff[2]);
                printf("  [AERC=%d]  [TrmTsk=%d]  NormACA=%d  HiSUP=%d "
                       " Resp_data_format=%d\n  SCCS=%d  ",
                       !!(rsp_buff[3] & 0x80), !!(rsp_buff[3] & 0x40),
                       !!(rsp_buff[3] & 0x20), !!(rsp_buff[3] & 0x10),
                       rsp_buff[3] & 0x0f, !!(rsp_buff[5] & 0x80));
                printf("ACC=%d  ALUA=%d  3PC=%d  Protect=%d\n",
                       !!(rsp_buff[5] & 0x40), ((rsp_buff[5] & 0x30) >> 4),
		       !!(rsp_buff[5] & 0x08), !!(rsp_buff[5] & 0x01));
                printf("  BQue=%d  EncServ=%d  MultiP=%d  MChngr=%d  "
                       "[ACKREQQ=%d]  ",
                       !!(rsp_buff[6] & 0x80), !!(rsp_buff[6] & 0x40), 
                       !!(rsp_buff[6] & 0x10), !!(rsp_buff[6] & 0x08), 
                       !!(rsp_buff[6] & 0x04));
                printf("Addr16=%d\n  [RelAdr=%d]  ",
                       !!(rsp_buff[6] & 0x01),
                       !!(rsp_buff[7] & 0x80));
                printf("WBus16=%d  Sync=%d  Linked=%d  [TranDis=%d]  ",
                       !!(rsp_buff[7] & 0x20), !!(rsp_buff[7] & 0x10),
                       !!(rsp_buff[7] & 0x08), !!(rsp_buff[7] & 0x04));
                printf("CmdQue=%d\n", !!(rsp_buff[7] & 0x02));
                if (len > 56)
                    printf("  Clocking=0x%x  QAS=%d  IUS=%d\n",
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

                if (len <= 8)
                    printf(" Inquiry response length=%d\n, no vendor, "
                           "product or revision data\n", len);
                else {
                    if (len < 36)
                        rsp_buff[len] = '\0';
                    memcpy(buff, &rsp_buff[8], 8);
                    buff[8] = '\0';
                    printf(" Vendor identification: %s\n", buff);
                    if (len <= 16)
                        printf(" Product identification: <none>\n");
                    else {
                        memcpy(buff, &rsp_buff[16], 16);
                        buff[16] = '\0';
                        printf(" Product identification: %s\n", buff);
                    }
                    if (len <= 32)
                        printf(" Product revision level: <none>\n");
                    else {
                        memcpy(buff, &rsp_buff[32], 4);
                        buff[4] = '\0';
                        printf(" Product revision level: %s\n", buff);
                    }
                }
            }
            if (!do_raw &&
                (0 == do_inq(sg_fd, 0, 1, 0x80, rsp_buff, MX_ALLOC_LEN, 0))) {
                len = rsp_buff[3];
                if (len > 0) {
                    memcpy(buff, rsp_buff + 4, len);
                    buff[len] = '\0';
                    printf(" Product serial number: %s\n", buff);
                }
            }
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
