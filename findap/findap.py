#!/usr/bin/python
# -*- coding: UTF-8 -*-
import re
import urllib2
import httplib
import traceback
import socket
import sys
import syslog
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
file = open("http8080")
lines = file.readlines()
i = 0;
total = len(lines)
syslog.openlog("findap")
for  line in lines:
	line = line.split('\n')
	line = line[0]
	i=i+1
	lineout = "%d/%d %s\r\n" %(i,total,line)
	syslog.syslog(lineout)
	#sys.stderr.write(lineout)
	html = getHtml(line)
	result = getApache(html)
	if result > 0:
		print "find Apache Tomact in %s" %(line)
	else:
		print "not Apache Tomcat in %s" %(line)
