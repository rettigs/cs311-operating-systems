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
    stats = False
    kill = False

# Parse arguments
    try:
        opts, args = getopt.getopt(sys.argv[1:], "da:p:i:skh")
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
        elif o == "-s":
            stats = True
        elif o == "-k":
            kill = True
        else:
            usage()
        
# Set up the socket
    if debug:
        print 'Attempting to connect to {0}:{1}'.format(ip, port)
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        s.connect((ip, port))
    except socket.error:
        print 'No server listening at {0}:{1}'.format(ip, port)
        sys.exit(2)

# Update server with new prefixes/ASNs
    if infile is not None:
        if debug:
            print 'Sending updates to server'
        for line in infile:
            prefix = re.sub(r'(.*) .*', r'\1', line)
            asn = re.sub(r'.* (.*)', r'\1', line)
            if debug > 1:
                print 'Sending update to server with prefix {0} and ASN {1}'.format(prefix, asn)
            s.send('<entry><cidr> {0} </cidr><asn> {1} </asn></entry>'.format(prefix, asn))

# Get usage statistics from server
    if stats:
        if debug:
            print 'Querying server for usage statistics'
        s.send('<stats />')
        if debug:
            print 'Waiting for response from server...'
        line = s.makefile().readline()
        print 'Unique hosts serviced:\t' + re.sub(r'.*<hosts> (\d+) </hosts>.*', r'\1', line)
        print 'Queries answered:\t' + re.sub(r'.*<queries> (\d+) </queries>.*', r'\1', line)
        print 'Prefixes stored:\t' + re.sub(r'.*<prefixes> (\d+) </prefixes>.*', r'\1', line)

# Kill server
    if kill:
        if debug:
            print 'Sending kill signal to server'
        s.send('<terminate />')

# Close the connection
    s.close()

def usage():
    print 'Usage: {0} [-h] [-a server_ipaddress] [-p server_port] [-i infile] [-o outfile] [-s] [-k] [-d]...'.format(sys.argv[0])
    print '\t-h\tview this help'
    print '\t-a\tspecify the IP address of the server to connect to, defaults to 127.0.0.1'
    print '\t-p\tspecify the port of the server to connect to, defaults to 54321'
    print '\t-i\tspecify an input file of CIDR prefixes and ASNs to update the server with'
    print '\t-s\tenable reporting mode; query server for usage statistics'
    print '\t-k\tkill the server'
    print '\t-d\tenable debug messages; use -dd for more even more messages'
    sys.exit(2)
        
if __name__ == '__main__':
    main()
