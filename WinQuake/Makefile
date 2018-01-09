#
# Quake Makefile for Amiga AOS4
#
# Dec '99 by Frank Wille <frank@phoenix.owl.de>
# Jan '05 by Massimiliano m3x Tretene
#

VERSION=1.09
ARCH=gcc_objects

BUILDDIR=$(ARCH)
CC=gcc

CFLAGS= -O0 -mcpu=604e -trigraphs -Wunused -D__USE_OLD_TIMEVAL__ \
       -DGLQUAKE -DAMIGA -DNDEBUG -Dstricmp=strcasecmp \
       -D__USE_INLINE__ -DNO_PAULA -mcrt=newlib

OPT_CFLAGS = -O1 -mcpu=604e -trigraphs -Wunused -D__USE_OLD_TIMEVAL__ \
       -DGLQUAKE -DAMIGA -DNDEBUG -Dstricmp=strcasecmp \
       -D__USE_INLINE__ -DNO_PAULA -mcrt=newlib

MAXOPT_CFLAGS = -O2 -mcpu=604e -trigraphs -Wunused -D__USE_OLD_TIMEVAL__ \
       -DGLQUAKE -DAMIGA -DNDEBUG -Dstricmp=strcasecmp \
       -D__USE_INLINE__ -DNO_PAULA -mcrt=newlib

XOPT_CFLAGS = -O3 -mcpu=604e -trigraphs -Wunused -D__USE_OLD_TIMEVAL__ \
       -DGLQUAKE -DAMIGA -DNDEBUG -Dstricmp=strcasecmp \
       -D__USE_INLINE__ -DNO_PAULA -mcrt=newlib

NETINC=-Isdk:include/netinclude/
LDFLAGS= -lmgl -lm -lauto -mcrt=newlib

AS=as
ASFLAGS=
#ASFLAGS=-F2 -mo -D WOS

DO_CC=$(CC) $(CFLAGS) -o $@ -c $<
DO_OPT1_CC=$(CC) $(OPT_CFLAGS) -o $@ -c $<
DO_OPT2_CC=$(CC) $(MAXOPT_CFLAGS) -o $@ -c $<
DO_OPT3_CC=$(CC) $(XOPT_CFLAGS) -o $@ -c $<
DO_CCNET=$(CC) $(CFLAGS) $(NETINC) -o $@ -c $<
#DO_ASPPC=$(AS) $(ASFLAGS) -o $@ $<
#DO_AS68K=$(AS68) $(AS68FLAGS) to $@ $<


#############################################################################
# SETUP AND BUILD
#############################################################################

TARGETS=$(BUILDDIR)/glquake_aos4

all: $(TARGETS)


#############################################################################
# CLIENT/SERVER
#############################################################################

QUAKE_OBJS= \
	$(BUILDDIR)/amiga.o \
	$(BUILDDIR)/amigaos.o \
	$(BUILDDIR)/chase.o \
	$(BUILDDIR)/cl_demo.o \
	$(BUILDDIR)/cl_input.o \
	$(BUILDDIR)/cl_main.o \
	$(BUILDDIR)/cl_parse.o \
	$(BUILDDIR)/cl_tent.o \
	$(BUILDDIR)/cmd.o \
	$(BUILDDIR)/common.o \
	$(BUILDDIR)/console.o \
	$(BUILDDIR)/crc.o \
	$(BUILDDIR)/cvar.o \
	$(BUILDDIR)/host.o \
	$(BUILDDIR)/host_cmd.o \
	$(BUILDDIR)/in_amiga.o \
	$(BUILDDIR)/in_amigajoy.o \
	$(BUILDDIR)/in_amigapsx.o \
	$(BUILDDIR)/in_amigamouse.o \
	$(BUILDDIR)/keys.o \
	$(BUILDDIR)/keys_amiga.o \
	$(BUILDDIR)/menu.o \
	$(BUILDDIR)/mathlib.o \
	$(BUILDDIR)/net_amiga.o \
	$(BUILDDIR)/net_bsdsocket.o \
	$(BUILDDIR)/net_dgrm.o \
	$(BUILDDIR)/net_loop.o \
	$(BUILDDIR)/net_main.o \
	$(BUILDDIR)/net_vcr.o \
	$(BUILDDIR)/pr_cmds.o \
	$(BUILDDIR)/pr_edict.o \
	$(BUILDDIR)/pr_exec.o \
	$(BUILDDIR)/r_part.o \
	$(BUILDDIR)/gl_draw.o \
	$(BUILDDIR)/gl_mesh.o \
	$(BUILDDIR)/gl_model.o \
	$(BUILDDIR)/gl_refrag.o \
	$(BUILDDIR)/gl_rlight.o \
	$(BUILDDIR)/gl_rmain.o \
	$(BUILDDIR)/gl_rmisc.o \
	$(BUILDDIR)/gl_rsurf.o \
	$(BUILDDIR)/gl_screen.o \
	$(BUILDDIR)/gl_test.o \
	$(BUILDDIR)/gl_vidamiga.o \
	$(BUILDDIR)/gl_warp.o \
	$(BUILDDIR)/sbar.o \
	$(BUILDDIR)/sv_main.o \
	$(BUILDDIR)/sv_phys.o \
	$(BUILDDIR)/sv_move.o \
	$(BUILDDIR)/sv_user.o \
	$(BUILDDIR)/zone.o \
	$(BUILDDIR)/view.o \
	$(BUILDDIR)/wad.o \
	$(BUILDDIR)/world.o \
	$(BUILDDIR)/cd_amiga.o \
	$(BUILDDIR)/snd_ahi.o \
	$(BUILDDIR)/snd_ahi_aos4.o \
	$(BUILDDIR)/snd_dma.o \
	$(BUILDDIR)/snd_mix.o \
	$(BUILDDIR)/snd_mem.o \
	$(BUILDDIR)/snd_amiga.o \
	$(BUILDDIR)/sys_amiga.o \
	$(BUILDDIR)/twfsound_cd.o

QUAKE_AS_OBJS= \
#	$(BUILDDIR)/snd_mixamigaPPC.o

QUAKE_AS68_OBJS= \
#	$(BUILDDIR)/in_amigamouse68k.o \
#	$(BUILDDIR)/snd_int68k.o \
#	$(BUILDDIR)/snd_int68kLC.o

# use -DGLPPCASM when you want to use the following
GLQUAKE_ASOPT_OBJS= \
#	$(BUILDDIR)/fconstPPC.o \
#	$(BUILDDIR)/commonPPC.o \
#	$(BUILDDIR)/mathlibPPC.o

INCS = quakedef.h \
       common.h bspfile.h vid.h sys.h \
       zone.h mathlib.h wad.h draw.h \
       cvar.h screen.h net.h protocol.h \
       cmd.h sbar.h sound.h render.h \
       client.h progs.h server.h \
       gl_model.h input.h \
       world.h keys.h console.h \
       view.h menu.h crc.h cdaudio.h \
       pr_comp.h progdefs.h modelgen.h \
       spritegn.h glquake.h



$(BUILDDIR)/glquake_aos4: $(QUAKE_OBJS) $(QUAKE_AS_OBJS) $(GLQUAKE_ASOPT_OBJS) $(QUAKE_AS68_OBJS)
	$(CC) -o $@ $(QUAKE_OBJS) $(LDFLAGS)
	strip -R.comment $(BUILDDIR)/glquake_aos4
#	copy $(BUILDDIR)/glquake_aos4 work:games/3d/quake/

#quakedefPPC.i: genasmheaders glquakeasmheaders.gen
#	genasmheaders glquakeasmheaders.gen $@ 1 "$(CC68) $(CFLAGS) -DGENDEFS"

#glquakedefPPC.i: genasmheaders glquakeasmheaders.gen
#	genasmheaders glquakeasmheaders.gen $@ 1 "$(CC68) $(CFLAGS) -DGENDEFS"

#genasmheaders: genasmheaders.c
#	$(CC68) -o $@ genasmheaders.c

##

$(BUILDDIR)/amiga.o :               amiga.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/amigaos.o :             amigaos.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/chase.o :               chase.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/cl_demo.o :             cl_demo.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/cl_input.o :            cl_input.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/cl_main.o :             cl_main.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/cl_parse.o :            cl_parse.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/cl_tent.o :             cl_tent.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/cmd.o :                 cmd.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/common.o :              common.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/console.o :             console.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/crc.o :                 crc.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/cvar.o :                cvar.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/host.o :                host.c $(INCS)  r_local.h r_shared.h
	$(DO_OPT3_CC)

$(BUILDDIR)/host_cmd.o :            host_cmd.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/keys.o :                keys.c $(INCS) 
	$(DO_OPT3_CC)

$(BUILDDIR)/keys_amiga.o :          keys_amiga.c $(INCS) 
	$(DO_OPT3_CC)

$(BUILDDIR)/menu.o :                menu.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/mathlib.o :             mathlib.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/pr_cmds.o :             pr_cmds.c $(INCS)
	$(DO_OPT2_CC)

$(BUILDDIR)/pr_edict.o :            pr_edict.c $(INCS)
	$(DO_OPT2_CC)

$(BUILDDIR)/pr_exec.o :             pr_exec.c $(INCS)
	$(DO_OPT2_CC)

$(BUILDDIR)/r_part.o :              r_part.c $(INCS) r_local.h r_shared.h
	$(DO_OPT3_CC)

$(BUILDDIR)/gl_draw.o :             gl_draw.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/gl_mesh.o :             gl_mesh.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/gl_model.o :            gl_model.c $(INCS) gl_model.h
	$(DO_OPT3_CC)

$(BUILDDIR)/gl_refrag.o :           gl_refrag.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/gl_rlight.o :           gl_rlight.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/gl_rmain.o :            gl_rmain.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/gl_rmisc.o :            gl_rmisc.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/gl_rsurf.o :            gl_rsurf.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/gl_screen.o :           gl_screen.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/gl_test.o :             gl_test.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/gl_vidamiga.o :         gl_vidamiga.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/gl_warp.o :             gl_warp.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/sbar.o :                sbar.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/sv_main.o :             sv_main.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/sv_phys.o :             sv_phys.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/sv_move.o :             sv_move.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/sv_user.o :             sv_user.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/zone.o	:               zone.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/view.o	:               view.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/wad.o :                 wad.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/world.o :               world.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/cd_amiga.o :            cd_amiga.c $(INCS) twfsound_cd.h
	$(DO_OPT3_CC)

$(BUILDDIR)/twfsound_cd.o :         twfsound_cd.c twfsound_cd.h
	$(DO_OPT3_CC)

$(BUILDDIR)/snd_dma.o :             snd_dma.c $(INCS)
	$(DO_OPT3_CC)

# compiling with optimizations enabled under gcc 4.2.4 produces noise...
$(BUILDDIR)/snd_mix.o :             snd_mix.c $(INCS)
	$(DO_CC)

$(BUILDDIR)/snd_mem.o :             snd_mem.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/snd_amiga.o :           snd_amiga.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/snd_ahi.o :             snd_AHI.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/snd_ahi_aos4.o :        snd_ahi_aos4.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/net_amiga.o :           net_amiga.c $(INCS) net_loop.h net_dgrm.h
	$(DO_OPT3_CC)

$(BUILDDIR)/net_bsdsocket.o :       net_bsdsocket.c $(INCS)
	$(DO_CCNET)

$(BUILDDIR)/net_dgrm.o :            net_dgrm.c $(INCS) net_dgrm.h
	$(DO_OPT3_CC)

$(BUILDDIR)/net_loop.o :            net_loop.c $(INCS) net_loop.h
	$(DO_OPT3_CC)

$(BUILDDIR)/net_main.o :            net_main.c $(INCS) net_vcr.h
	$(DO_OPT3_CC)

$(BUILDDIR)/net_vcr.o :             net_vcr.c $(INCS) net_vcr.h
	$(DO_OPT3_CC)

$(BUILDDIR)/sys_amiga.o :           sys_amiga.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/in_amiga.o :            in_amiga.c $(INCS) in_amiga.h
	$(DO_OPT3_CC)

$(BUILDDIR)/in_amigajoy.o :         in_amigajoy.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/in_amigapsx.o :         in_amigapsx.c $(INCS)
	$(DO_OPT3_CC)

$(BUILDDIR)/in_amigamouse.o :       in_amigamouse.c $(INCS)
	$(DO_OPT3_CC)


#####

$(BUILDDIR)/snd_mixamigaPPC.o: snd_mixamigaPPC.s quakedefPPC.i
	$(DO_ASPPC)

$(BUILDDIR)/commonPPC.o: commonPPC.s
	$(DO_ASPPC)

$(BUILDDIR)/fconstPPC.o: fconstPPC.s
	$(DO_ASPPC)

$(BUILDDIR)/mathlibPPC.o: mathlibPPC.s glquakedefPPC.i
	$(DO_ASPPC)


#####

$(BUILDDIR)/in_amigamouse68k.o: in_amigamouse68k.s
	$(DO_AS68K)

$(BUILDDIR)/snd_int68k.o: snd_int68k.s
	$(DO_AS68K)

$(BUILDDIR)/snd_int68kLC.o: snd_int68kLC.s
	$(DO_AS68K)


#############################################################################
# MISC
#############################################################################

clean:
	delete force quiet $(QUAKE_OBJS)
	delete force quiet $(QUAKE_AS_OBJS)
	delete force quiet $(QUAKE_AS68_OBJS)
	delete force quiet $(QUAKE_ASOPT_OBJS)
