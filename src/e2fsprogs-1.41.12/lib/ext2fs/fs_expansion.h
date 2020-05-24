/*
 *  Copyright (C) 2011
 *  Benjamin Wang (benjaminwang@qnap.com)
 */
#ifndef __QNAP_FS_EXPANSION__
#define __QNAP_FS_EXPANSION__

#include "ext2_fs.h"

/* Note that DEFAULT_BLK_SIZE should be the same with blocksize in [defaults] in default_profile.c */
#define DEFAULT_BLK_SIZE 4096ULL

#define BYTE_CNT_1GB    (1024ULL * 1024ULL * 1024ULL)
#define BYTE_CNT_1TB    (1024ULL * BYTE_CNT_1GB)
#define BYTE_CNT_128TB  (128ULL * BYTE_CNT_1TB)
#define BLK_CNT_1TB     (BYTE_CNT_1TB / DEFAULT_BLK_SIZE)
#define BLK_CNT_128TB   (128ULL * BLK_CNT_1TB) // 128 TB has 34359738368 (4-KByte) blocks

#endif /* __QNAP_FS_EXPANSION__ */
