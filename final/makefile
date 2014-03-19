default: pdf c

# Compiles C code

CC = icc
CFLAGS = -Wall -std=c99 -openmp -O0 -g -I.
LDFLAGS = -lrt -lpthread -lm

TARGET1 = mycopy
TARGET2 = server_posix
TARGET3 = client_posix
TARGET4 = pipe_posix

c:
	${CC} ${CFLAGS} ${TARGET1}.c -o ${TARGET1} ${LDFLAGS}
	${CC} ${CFLAGS} ${TARGET2}.c -o ${TARGET2} ${LDFLAGS}
	${CC} ${CFLAGS} ${TARGET3}.c -o ${TARGET3} ${LDFLAGS}
	${CC} ${CFLAGS} ${TARGET4}.c -o ${TARGET4} ${LDFLAGS}


# Makes LaTeX into a PDF

LATEXTARGET=writeup
HTML=main_html
SRC={$LATEXTARGET}

dvi: ${LATEXTARGET}.tex 
#	pygmentize the input source file -- THIS NAME SHOULD BE SAFE
#	pygmentize -f latex -o __${LATEXTARGET}.tex ${LATEXTARGET}.tex
#	run latex twice to get references correct
	latex -shell-escape ${LATEXTARGET}.tex
#	you can also have a bibtex line here
#	bibtex $(LATEXTARGET)
#	latex -shell-escape ${LATEXTARGET}.tex
#	remove the pygmentized output to avoid cluttering up the directory
#	rm __${TARGET}.tex

ps: dvi
	dvips -R -Poutline -t letter ${LATEXTARGET}.dvi -o ${LATEXTARGET}.ps
#	remove intermediate outputs to avoid cluttering up the directory

pdf: ps
	ps2pdf ${LATEXTARGET}.ps
#	remove intermediate outputs to avoid cluttering up the directory
	rm ${LATEXTARGET}.aux ${LATEXTARGET}.out ${LATEXTARGET}.log ${LATEXTARGET}.dvi ${LATEXTARGET}.ps
#	rm ${LATEXTARGET}.bbl ${LATEXTARGET}.blg

html:
	cp ${LATEXTARGET}.tex ${HTML}.tex
	latex ${HTML}.tex
	latex2html -split 0 -noshow_section_numbers -local_icons -no_navigation -noinfo -noaddress ${HTML}

	sed 's/<BR><HR>//g' < ${HTML}/index.html > ${HTML}/index2.html
	mv ${HTML}/index2.html ${LATEXTARGET}.html
	rm ${HTML}.*
	rm -rf ${HTML}
