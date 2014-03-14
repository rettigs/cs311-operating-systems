#!/usr/bin/python

import os
import sys
import getopt
import socket
import re

def main():

# Defaults
    debug = 0
    ip = '127.0.0.1'
    port = 54321
    infile = None
    outfile = sys.stdout

# Parse arguments
    try:
        opts, args = getopt.getopt(sys.argv[1:], "da:p:i:o:q:h")
    except getopt.GetoptError as err:
        # print help information and exit:
        print str(err) # will print something like "option -a not recognized"
        sys.exit(2)
    for o, a in opts:
        if o == "-d":
            debug += 1
        elif o == "-a":
            ip = a
        elif o == "-p":
            port = int(a)
        elif o == "-i":
            infile = open(a, 'r')
        elif o == "-o":
            outfile = open(a, 'w')
        elif o == "-q":
            checkip = a
        else:
            usage()
        
# Set up the socket
    if debug:
        print 'Attempting to connect to {}:{}'.format(ip, port)
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        s.connect((ip, port))
    except socket.error:
        print 'No server listening at {}:{}'.format(ip, port)
        sys.exit(2)

    if infile is not None: # Run in batch mode if we were given an infile
        if debug:
            print 'Running in batch mode'
        for line in infile:
            line = line.rstrip('\n')
            if debug > 1:
                print 'Line is ' + line
                print 'Sending query to server'
            s.send('<query><ip> {} </ip></query>\n'.format(line))
            if debug > 1:
                print 'Waiting for response from server...'
            asn = re.sub(r'<answer><asn> (\d+) </asn></answer>', r'\1', s.makefile().readline())
            outfile.write(line + '\t' + asn)
    elif checkip is not None: # Run in one-off mode if we were given a single query ip address
        if debug:
            print 'Running in one-off mode'
        s.send('<query><ip> {} </ip></query>\n'.format(checkip))
        asn = re.sub(r'<answer><asn> (\d+) </asn></answer>', r'\1', s.makefile().readline())
        print asn
    else:
        usage()

# Close the connection
    s.send('<done />\n')
    s.close()

def usage():
    print 'Usage: {} [-h] [-a server_ipaddress] [-p server_port] [-i infile] [-o outfile] [-q query_ipaddress] [-d]...'.format(sys.argv[0])
    print '\t-h\tview this help'
    print '\t-a\tspecify the IP address of the server to connect to, defaults to 127.0.0.1'
    print '\t-p\tspecify the port of the server to connect to, defaults to 54321'
    print '\t-i\tspecify an input file of IP addresses to query the server with'
    print '\t-o\tspecify an output file for query results, defaults to stdout; only works with -i'
    print '\t-q\tquery a single IP address; cannot be used with -i'
    print '\t-d\tenable debug messages; use -dd for more even more messages'
    sys.exit(2)
        
if __name__ == '__main__':
    main()
