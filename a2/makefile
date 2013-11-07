default: c

both: pdf c

# Compiles C code

CC = gcc
CFLAGS = -Wall -std=c99 -openmp -O3 -g -I.
#overkill on the flags, but that doesn't hurt anything
LDFLAGS = -lrt -lpthread -lm
#replace this if you want to change the output name
TARGET = example

#any headers go here
INCLUDES = myar.h

#any .c or .cpp files go here
SOURCE = myar.c

#depends on all of you source and header files
c: ${SOURCE} ${INCLUDES}
#this assumes you actually are linking all of the source files together
	${CC} ${CFLAGS} ${SOURCE} -o ${TARGET} ${LDFLAGS}

#	debug: ${SOURCE} ${INCLUDES}
#		${CC} ${CFLAGS} ${SOURCE} -o ${TARGET} ${LDFLAGS} -DDEBUG

# Makes LaTeX into a PDF

TARGET=writeup
HTML=main_html
SRC={$TARGET}

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


html:
	cp ${TARGET}.tex ${HTML}.tex
	latex ${HTML}.tex
	latex2html -split 0 -noshow_section_numbers -local_icons -no_navigation -noinfo -noaddress ${HTML}

	sed 's/<BR><HR>//g' < ${HTML}/index.html > ${HTML}/index2.html
	mv ${HTML}/index2.html ${TARGET}.html
	rm ${HTML}.*
	rm -rf ${HTML}

