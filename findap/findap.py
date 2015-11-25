#!/usr/bin/python
# -*- coding: UTF-8 -*-
import re
import urllib2
import httplib
import traceback
import socket
import sys

def getHtml(url):
	try:
		page = urllib2.urlopen(url)
	except Exception, e: 
		print 'failed to open %s' %(url)
		print e
		html = "error"
	else: 
		try:
			html = page.read()
		except Exception, e: 
			print 'failed to read %s' %(url)
			print e
			html = "error"
	return html

def getApache(html):
    reg = r'Apache Tomcat'
    apre = re.compile(reg)
    aplist = re.findall(apre,html)
    return len(aplist)


timeout = 2
socket.setdefaulttimeout(timeout)
urllib2.socket.setdefaulttimeout(timeout)
file = open(sys.argv[1])
lines = file.readlines()
i = 0;
total = len(lines)
for  line in lines:
	line = line.split('\n')
	line = line[0]
	i=i+1
	lineout = "%d/%d %s\r\n" %(i,total,line)
	sys.stderr.write(lineout)
	html = getHtml(line)
	result = getApache(html)
	if result > 0:
		print "find Apache Tomact in %s" %(line)
