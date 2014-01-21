TARGET=answers

default: pdf pdf2 c

dvi: ${TARGET}.tex 
#	pygmentize the input source file -- THIS NAME SHOULD BE SAFE
#	pygmentize -f latex -o __${TARGET}.tex ${TARGET}.tex
#	run latex twice to get references correct
	latex ${TARGET}.tex
#	you can also have a bibtex line here
#	bibtex $(TARGET)
	latex $(TARGET).tex
#	remove the pygmentized output to avoid cluttering up the directory
#	rm __${TARGET}.tex

ps: dvi
	dvips -R -Poutline -t letter ${TARGET}.dvi -o ${TARGET}.ps

pdf: ps
	ps2pdf ${TARGET}.ps

TARGET2=surface_integral


dvi2: ${TARGET2}.tex 
#	pygmentize the input source file -- THIS NAME SHOULD BE SAFE
#	pygmentize -f latex -o __${TARGET2}.tex ${TARGET2}.tex
#	run latex twice to get references correct
	latex ${TARGET2}.tex
#	you can also have a bibtex line here
#	bibtex $(TARGET2)
	latex $(TARGET2).tex
#	remove the pygmentized output to avoid cluttering up the directory
#	rm __${TARGET2}.tex

ps2: dvi2
	dvips -R -Poutline -t letter ${TARGET2}.dvi -o ${TARGET2}.ps

pdf2: ps2
	ps2pdf ${TARGET2}.ps

CC = icc
CFLAGS = -Wall -std=c99 -openmp -O3 -g -I.
#overkill on the flags, but that doesn't hurt anything
LDFLAGS = -lrt -lpthread -lm
#replace this if you want to change the output name
CTARGET = sieve

#any headers go here
INCLUDES = sieve.h

#any .c or .cpp files go here
SOURCE = sieve.c

#depends on all of you source and header files
c: ${SOURCE} ${INCLUDES}
#this assumes you actually are linking all of the source files together
	${CC} ${CFLAGS} ${SOURCE} -o ${CTARGET} ${LDFLAGS}

#	debug: ${SOURCE} ${INCLUDES}
#	${CC} ${CFLAGS} ${SOURCE} -o ${TARGET} ${LDFLAGS} -DDEBUG
