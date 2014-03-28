#! /usr/bin/python
# basic X-GEN processing script in Python
# Change record:
# Syntax:
# xgenproc.py [-dns] [-cfhilmpxy<val>] [dirn]
# i.e. d,n,s are Booleans and the other - options all take values
# and an option that doesn't begin with a minus is a directory name
# Change record:
#	16 Sep 2005: built from xgenproc
#
#	Copyright (c) 2002, Illinois Institute of Technology
#	X-GEN: Crystallographic Data Processing Software
#	author: Andrew J Howard, BCPS Department, IIT
# See the file "LICENSE" for information on usage and redistribution
#	of this file, and for a DISCLAIMER OF ALL WARRANTIES.

import os
import sys
import fpformat
import string
import time
import commands
import getopt
import glob

def cleanrun(xgstring) :
	# generalized routine to carefully run an X-GEN command
	# y = os.fork()
	# if y == -1 :
	#	print 'attempt to fork failed'
	#	return y
	# elif y == 0 :
	#	thisstr = 'xgen ' + xgstring
	#	thistuple = string.split(thisstr, ' ')
	#	os.execvp('xgen', thistuple)
	#	print 'successful run of' + xgstring
	# else :
	#	return 0
	thisstr = 'xgen ' + xgstring
	outv = os.system(thisstr)
	if outv == 0 :
		print 'successful run of ' + xgstring
		return
	else :
		print xgstring + ' returned ' + str(outv)
	

def mygzip(str)	:	# gzip a group of files
	zq = glob.glob(str)
	for zz in zq :
		comd = 'gzip ' + zz
		resu = os.system(comd)
		if resu != 0 :
			diag = 'Unable to ' + comd + '; error code ' + resu
			print diag

def mygunzip(str) :	# gunzip a group of files
	zq = glob.glob(str)
	for zz in zq :
		comd = 'gunzip ' + zz
		resu = os.system(comd)
		if resu != 0 :
			diag = 'Unable to ' + comd + '; error code ' + resu
			print diag

def main (record_list, argv ) :
	docompress = 0	# default is _not_ to compress the data
	stepflip = 0	# default is to leave the sign of the stepsize intact
	datadir = '.'	# default data directory is the current one
	spaceg = 0	# default is to let the program determine the spacegroup
	doregions = 0	# default is don't look for regional offsets
	maxspots = 400	# default: keep looking for spots until we find 400
	multicentroid = 0	# default: get centroids from a single run
	xcal = -1	# don't run calibrate unless x and y are specified
	ycal = -1	# see above
	specfile = ''	# don't specify a filename
	specprefix = ''	# don't specify a prefix
	hifram = -1	# don't specify a maximum frame number
	dohelp = 0	# don't give out help
	doversion = 0	# don't tell us the version number
	dorecenter = 1	# recenter things right after auto-indexing
	lofram = 10000	# don't specify a minimum frame number
	minremap = 2000	# RMSError(X,Y) * 10000 for remapping in integrate
	poltype = 3	# default polarization type = fully polarized
	options, args = getopt.getopt(argv[1:],
	 'c:d:f:h:i:j:k:l:m:n:p:s:v:w:x:y:z',
	 [	'maxspots=','docompress','specfile=','hifram=','spaceg=',
		'minremap=','doregions','lofram=','multicentroid=',
		'dorecenter', 'specprefix=','specflip=','doversion=',
		'xcal=','ycal=','dohelp='
	 ])
	for o,a in options:
		if o in ( '-v', '--version'):
			doversion = 1
		if o in ( '-w', '--help'):
			dohelp = 1
		if o in ( '-c', '--maxspots') :
			maxspots = a
		if o in ( '-d', '--docompress') :
			docompress = 1
		if o in ( '-f', '--specfile') :
			specfile = a
		if o in ( '-h', '--hifram') :
			hifram = a
		if o in ( '-i', '--spaceg') :
			spaceg = a
		if o in ( '-j', '--minremap') :
			minremap = a
		if o in ( '-k', '--minregions') :
			doregions = 1
		if o in ( '-l', '--lofram') :
			lofram = a
		if o in ( '-m', '--multicentroid') :
			multicentroid = a
		if o in ( '-n', '--dorecenter') :
			dorecenter = 0
		if o in ( '-p', '--specprefix') :
			specprefix = a
		if o in ( '-s', '--specflip') :
			specflip = 1
		if o in ( '-x', '--xcal') :
			xcal = a
		if o in ( '-y', '--ycal') :
			ycal = a
		if o in ( '-z', '--polarization_type') :
			poltype = a
	if (len(args) > 0) :
		scan_name = args[0]
		print 'there are ' + len(args) + ' remaining arguments'
	else :
		print 'there are no more arguments'
	# Handle documentational requests
	if dohelp != 0:
		os.system('xgen prhelp xgenproc')
		return 0
	if doversion != 0:
		os.system('xgen prversion xgenproc')
		return 0
	
	# make sure the data directory exists and is a directory
	if os.access(datadir, 4) == False:
		print 'Specified data directory ' + datadir + ' is not readable'
		return 12

	if datadir != '.' :
		print 'Changing directory to ' + datadir
		os.chdir(datadir)	# if the directory exists, cd to it
	else :
		print 'We are already in the right starting directory'
	
	# create a processing directory. If a directory xgen already exists,
	# use xgena; if xgena exists, use xgenb; and so on
	workb = 'xgen'	# base of the work names
	if os.access(workb, 4) == False :
		workdir = workb
	else :
		suffix = 'a'
	lsufstr = 'b c d e f g h i j k l m n o p q r s t u v w x y z ' 
	lsuf = string.split(lsufstr, ' ')
	for nextsuffix in lsuf :
		workdir = workb + suffix
		# nextdir = workb + nextsuffix
		acval = os.access(workdir, 4)
		if acval == False :
			break
		else :
			suffix = nextsuffix
	tocreate = datadir + '/' + workdir
	print 'Creating directory ' + tocreate
	u = os.mkdir(workdir, 0755)
	u = os.chdir(workdir)
	print 'We just changed directories to ' + workdir
	paramname = workdir + '/params'
	u = os.mkdir('params', 0755)
	print 'We just created ' + paramname
	
	xgnowstr = 'xgnow'
	if hifram > 0 :
		xgnowstr = xgnowstr + ' -h' + str(hifram)
	if lofram < 9999 :
		xgnowstr = xgnowstr + ' -l' + str(lofram)
	if specfile != '' :
		xgnowstr = xgnowstr + ' -x' + specfile
	if specprefix != '' :
		xgnowstr = xgnowstr + ' -i' + specprefix
	print 'xgnowstr = ' + xgnowstr
	cleanrun(xgnowstr)
	
	# do the equivalent of `source'ing the command file
	comfiles = glob.glob('*.com')	# look for command files
	for comfil in comfiles :
		fcom = open(comfil, 'r')
		while (1) :
			cfstr = fcom.readline()
			if len(cfstr) < 2 :
				break
			wordsincfstr = string.split(cfstr, ' ')
			if len(wordsincfstr) < 2 :
				break
			elt = string.rstrip(wordsincfstr[1], '\n')
			elements = string.split(elt, '=')
			if len(elements) < 2 :
				break
			os.environ[elements[0]] = elements[1]
			print 'Setting ' + elements[0] + ' to ' + elements[1]
		fcom.close()	# close the file when we're done
		print 'environment variables from ' + comfil + ' set up  '
		break		# we really want the first one only

	# as long as we're dealing with environment variables,
	# we set the polarization type too
	if ((1 <= poltype) and (poltype <= 5)) :
		os.environ['POLARIZATION_TYPE'] = str(poltype)
		print 'Polarization type ' + str(poltype) + ' selected'

	# calibrate the detector if needed
	if xcal > 0 and ycal > 0 :
		calstr = 'calibrate' + ' -j ' + str(xcal) + ' ' + str(ycal)
		cleanrun(calstr)
	
	spotstr = 'spots'
	if maxspots > 0 :
		spotstr = spotstr + ' -c' + str(maxspots)
	if multicentroid > 0 :
		oldcen = '$CENTROIDS'
		divisions = multicentroid
		if divisions > 9 :
			divisions = 9
		totframes = 0
		framename = os.getenv('FRAMES')
		f = open(framename, 'r')
		while f.readline() :
			totframes = totframes + 1
		f.close()
		calclen = totframes / divisions
		ndone = 1
		next = 1
		while ndone <= divisions :
			cname = 'spots' + str(ndone) + '.cen'
			os.environ['CENTROIDS'] = cname
			thisstr = spotstr + ' ' + str(next) + ' ' + str(next)
			cleanrun(thisstr)
			next=next+calclen
			if (next > totframes) :
				break
			ndone=ndone+1
		os.environ['CENTROIDS'] = oldcen
		os.environ['CENOUT'] = oldcen
		thisstr = 'cenmer' + 'spots?.cen'
		
		cleanrun(thisspot)
		zq = glob.glob('spots?.cen') # remove individual .cen files
		for zz in zq :
			os.unlink(zz)
	else :
		cleanrun(spotstr)
	
	refstr = 'refall' + ' -i' + str(spaceg)
	if dorecenter == 0 :
		refstr = refstr + ' -n'
	if stepflip != 0 :
		refstr = refstr + ' -f'
	cleanrun(refstr)
	
	integstr = 'integrate' + ' -j' + str(minremap)
	if docompress > 0 :
		integstr = integstr + ' -d'
	if doregions > 0 :
		integstr = integstr + ' -g12'
	cleanrun(integstr)
	
	zq = glob.glob('*.bgi*')	# get rid of background images
	for zz in zq :
		os.unlink(zz)
	# transfer the .upr files to a subdirectory
	curuparams = os.getenv('UPARAMS')
	newuparams = curuparams + '.s'
	os.rename(curuparams, newuparams)
	allpar = glob.glob('*.upr')
	for zz in allpar :
		newname = 'params/' + zz
		os.rename(zz, newname)
	os.rename(newuparams, curuparams)
	
	cleanrun('reduce')	# convert & filter output of `integrate'
	
	cleanrun('scalem')	# scaling, outlier rejection, ascii output
	
	zq = glob.glob('*.*,[1-9]')
	for zz in zq :
		os.unlink(zz)
	mygzip('params/*')	# compress the .upr files in the subdirectory
	mygzip('*.[asuw]* *.mr* *.ce*') # compress the working files
	mygunzip('*.asc.gz *.sca.gz')	# uncompress the .asc and .sca files
	return 0

nar = len(sys.argv)
main(nar, sys.argv)
