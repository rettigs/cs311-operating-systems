import os
import sys
import getopt
from array import array

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
                                '4' : a4
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
        a = array("c", "7316717653133062491922511967442657474235534919493496983520312774506326239578318016984801869478851843858615607891129494954595017379583319528532088055111254069874715852386305071569329096329522744304355766896648950445244523161731856403098711121722383113622298934233803081353362766142828064444866452387493035890729629049156044077239071381051585930796086670172427121883998797908792274921901699720888093776657273330010533678812202354218097512545405947522435258490771167055601360483958644670632441572215539753697817977846174064955149290862569321978468622482839722413756570560574902614079729686524145351004748216637048440319989000889524345065854122758866688116427171479924442928230863465674813919123162824586178664583591245665294765456828489128831426076900422421902267105562632111110937054421750694165896040807198403850962455444362981230987879927244284909188845801561660979191338754992005240636899125607176060588611646710940507754100225698315520005593572972571636269561882670428252483600823257530420752963450")
        highest = 0
        for i in xrange(len(a) - 4):
                product = int(a[i])
                for n in xrange(i + 1, i + 5):
                        product = product * int(a[n])
                if product > highest:
                        highest = product
        print highest

def a3():
        names = open("names.txt", "r").read()[1:-1].split("\",\"")
        total = 0
        for namei in xrange(len(names)):
                subscore = 0
                for letteri in xrange(len(names[namei])):
                        subscore = subscore + ord(names[namei][letteri]) - 64
                score = subscore * (namei + 1)
                total = total + score
                #print "Name: %s\tSubscore: %s\tIndex: %s\tScore: %s" % (names[namei], subscore, namei + 1, score)
        print total

def a4():
        print("derp4")

if __name__ == '__main__':
        main()
