#!/bin/sh
# Copyright (C) 2008-2012 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#
# tests basic functionality of read-ahead and ra regressions
#

. lib/test

aux prepare_devs 5

TEST_UUID="aaaaaa-aaaa-aaaa-aaaa-aaaa-aaaa-aaaaaa"

pvcreate "$dev1"
pvcreate --metadatacopies 0 "$dev2"
pvcreate --metadatacopies 0 "$dev3"
pvcreate "$dev4"
pvcreate --norestorefile -u $TEST_UUID --metadatacopies 0 "$dev5"
vgcreate -c n $vg $(cat DEVICES)
lvcreate -l 5 -i5 -I256 -n $lv $vg

# test *scan and *display tools
pvscan
vgscan
lvscan
lvmdiskscan
vgdisplay --units k
lvdisplay --units g
for i in h b s k m g t p e H B S K M G T P E; do
	pvdisplay --units $i "$dev1"
done

# test vgexport vgimport tools
vgchange -an $vg
vgexport $vg
vgimport $vg
vgchange -ay $vg

# "-persistent y --major 254 --minor 20"
# "-persistent n"
# test various lvm utils
for i in dumpconfig formats segtypes; do
	lvm $i
done

for i in pr "p rw" an ay "-monitor y" "-monitor n" \
        -resync -refresh "-addtag MYTAG" "-deltag MYETAG"; do
	lvchange -$i $vg/$lv
done

pvck "$dev1"
vgck $vg
lvrename $vg $lv $lv-rename
vgcfgbackup -f backup.$$ $vg
vgchange -an $vg
vgcfgrestore  -f backup.$$ $vg
pvremove -y -ff "$dev5"
not vgcfgrestore  -f backup.$$ $vg
pvcreate -u $TEST_UUID --restorefile  backup.$$ "$dev5"
vgremove -f $vg
pvresize --setphysicalvolumesize 10M "$dev1"

# test various errors and obsoleted tools
not lvmchange
not lvrename $vg
not lvrename $vg-xxx
not lvrename $vg  $vg/$lv-rename $vg/$lv
