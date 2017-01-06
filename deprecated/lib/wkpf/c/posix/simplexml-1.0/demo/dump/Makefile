# $Id: Makefile,v 1.1.1.1 2002/08/23 10:38:58 essmann Exp $
#
# Copyright (c) 2001-2002 Bruno Essmann <essmann@users.sourceforge.net>
# All rights reserved.

.SILENT:

TARGET = dump
OBJECT = dump.o
DATA = dump.xml

SIMPLEXMLDIR = ../../src
SIMPLEXMLOBJECT = ${SIMPLEXMLDIR}/simplexml.o
SIMPLEXMLHEADER = ${SIMPLEXMLDIR}/simplexml.h

INCLUDES = -I${SIMPLEXMLDIR}

all: ${TARGET}

run: ${TARGET}
	@echo "*** `pwd`: $@" ; \
	echo ${TARGET} ${DATA} ; \
	${TARGET} ${DATA}

clean:
	@echo "*** `pwd`: $@" ; \
	for i in ${OBJECT}; do if [ -f $$i ] ; then rm $$i; fi; done ; \
	if [ -f ${TARGET} ] ; then rm ${TARGET}; fi

.c.o:
	@echo "*** `pwd`: $@" ; \
	gcc -g -c -Wall ${INCLUDES} ${FLAGS} $< -o $*.o

${SIMPLEXMLOBJECT}: ${SIMPLEXMLHEADER}

${OBJECT}: ${SIMPLEXMLHEADER}

${TARGET}: ${OBJECT} ${SIMPLEXMLOBJECT}
	@echo "*** `pwd`: $@" ; \
	gcc -g -o ${TARGET} ${OBJECT} ${SIMPLEXMLOBJECT} ${LIBINC} ${LIBS} ; \
	strip ${TARGET}
	
