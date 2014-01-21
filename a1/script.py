import os
import sys
import getopt
import math
import numpy

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
                                '1' : a1,
                                '2' : a2,
                                '4' : a4,
                                '5' : a5
                        }

                        if a == "3":
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
                                        a3(t, c)
                        elif a in options:
                                options[a]()
                        else:
                                print "-i must specify an integer between 1 and 5"

def a1():
    primes = sieve(100)
    print "primes: %d" % len(primes)
    for p in primes:
        print p

def a2():
    a = numpy.array([[1, 2, 3],
                     [4, 5, 6]])
    b = numpy.array([[1, 2],
                     [3, 4],
                     [5, 6]])
    print matrix_mult(a, b)

def a3(t, c):
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

def a4():
    olddict = {'1':'a',
               '2':'b',
               '3':'c'}
    print dictswap(olddict)

def a5():
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

def sieve(n):
    primes = []

    # Fill working list with 1s.
    numbers = [1 for i in range(n+1)]

    # Mark the number 1 as special.
    numbers[1] = 0

    # Mark all multiples of composite numbers as 0, starting with the number 2 .
    for k in range(1, int(math.sqrt(n))):

        # Find the first number in the list greater than k that has not been identified as composite and call it m.
        for m in range(1, n+1):
            if m > k and numbers[m] != 0:
                break

        # Mark multiples of m as composite.
        i = 2
        while i*m <= n:
            numbers[i*m] = 0
            i += 1

    # Add all left over numbers to the list of primes.
    for i in range(1, n):
        if numbers[i] != 0:
            primes.append(i)

    return primes

def matrix_mult(a, b):
    arows = a.shape[0]
    acols = a.shape[1]
    brows = b.shape[0]
    bcols = b.shape[1]

    if acols != brows:
        return "Cannot multiply matrices: size mismatch."
    else:
        rows = arows
        cols = bcols
        iters = acols # = brows
        c = [[0 for x in range(cols)] for y in range(rows)]

        for i in range(rows):
            for j in range(cols):
                for k in range(iters):
                    c[i][j] += a[i][k] * b[k][j]
        return c

def dictswap(olddict):
    newdict = {}
    for k in olddict:
        newdict[olddict[k]] = k
    return newdict

if __name__ == '__main__':
        main()
