## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.3

# library interface 2.1, gd release 1.8.0
libgd_la_LDFLAGS = -version-info 3:0

noinst_HEADERS = gdcache.h 

lib_LTLIBRARIES = libgd.la

include_HEADERS = gd.h gd_io.h gdfontt.h gdfonts.h \
	gdfontmb.h gdfontl.h gdfontg.h gdhelpers.h wbmp.h jisx0208.h

libgd_la_SOURCES = gd.c gd_io.c gd_io_file.c gd_io_dp.c gd_io_ss.c \
	gd_png.c gd_ss.c gd_gd.c gd_gd2.c gdfontt.c gdfonts.c gdfontmb.c \
	gdfontl.c gdfontg.c gdttf.c gdcache.c gdxpm.c gdtables.c gdkanji.c \
    gd_gif_in.c gd_gif_out.c gd_biggif_out.c gd_lzw_out.c \
	gd_jpeg.c gdft.c gdhelpers.c gd_wbmp.c wbmp.c 

bin_PROGRAMS = pngtogd pngtogd2 gdtopng gdtojpeg gd2topng gd2copypal gdparttopng webpng \
               gd2togif gdcmpgif giftogd2

pngtogd_LDADD = libgd.la

noinst_PROGRAMS = gdtest gddemo gd2time gdtestttf gdtestft

gdtest_SOURCES = gdtest.c gd.h
gdtest_LDADD = libgd.la

gdtestttf_SOURCES = gdtestttf.c gd.h
gdtestttf_LDADD = libgd.la

gdtestft_SOURCES = gdtestft.c gd.h
gdtestft_LDADD = libgd.la

gddemo_SOURCES = gddemo.c gd.h gdfonts.h gdfontg.h
gddemo_LDADD = libgd.la

pngtogd_SOURCES = pngtogd.c gd.h
pngtogd_LDADD = libgd.la

pngtogd2_SOURCES = pngtogd2.c gd.h
pngtogd2_LDADD = libgd.la

gdtojpeg_SOURCES = gdtojpeg.c gd.h
gdtojpeg_LDADD = libgd.la

gdtopng_SOURCES = gdtopng.c gd.h
gdtopng_LDADD = libgd.la

gd2topng_SOURCES = gd2topng.c gd.h
gd2topng_LDADD = libgd.la

gd2copypal_SOURCES = gd2copypal.c gd.h
gd2copypal_LDADD = libgd.la

gdparttopng_SOURCES = gdparttopng.c gd.h
gdparttopng_LDADD = libgd.la

gd2time_SOURCES = gd2time.c gd.h
gd2time_LDADD = libgd.la

webpng_SOURCES = webpng.c gd.h
webpng_LDADD = libgd.la

gd2togif_SOURCES = gd2togif.c gd.h
gd2togif_LDADD = libgd.la

gdcmpgif_SOURCES = gdcmpgif.c gd.h
gdcmpgif_LDADD = libgd.la

giftogd2_SOURCES = giftogd2.c gd.h
giftogd2_LDADD = libgd.la


EXTRA_DIST = demoin.png index.html test/* mathmake.c bdftogd Makefile.nt

CLEANFILES = demoout.png test/fttest.png

