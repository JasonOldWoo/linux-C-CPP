include makefile.def

DIRS =	\
	mhcomm_dll	\
	server	\
	client

all:
	for i in $(DIRS) ; do			\
		if [ -d $$i ]; then		\
			(cd $$i; make) ;	\
		else				\
			echo "-----------------------------------------------"	\
			echo " $$i is not a directory......";			\
			echo "-----------------------------------------------";	\
		fi				\
	done

clean:
	for i in $(DIRS) ; do			\
		if [ -d $$i ]; then		\
			(cd $$i; make clean) ;	\
		else				\
			echo "-----------------------------------------------"	\
			echo " $$i is not a directory......";			\
			echo "-----------------------------------------------";	\
		fi				\
	done

