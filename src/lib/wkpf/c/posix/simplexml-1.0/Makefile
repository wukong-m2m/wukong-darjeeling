# $Id: Makefile,v 1.1.1.1 2002/08/23 10:38:57 essmann Exp $
#
# Copyright (c) 2001-2002 Bruno Essmann <essmann@users.sourceforge.net>
# All rights reserved.

.SILENT:

DEMOS = dump group

all: 
	@echo "*** `pwd`: $@" ; \
	${MAKE} COMMAND=all command

clean: 
	@echo "*** `pwd`: $@" ; \
	${MAKE} COMMAND=clean command ; \
	if [ -f src/simplexml.o ] ; then \
		rm -f src/simplexml.o ; \
	fi

command:
	for i in ${DEMOS} ; do \
		(cd demo/$$i ; ${MAKE} ${COMMAND}) ; \
	done
