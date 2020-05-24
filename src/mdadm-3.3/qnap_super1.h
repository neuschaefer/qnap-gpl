#ifndef __QNAP_SUPER_H__
#define __QNAP_SUPER_H__


typedef enum {
    eQNAP_MD_SUPER1_FEATURE_MIN = 0,
    eQNAP_MD_SUPER1_FEATURE_FLEX_RAID = eQNAP_MD_SUPER1_FEATURE_MIN,
    eQNAP_MD_SUPER1_FEATURE_MAX
}eQNAP_MD_SUPER1_FEATURE;

/* generic */
void qnap_super_set_feature(struct supertype *st, eQNAP_MD_SUPER1_FEATURE feature);
void qnap_dump_qnap_feature(struct supertype *st);

/* super1 */
int qnap_super1_is_feature_supported(eQNAP_MD_SUPER1_FEATURE feature);

/* others */
//super-ddf 
//super-mbr ..and so on
//
//
#endif
