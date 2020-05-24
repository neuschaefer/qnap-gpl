#!/bin/bash
# Program:
#    This program is auto generate compat file for new kernel.
#    But it can not handle QNAP patch, please modify scsi in new kernel.
#    File path: $KSRC/drivers/scsi/libiscsi.c, drivers/scsi/iscsi_tcp.c 
#    and include/scsi/libiscsi.h

KVERSION=`cat $KSRC/Makefile | awk -F= '/^VERSION =/ {print $2}' | \
		sed 's/^[ \t]*//;s/[ \t]*$$//'`
KPATCHLEVEL=`cat $KSRC/Makefile | awk -F= '/^PATCHLEVEL =/ {print $2}' | \
		sed 's/^[ \t]*//;s/[ \t]*$$//'`
KSUBLEVEL=`cat $KSRC/Makefile | awk -F= '/^SUBLEVEL =/ {print $2}' | \
		sed 's/^[ \t]*//;s/[ \t]*$$//'`

OLD_KERNEL_FOLDER=$PWD/kernel/old
NEW_KERNEL_FOLDER=$PWD/kernel/new

KERNEL_TARGET=linux\_$KVERSION\_$KPATCHLEVEL\_$KSUBLEVEL
COMPAT_FILE=$KVERSION.$KPATCHLEVEL.$KSUBLEVEL\_compat.patch


modify_openiscsi_kernel_makefile_func()
{
if [ -f $PWD/kernel/Makefile.bak ]
then
    echo "Makefile.bak is exist!"
else
    /bin/cp $PWD/kernel/Makefile $PWD/kernel/Makefile.bak
fi
    /bin/sed -i "s/linux_sample/$KERNEL_TARGET/g" $PWD/kernel/Makefile
    /bin/sed -i "s/sample_compat.patch/$COMPAT_FILE/g" $PWD/kernel/Makefile
}

gen_compat_file_func()
{
    /bin/mkdir $OLD_KERNEL_FOLDER
    /bin/mkdir $NEW_KERNEL_FOLDER
    /bin/cp $PWD/kernel/iscsi_tcp.h $OLD_KERNEL_FOLDER/
    /bin/cp $PWD/kernel/libiscsi.h $OLD_KERNEL_FOLDER/
    /bin/cp $PWD/kernel/libiscsi_tcp.h $OLD_KERNEL_FOLDER/
    /bin/cp $PWD/kernel/scsi_transport_iscsi.h $OLD_KERNEL_FOLDER/
    /bin/cp $PWD/kernel/iscsi_tcp.c $OLD_KERNEL_FOLDER/
    /bin/cp $PWD/kernel/libiscsi.c $OLD_KERNEL_FOLDER/
    /bin/cp $PWD/kernel/libiscsi_tcp.c $OLD_KERNEL_FOLDER/
    /bin/cp $PWD/kernel/scsi_transport_iscsi.c $OLD_KERNEL_FOLDER/
    /bin/cp $KSRC/include/scsi/scsi_transport_iscsi.h $NEW_KERNEL_FOLDER/
    /bin/cp $KSRC/include/scsi/libiscsi_tcp.h $NEW_KERNEL_FOLDER/
    /bin/cp $KSRC/include/scsi/libiscsi.h $NEW_KERNEL_FOLDER/
    /bin/cp $KSRC/drivers/scsi/iscsi_tcp.h $NEW_KERNEL_FOLDER/
    /bin/cp $KSRC/drivers/scsi/iscsi_tcp.c $NEW_KERNEL_FOLDER/
    /bin/cp $KSRC/drivers/scsi/libiscsi.c $NEW_KERNEL_FOLDER/
    /bin/cp $KSRC/drivers/scsi/libiscsi_tcp.c $NEW_KERNEL_FOLDER/
    /bin/cp $KSRC/drivers/scsi/scsi_transport_iscsi.c $NEW_KERNEL_FOLDER/
    if [ -d "$NEW_KERNEL_FOLDER/qnap" ]; then
        rm -fr $NEW_KERNEL_FOLDER/qnap
    fi
    if [ "x$KVERSION.$KPATCHLEVEL" = "x4.14" ]; then
        mkdir $NEW_KERNEL_FOLDER/qnap
        /bin/cp $KSRC/drivers/scsi/sd.h $NEW_KERNEL_FOLDER/
        /bin/cp $KSRC/drivers/scsi/sd.c $NEW_KERNEL_FOLDER/
        /bin/cp $KSRC/drivers/scsi/qnap/qnap_libiscsi.h $NEW_KERNEL_FOLDER/qnap/
        /bin/cp $KSRC/drivers/scsi/qnap/qnap_libiscsi.c $NEW_KERNEL_FOLDER/qnap/
        /bin/cp $KSRC/drivers/scsi/qnap/qnap_virtual.h $NEW_KERNEL_FOLDER/qnap/
        /bin/cp $KSRC/drivers/scsi/qnap/qnap_virtual.c $NEW_KERNEL_FOLDER/qnap/
        /bin/cp $KSRC/drivers/scsi/qnap/qnap_virtual_jbod.h $NEW_KERNEL_FOLDER/qnap/
        /bin/cp $KSRC/drivers/scsi/qnap/qnap_virtual_jbod.c $NEW_KERNEL_FOLDER/qnap/
    fi
    cd kernel/
    /usr/bin/diff -uNr old/ new/ > $COMPAT_FILE
    cd -
}

clean_func()
{
if [ -f $PWD/kernel/Makefile.bak ]
then
    /bin/mv $PWD/kernel/Makefile.bak $PWD/kernel/Makefile
    /bin/rm -Rf $OLD_KERNEL_FOLDER/
    /bin/rm -Rf $NEW_KERNEL_FOLDER/
    /bin/rm $PWD/kernel/$COMPAT_FILE
    if [ -d "qnap" ]; then
        rm -fr qnap
    fi
fi

}

start_func()
{
if [ -f "$COMPAT_FILE" ]
then
    echo $COMPAT_FILE "is exist, auto gen script do nothing."
else
    echo $COMPAT_FILE "is not exist, generate this file and modify Makefile."
    gen_compat_file_func
    modify_openiscsi_kernel_makefile_func
fi
}

case "$1" in
    start)
        start_func
    ;;
    clean)
        clean_func
    ;;
esac
