import os
import sys
import getopt
import math
import re

# Defaults
ip = "127.0.0.1"
port = 54321
outfile = sys.stdout

def main():

# Parse arguments
    try:
        opts, args = getopt.getopt(sys.argv[1:], "da:p:i:o:s:h")
    except getopt.GetoptError as err:
        # print help information and exit:
        print str(err) # will print something like "option -a not recognized"
        sys.exit(2)
    for o, a in opts:
        if o == "-d":
            debug++
        elif o == "-a":
            ip = a
        elif o == "-p":
            port = a
        elif o == "-i":
            infile = open(a, 'r')
        elif o == "-o":
            outfile = open(a, 'w')
        elif o == "-s":
            checkip = a
        else:
            usage()
        
# Set up the socket
    socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    socket.connect((ip, port))

    if infile != null: # Run in batch mode if we were given an infile
        for line in infile:
            socket.send('<query><ip>{}</ip></query>'.format(line))
            socket.flush()
            asn = re.sub(r'<answer><asn>(\d+)</asn></answer>', r'\1', socket.makefile().readline()
            outfile.write(line + ' ' + asn + '\n')
    elif checkip != null: # Run in one-off mode if we were given a single query ip address
        socket.send('<query><ip>{}</ip></query>'.format(line))
        socket.flush()
        asn = re.sub(r'<answer><asn>(\d+)</asn></answer>', r'\1', socket.makefile().readline()
        printf asn + '\n'
    else:
        usage()

def usage():
    print 'Usage: {} [-h] [-a server_ipaddress] [-p server_port] [-i infile] [-o outfile] [-q query_ipaddress] [-d]...'.format(sys.argv[0])
    sys.exit(2)
        
if __name__ == '__main__':
    main()
