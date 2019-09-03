# This Makefile is generated automatically from a template

#SRCS= kmod/module_s/abs_diag kmod/module_s/addrmap kmod/module_s/memtest
SRCS= kmod/module_s/memtest

all:
	@for dir in ${SRCS}; \
	do \
		echo "Making all in $$dir"; \
		(cd $$dir; ${MAKE} all) || exit 1; \
	done

clean:
	@for dir in ${SRCS}; \
	do \
		echo "Making clean in $$dir"; \
		(cd $$dir; ${MAKE} clean) || exit 1; \
	done

install:
	@for dir in ${SRCS}; \
	do \
		echo "Making install in $$dir"; \
		(cd $$dir; ${MAKE} install) || exit 1; \
	done

uninstall:
	@for dir in ${SRCS}; \
	do \
		echo "Making uninstall in $$dir"; \
		(cd $$dir; ${MAKE} uninstall) || exit 1; \
	done

