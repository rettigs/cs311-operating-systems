import os
import sys
import getopt
from array import array

if __name__ == '__main__':
    names = open("names.txt", "r").read()[1:-2].split("\",\"")
    total = 0
    for namei in xrange(len(names)):
            subscore = 0
            for letteri in xrange(len(names[namei])):
                    subscore = subscore + ord(names[namei][letteri]) - 64
            score = subscore * (namei + 1)
            total = total + score
            #print "Name: %s\tSubscore: %s\tIndex: %s\tScore: %s" % (names[namei], subscore, namei + 1, score)
    print total
