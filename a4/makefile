default: pdf c

# Compiles C code

CC = gcc
CFLAGS = -Wall -std=c99 -openmp -O0 -g -I.
LDFLAGS = -lrt -lpthread -lm

TARGET1 = trouter
TARGET2 = prouter

c:
	${CC} ${CFLAGS} ${TARGET1}.c -o ${TARGET1} ${LDFLAGS}
	${CC} ${CFLAGS} ${TARGET2}.c -o ${TARGET2} ${LDFLAGS}


# Makes LaTeX into a PDF

LATEXTARGET=writeup
HTML=main_html
SRC={$LATEXTARGET}

dvi: ${LATEXTARGET}.tex 
#	pygmentize the input source file -- THIS NAME SHOULD BE SAFE
#	pygmentize -f latex -o __${LATEXTARGET}.tex ${LATEXTARGET}.tex
#	run latex twice to get references correct
	latex ${LATEXTARGET}.tex
#	you can also have a bibtex line here
#	bibtex $(LATEXTARGET)
	latex $(LATEXTARGET).tex
#	remove the pygmentized output to avoid cluttering up the directory
#	rm __${LATEXTARGET}.tex

ps: dvi
	dvips -R -Poutline -t letter ${LATEXTARGET}.dvi -o ${LATEXTARGET}.ps

pdf: ps
	ps2pdf ${LATEXTARGET}.ps


html:
	cp ${LATEXTARGET}.tex ${HTML}.tex
	latex ${HTML}.tex
	latex2html -split 0 -noshow_section_numbers -local_icons -no_navigation -noinfo -noaddress ${HTML}

	sed 's/<BR><HR>//g' < ${HTML}/index.html > ${HTML}/index2.html
	mv ${HTML}/index2.html ${LATEXTARGET}.html
	rm ${HTML}.*
	rm -rf ${HTML}

