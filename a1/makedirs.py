import os
import sys
import getopt

def main():
        try:
                opts, args = getopt.getopt(sys.argv[1:], "t:c:", ["term=", "course="])
        except getopt.GetoptError as err:
                # print help information and exit:
                print str(err) # will print something like "option -a not recognized"
                sys.exit(2)
        for o, a in opts:
            t = None
            c = None
            for o, a in opts:
                    if a is not None:
                            if o == "-t" or o == "--term":
                                    t = a
                            elif o == "-c" or o == "--course":
                                    c = a
            if t is None or c is None:
                    print "-t or --term must specify a term and -c or --course must specify a course"
            else:
                    a1(t, c)

def makedirs(t, c):
        dirs = "assignments examples exams lecture_notes submissions".split()
        for d in dirs:
                try:
                        os.makedirs("./%s/%s/%s" % (t, c, d))
                except OSError:
                        pass
        
        try:
                os.symlink("/usr/local/classes/eecs/%s/%s/public_html" % (t, c), "website")
        except OSError:
                pass
        try:
                os.symlink("/usr/local/classes/eecs/%s/%s/handin" % (t, c), "handin")
        except OSError:
                pass
        print "Created course directories for term '%s' and course '%s'" % (t, c)

if __name__ == '__main__':
        main()
