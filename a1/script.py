import os
import sys
import getopt

def main():
        try:
                opts, args = getopt.getopt(sys.argv[1:], "i:t:c:", ["term=", "course="])
        except getopt.GetoptError as err:
                # print help information and exit:
                print str(err) # will print something like "option -a not recognized"
                sys.exit(2)
        for o, a in opts:
                if o == "-i":
                        options = {
                                '2' : a2,
                                '3' : a3,
                                '4' : a4,
                                '5' : a5
                        }

                        if a == "1":
                                t = None
                                c = None
                                for o, a in opts:
                                        if o == "-t" or o == "--term":
                                                t = a
                                        elif o == "-c" or "--course":
                                                c = a
                                if t is None or c is None:
                                        print "-t or --term must specify a term and -c or --course must specify a course"
                                else:
                                        a1(t, c)
                        elif a in options:
                                options[a]()
                        else:
                                print "-i must specify an integer between 1 and 5"

def a1(t, c):
        print "Creating course directories for term '%s' and course '%s'" % (t, c)

def a2():
        print()

def a3():
        print("derp3")

def a4():
        print("derp4")

def a5():
        print("derp5")

if __name__ == '__main__':
        main()
