/* marccd.c -- routines for dealing with Mar CCD data

	Copyright (c) 2002, Illinois Institute of Technology
	X-GEN: Crystallographic Data Processing Software
	author: Andrew J Howard, BCPS Department, IIT
 See the file "LICENSE" for information on usage and redistribution
	of this file, and for a DISCLAIMER OF ALL WARRANTIES.

Change record:
	12 Oct 2009: handle situation where if mch_rotation_range == 0
	 but start_phi != end_phi or start_omega != end_omega
	13 Jun 2008: rename mchread to mch_read so diagnostics work better
	25 Feb 2008: reverse byte-orders in stuffmarxgl and stuffmarxgu
	29 Dec 2007: test for little-endedness rather than forcing it
	18 Aug 2007: manage beam-center values correctly
	19 Apr 2007: reorganize use of "{"
	11 Aug 2006: manage byteswapped formats without specific defines
	24 Jun 2006: include xgimage.h; the structures are there
	22 Apr 2006: edmarccd and other routines added to allow header editing
	 1 Mar 2006: in mchread(), fix BioCARS MarCCD header motor-motion
	20 Nov 2005: inline argument declarations and strict prototyping
	 9 Sep 2004: in mchread(), believe the exposure time only if plausible
	28 Aug 2004: summ_mch(), used only here, declared as static; cosmetics
	13 Aug 2004: run_fmmin, run_fmmax replace other DATARUN elements
	 4 Jun 2004: stepsize sign convention for omega corrected;
		eliminate pause in summ_mch()
	21 May 2004: keep the last 15 characters, not the first 15,
		of the Mar filename in heads->filename
	 2 May 2004: don't save Mar headersize in count1
	21 Jan 2004: license message inserted
	 5 Jan 2004: external variable `heads' omitted
	19 Sep 2003: printf mismatches between xglong and int corrected
		using the IVER(x) macro
	18 Sep 2003: all external X-GEN routines declared here, not in xdefs.h
	 5 Jan 2003: reorganized source-file header
	 1 Oct 2002: (?) set rhdp->run_expos to mahe.mch_exposure_time / 1000
		rather than mahe.mch_integration_time / 1000;
		similar changes to mp->accumsecs and mp->elap100ths
	25 Mar 2002: removed MSI attachments
	24 Nov 1998: in mchread, flip the sign of rhdp->run_stepsize.
	15 Nov 1998: initial version
	
   Purpose: reading and interpreting the header of Mar CCD data
   Cross references: imhead.c

*************************************************************************/
#define		NEED_ERRNO	1
#define		NEED_TIME	1
#include	"xgimage.h"

#define MAXIMAGES	9	/* wild guess */
#define	IVER(x)		(int)(mchp->x)	/* macro to simplify printouts */
#define	TIFFLENGTH	1024	/* length of TIFF header */

extern int edmarccd(char *imname, char *edname);

 typedef struct {
	/* File/header format parameters (256 bytes) */
	xgulong	mch_header_type;	/* magic #-flag for header type */
	char	mch_header_name[16];	/* header name (MMX) */
	xgulong	mch_header_major_version;	/* header_major_version (n.) */
	xgulong	mch_header_minor_version;	/* header_minor_version (.n) */
	xgulong	mch_header_byte_order;	/* BIG_ENDIAN, LITTLE_ENDIAN */
	xgulong	mch_data_byte_order;	/* BIG_ENDIAN, LITTLE_ENDIAN */
	xgulong	mch_header_size;	/* in bytes			*/
	xgulong	mch_frame_type;	/* flag for frame type */
	xgulong	mch_magic_number;	/* flag - usually indicates new file */
	xgulong	mch_compression_type;	/* type of image compression    */
	xgulong	mch_compression1;	/* compression parameter 1 */
	xgulong	mch_compression2;	/* compression parameter 2 */
	xgulong	mch_compression3;	/* compression parameter 3 */
	xgulong	mch_compression4;	/* compression parameter 4 */
	xgulong	mch_compression5;	/* compression parameter 4 */
	xgulong	mch_compression6;	/* compression parameter 4 */
	xgulong	mch_nheaders;	/* total number of headers 	*/
 	xgulong	mch_nfast;	/* number of pixels in one line */
 	xgulong	mch_nslow;	/* number of lines in image     */
 	xgulong	mch_depth;	/* number of bytes per pixel    */
 	xgulong	mch_record_length;	/* # pixels between succesive rows */
 	xgulong	mch_signif_bits;	/* true depth of data, in bits  */
 	xgulong	mch_data_type;	/* (signed,unsigned,float...) */
 	xgulong	mch_saturated_value;	/* value marks pixel as saturated */
	xgulong	mch_sequence;	/* TRUE or FALSE */
	xgulong	mch_nimages;	/* total # of images */
 	xgulong	mch_origin;	/* corner of origin 		*/
 	xgulong	mch_orientation;	/* direction of fast axis 	*/
        xgulong	mch_view_direction;	/* direction to view frame      */
	xgulong	mch_overflow_location;	/* FOLLOWING_HEADER, FOLLOWING_DATA */
	xgulong	mch_over_8_bits;	/* # of pixels with counts > 255 */
	xgulong	mch_over_16_bits;	/* # of pixels with count > 65535 */
	xgulong	mch_multiplexed;	/* multiplex flag */
	xgulong	mch_nfastimages;	/* # of images in fast direction */
	xgulong	mch_nslowimages;	/* # of images in slow direction */
	xgulong	mch_background_applied;	/* flags correction has been applied */
	xgulong	mch_bias_applied;	/* flags correction has been applied*/
	xgulong	mch_flatfield_applied;	/* flags correction has been applied */
	xgulong	mch_distortion_applied;	/* flags correction has been applied */
	xgulong	mch_original_header_type; /* Header/frame type from orig file*/
	xgulong	mch_file_saved;	/* Flag that file has been saved: 0 if modif.*/
	char	mch_reserve1[(64-40)*sizeof(xglong)-16];

	/* Data statistics (128) */
	xgulong	mch_total_counts[2];	/* 64 bit integer range = 1.85E19*/
	xgulong	mch_special_counts1[2];
	xgulong	mch_special_counts2[2];
	xgulong	mch_min;
	xgulong	mch_max;
	xgulong	mch_mean;
	xgulong	mch_rms;
	xgulong	mch_p10;
	xgulong	mch_p90;
	xgulong	mch_stats_uptodate;
        xgulong	mch_pixel_noise[MAXIMAGES];	/* 1000*base noise val(ADUs) */
	char	mch_reserve2[(32-13-MAXIMAGES)*sizeof(xglong)];

	/* More statistics (256) */
	u_short	mch_percentile[128];

	/* Goniostat parameters (128 bytes) */
        xglong	mch_xtal_to_detector;	/* distance in microns */
        xglong	mch_beam_x;		/* 1000*x beam position (pixels) */
        xglong	mch_beam_y;		/* 1000*y beam position (pixels) */
        xglong	mch_integration_time;	/* integration time in milliseconds */
        xglong	mch_exposure_time;	/* exposure time in milliseconds */
        xglong	mch_readout_time;	/* readout time in milliseconds */
        xglong	mch_nreads;		/* # of readouts to get this image */
        xglong	mch_start_twotheta;	/* 1000*two_theta angle */
        xglong	mch_start_omega;	/* 1000*omega angle */
        xglong	mch_start_chi;		/* 1000*chi angle */
        xglong	mch_start_kappa;	/* 1000*kappa angle */
        xglong	mch_start_phi;		/* 1000*phi angle */
        xglong	mch_start_delta;	/* 1000*delta angle */
        xglong	mch_start_gamma;	/* 1000*gamma angle */
        xglong	mch_start_xtal_to_detector;	/* start distance in microns */
        xglong	mch_end_twotheta;	/* 1000*two_theta angle */
        xglong	mch_end_omega;		/* 1000*omega angle */
        xglong	mch_end_chi;		/* 1000*chi angle */
        xglong	mch_end_kappa;		/* 1000*kappa angle */
        xglong	mch_end_phi;		/* 1000*phi angle */
        xglong	mch_end_delta;		/* 1000*delta angle */
        xglong	mch_end_gamma;		/* 1000*gamma angle */
        xglong	mch_end_xtal_to_detector;	/* end distance in microns */
        xglong	mch_rotation_axis;	/* active rotation axis */
        xglong	mch_rotation_range;	/* 1000*rotation angle */
        xglong	mch_detector_rotx;	/* 1000*rot of detector around X */
        xglong	mch_detector_roty;	/* 1000*rot of detector around Y */
        xglong	mch_detector_rotz;	/* 1000*rot of detector around Z */
	char	mch_reserve3[(32-28)*sizeof(xglong)];

	/* Detector parameters (128 bytes) */
	xglong	mch_detector_type;	/* detector type */
	xglong	mch_pixelsize_x;	/* pixel size (nanometers) */
	xglong	mch_pixelsize_y;	/* pixel size (nanometers) */
        xglong	mch_mean_bias;	/* 1000*mean bias value */
        xglong	mch_photons_per_100adu;	/* photons / 100 ADUs */
        xglong	mch_measured_bias[MAXIMAGES];	/* 1000*mean bias value */
        xglong	mch_measured_temperature[MAXIMAGES];	/* det.temp, mK */
        xglong	mch_measured_pressure[MAXIMAGES];	/* chamber press,uTor*/

	/* X-ray source and optics parameters (128 bytes) */
	/* X-ray source parameters (8*4 bytes) */
        xglong	mch_source_type;	/* (code) - target, synch. etc */
        xglong	mch_source_dx;	/* Optics param. - (size microns) */
        xglong	mch_source_dy;	/* Optics param. - (size microns) */
        xglong	mch_source_wavelength;	/* wavelength (femtoMeters) */
        xglong	mch_source_power;	/* (Watts) */
        xglong	mch_source_voltage;	/* (Volts) */
        xglong	mch_source_current;	/* (microAmps) */
        xglong	mch_source_bias;	/* (Volts) */
        xglong	mch_source_polarization_x;	/* () */
        xglong	mch_source_polarization_y;	/* () */
	char	mch_reserve_source[4*sizeof(xglong)];

	/* X-ray optics_parameters (8*4 bytes) */
        xglong	mch_optics_type;	/* Optics type (code)*/
        xglong	mch_optics_dx;	/* Optics param. - (size microns) */
        xglong	mch_optics_dy;	/* Optics param. - (size microns) */
        xglong	mch_optics_wavelength;	/* Optics param. - (size microns) */
        xglong	mch_optics_dispersion;	/* Optics param. - (*10E6) */
        xglong	mch_optics_crossfire_x;	/* Optics param. - (microRadians) */
        xglong	mch_optics_crossfire_y;	/* Optics param. - (microRadians) */
        xglong	mch_optics_angle;	/* (monoch. 2theta - microradians) */
        xglong	mch_optics_polarization_x;	/* () */
        xglong	mch_optics_polarization_y;	/* () */
	/* ALTERATION FROM ORIGINAL:
	char	mch_reserve_optics[4*sizeof(xglong)];
	*/
	xglong	mch_optics_transmission;	/* transmission fraction*10E6*/
	char	mch_reserve_optics[3*sizeof(xglong)];

	char	mch_reserve5[((32-28)*sizeof(xglong))];

	/* File parameters (1024 bytes) */
	char	mch_filetitle[128];		/* Title */
	char	mch_filepath[128];		/* path name for data file */
	char	mch_filename[64];		/* name of data file */
        char	mch_acquire_timestamp[32];	/* date, time of acquisition */
        char	mch_header_timestamp[32];	/* date, time of hdr update */
        char	mch_save_timestamp[32];		/* date and time file saved */
        char	mch_file_comments[512];		/* comments */ 
	char	mch_reserve6[1024-(128+128+64+(3*32)+512)];

	/* Dataset parameters (512 bytes) */
        char	mch_dataset_comments[512];	/* comments */

	char	mch_pad[3072-(256+128+256+(3*128)+1024+512)];	/* pad->3072 */
	} MARCCDHEAD;

#define		MCHBUFSIZ	128

static void summ_mch(MARCCDHEAD *mchp, FILE *fp)
{ /*	This summarizes the contents of a Mar CCD header to stream fp */
	int	i, j;

	fprintf(fp, "header type = %d\n", IVER(mch_header_type));
	fprintf(fp, "header name = (%s)\n", mchp->mch_header_name);
	fprintf(fp, "major, minor version = %d, %d\n",
	 IVER(mch_header_major_version), IVER(mch_header_minor_version));
	fprintf(fp, "header, data byte-order = %d, %d\n",
	 IVER(mch_header_byte_order), IVER(mch_data_byte_order));
	fprintf(fp, "header size = %d\n", IVER(mch_header_size));
	fprintf(fp, "frame type, magic number = %d, %d\n",
	 IVER(mch_frame_type), IVER(mch_magic_number));
	fprintf(fp, "compression type, params = %d, %d, %d, %d, %d, %d, %d\n",
	 IVER(mch_compression_type),
	 IVER(mch_compression1), IVER(mch_compression2),
	 IVER(mch_compression3), IVER(mch_compression4),
	 IVER(mch_compression5), IVER(mch_compression6));
	fprintf(fp, "nheaders = %d\n", IVER(mch_nheaders));
	fprintf(fp, "#pixels fast, slow; depth = %d, %d, %d\n",
	 IVER(mch_nfast), IVER(mch_nslow), IVER(mch_depth));
	fprintf(fp, "record length = %d\n", IVER(mch_record_length));
	fprintf(fp, "signif bits, data_type, saturated val = %d, %d, %d\n",
	 IVER(mch_signif_bits), IVER(mch_data_type), IVER(mch_saturated_value));
	fprintf(fp, "sequence, nimages = %d, %d\n",
	 IVER(mch_sequence), IVER(mch_nimages));
	fprintf(fp, "origin, orientation, view-direction = %d, %d, %d\n",
	 IVER(mch_origin), IVER(mch_orientation),  IVER(mch_view_direction));
	fprintf(fp, "overflow position = %d\n", IVER(mch_overflow_location));
	fprintf(fp, "over 8, 16 bits: %d, %d\n",
	 IVER(mch_over_8_bits), IVER(mch_over_16_bits));
	fprintf(fp, "multiplexed, nfastimages, nslowimages = %d, %d, %d\n",
	 IVER(mch_multiplexed), IVER(mch_nfastimages), IVER(mch_nslowimages));
	fprintf(fp,
	 "background,bias,flatfield,distortion applied = %d,%d,%d,%d\n",
		IVER(mch_background_applied), IVER(mch_bias_applied),
		IVER(mch_flatfield_applied), IVER(mch_distortion_applied));
	fprintf(fp, "original header type = %d; file saved = %d\n",
		IVER(mch_original_header_type), IVER(mch_file_saved));
	fprintf(fp, "total, special counts = (%d,%d), (%d,%d), (%d,%d)\n",
	 IVER(mch_total_counts[0]), IVER(mch_total_counts[1]),
	 IVER(mch_special_counts1[0]), IVER(mch_special_counts1[1]),
	 IVER(mch_special_counts2[0]), IVER(mch_special_counts2[1]));
	fprintf(fp,
	 "min,max,mean,rms,p10,p90,stats_uptodate = %d,%d,%d,%d,%d,%d,%d\n",
	 IVER(mch_min), IVER(mch_max), IVER(mch_mean), IVER(mch_rms),
	 IVER(mch_p10), IVER(mch_p90), IVER(mch_stats_uptodate));
	fprintf(fp, "pixel noise: ");
	for (i = 0; i < MAXIMAGES; i++)
		fprintf(fp, " %7d", IVER(mch_pixel_noise[i]));
	fprintf(fp, "\n"); 
	fprintf(fp, "percentiles:\n");
	for (i = 0; i < 16; i++)
	   {
		for (j = 0; j < 8; j++) fprintf(fp, " %8d",
			IVER(mch_percentile[i*8 + j]));
		fprintf(fp, "\n");
	   }
	fprintf(fp, "xtal_to_detector, beam_x, beam_y = %d, %d, %d\n",
	 IVER(mch_xtal_to_detector), IVER(mch_beam_x), IVER(mch_beam_y));
	fprintf(fp, "times: integration, exposure, readout = %d, %d, %d\n",
	 IVER(mch_integration_time), IVER(mch_exposure_time),
	 IVER(mch_readout_time));
	fprintf(fp, "nreads = %d\n", IVER(mch_nreads));
	fprintf(fp,
 "start 2th,omega,chi,kappa,phi,delta,gamma,dist=%d,%d,%d,%d,%d,%d,%d,%d\n",
	 IVER(mch_start_twotheta), IVER(mch_start_omega),
	 IVER(mch_start_chi), IVER(mch_start_kappa),
	 IVER(mch_start_phi), IVER(mch_start_delta),
	 IVER(mch_start_gamma), IVER(mch_start_xtal_to_detector));
	fprintf(fp,
 "  end 2th,omega,chi,kappa,phi,delta,gamma,dist=%d,%d,%d,%d,%d,%d,%d,%d\n",
	 IVER(mch_end_twotheta), IVER(mch_end_omega),
	 IVER(mch_end_chi), IVER(mch_end_kappa),
	 IVER(mch_end_phi), IVER(mch_end_delta),
	 IVER(mch_end_gamma), IVER(mch_end_xtal_to_detector));
	fprintf(fp, "rotation axis, range = %d, %d\n",
	 IVER(mch_rotation_axis), IVER(mch_rotation_range));
	fprintf(fp, "detector rotx,y,z = %d, %d, %d\n",
	 IVER(mch_detector_rotx), IVER(mch_detector_roty),
	 IVER(mch_detector_rotz));
	fprintf(fp, "detector type = %d; pixel size (x,y) = %d, %d\n",
	 IVER(mch_detector_type),IVER(mch_pixelsize_x),IVER(mch_pixelsize_y));
	fprintf(fp, "mean bias, photons/100adu = %d, %d\n",
	 IVER(mch_mean_bias), IVER(mch_photons_per_100adu));
	fprintf(fp, "measured bias, temperature pressure on each image:\n");
	for (i = 0; i < MAXIMAGES; i++)
		fprintf(fp, " %6d", IVER(mch_measured_bias[i]));
	fprintf(fp, "\n");
	for (i = 0; i < MAXIMAGES; i++)
		fprintf(fp, " %6d", IVER(mch_measured_temperature[i]));
	fprintf(fp, "\n");
	for (i = 0; i < MAXIMAGES; i++)
		fprintf(fp, " %6d", IVER(mch_measured_pressure[i]));
	fprintf(fp, "\n");
	fprintf(fp, "source type, dx, dy, wavelength = %d, %d, %d, %d\n",
	 IVER(mch_source_type), IVER(mch_source_dx), IVER(mch_source_dy),
	 IVER(mch_source_wavelength));
	fprintf(fp, "source power, voltage, current, bias = %d, %d, %d, %d\n",
	 IVER(mch_source_power), IVER(mch_source_voltage),
	 IVER(mch_source_current), IVER(mch_source_bias));
	fprintf(fp, "source polarization in x, y = %d, %d\n",
	 IVER(mch_source_polarization_x), IVER(mch_source_polarization_y));
	fprintf(fp,
 "optics type, dx, dy, wavelength, dispersion = %d, %d, %d, %d, %d\n",
	 IVER(mch_optics_type), IVER(mch_optics_dx),
	 IVER(mch_optics_dy), IVER(mch_optics_wavelength),
	 IVER(mch_optics_dispersion));
	fprintf(fp, "optics crossfire: x,y = %d, %d\n",
	 IVER(mch_optics_crossfire_x), IVER(mch_optics_crossfire_y));
	fprintf(fp, "optics angle = %d\n", IVER(mch_optics_angle));
	fprintf(fp, "optics polarization: x,y = %d, %d\n",
	 IVER(mch_optics_polarization_x), IVER(mch_optics_polarization_y));
	fprintf(fp, "file title = (%s)\n", mchp->mch_filetitle);
	fprintf(fp, "file path = (%s)\n", mchp->mch_filepath);
	fprintf(fp, "file name = (%s)\n", mchp->mch_filename);
	fprintf(fp, "acquire timestamp = (%s)\n", mchp->mch_acquire_timestamp);
	fprintf(fp, "header timestamp = (%s)\n", mchp->mch_header_timestamp);
	fprintf(fp, "save timestamp = (%s)\n", mchp->mch_save_timestamp);
	fprintf(fp, "file comments = (%s)\n", mchp->mch_file_comments);
	fprintf(fp, "dataset comments = (%s)\n", mchp->mch_dataset_comments);
	fprintf(fp, "header-read completed\n");
	return;
}

int mch_read(MAIL *mp, DATARUN *rhdp, int verboseread)
{ /*	This reads a Mar CCD header */
	size_t			i, ntoread, fnlen;
	time_t			tl0;
	int			swfmt, tfmt, delmov;
	char			*cp;
	xgulong			*ulp;
	xglong			*lp;
	u_short			*usp;
	float			readexpos, sinthmax, tantwothmax;
	static char		mchbuf[MCHBUFSIZ];	/* local scratch */
	/* converts from Mar's motor-numbering system to mine */
	static char		motorconv[7] = { 3, 0, 1, 1, 2, 0, 0 } ;
	static MARCCDHEAD	mahe;

	/* implicit seek to the end of the tiff part of the header. We've
	already read 128 bytes, so we need 1024-128 = 896 = 7*128 */
	for (i = 128; i < TIFFLENGTH; i += MCHBUFSIZ)
	   if (MCHBUFSIZ != imread(mchbuf, MCHBUFSIZ))
	     {
		fprintf(stderr,
		 "Error skipping tif header element %d\n", (int)i);
		return FRAME_BADMAIL;
	     }
	/* Mar information itself */
	ntoread = sizeof (MARCCDHEAD);
	if (ntoread != (i = imread((AVOID *)&mahe, ntoread)))
	  {
		fprintf(stderr,
		 "Error reading header data: expected %d bytes, got %d\n",
			(int)ntoread, (int)i);
		return FRAME_BADMAIL;
	  }
	/* word- and byte-swap if needed */
	tfmt = mp->format;	swfmt = swawb(tfmt);
	if ((swfmt == FMT_MARCCD) || (tfmt == FMT_MARCCD_L))
	  {
		mahe.mch_header_type = swawb(mahe.mch_header_type);
		for (ulp = &(mahe.mch_header_minor_version);
		 ulp <= &(mahe.mch_file_saved); ulp++) *ulp = swawb(*ulp);
		for (ulp = &(mahe.mch_total_counts[0]);
		 ulp <= &(mahe.mch_pixel_noise[MAXIMAGES-1]); ulp++)
			*ulp = swawb(*ulp);
		for (usp = &(mahe.mch_percentile[0]);
		 usp <= &(mahe.mch_percentile[127]); usp++) *usp = uswab(*usp);
		for (lp = &(mahe.mch_xtal_to_detector);
		 lp <= &(mahe.mch_optics_polarization_y); lp++)
			*lp = lswawb(*lp, 1);
	  }
	mahe.mch_header_name[15] = '\0';	/* null-terminate */
	/* any diagnostic printouts desired would come here */
	if (verboseread) summ_mch(&mahe, stdout);
	/* convert to ordinary MAIL stuff */
	mp->xdimen = mahe.mch_nfast;
	mp->ydimen = mahe.mch_nslow;
	mp->quadrant = 0;
	if (!mahe.mch_total_counts[0])
	  {
		if (mahe.mch_total_counts[1])
			mp->eventcount = mahe.mch_total_counts[1];
		else	mp->eventcount = 10000000;
	  }
	else	mp->eventcount = 100000000; /* temporary ! */
	mp->over64k = mp->oflowcount = mahe.mch_over_16_bits;
	mp->zmax = mahe.mch_max; /* probably overridden later... */
	mp->zmaxx = mp->xdimen / 2;	mp->zmaxy = mp->ydimen / 2; /* temp */
	mp->ontime = mp->eventcount;
	mp->count1 = mp->late = 0;
	(void)timecvt(mp->datetime, mahe.mch_header_timestamp,
		NULL, &tl0, 10, 0);
	/* Sep 2004: distrust exposure time if it's huge */
	if ((0.5 <= (readexpos = mahe.mch_exposure_time)) &&
		(readexpos < 8.64e7))
	  {
		mp->accumsecs = (xgulong)(1.e-3 * readexpos + 0.49999);
		mp->elap100ths = (xgulong)(0.1 * readexpos + 0.49999);
	  }
	else {
		mp->accumsecs = 10;	mp->elap100ths = 1000;
	     }
	mp->cellsize = (xgulong)(mahe.mch_pixelsize_x * 1.e-3 + 0.49999);
	mp->nonzpix = 0;
	mp->over64k = mp->oflowcount;
	if ((swfmt == FMT_MARCCD) || (tfmt == FMT_MARCCD_L))
		mp->improps = IPR_BYTESWAPPED | IPR_NEEDCOUNT;
	else	mp->improps = IPR_NEEDCOUNT;
	mp->frameno = 1;	/* temporary */
	/* generally the most significant portions of the filename
	 are at the end, not at the beginning */
	if (15 < (fnlen = strlen(mahe.mch_filename)))
	  {
		cp = &(mahe.mch_filename[fnlen - 15]);
		strncpy(mp->filename, cp, 15);
	  }
	else strncpy(mp->filename, mahe.mch_filename, 15);
	mp->filename[15] = '\0';
	mp->count0 = mahe.mch_readout_time;
	/* we used to keep the Mar header size in mp->count1.
	 That isn't the right way to remember that number.
	mp->count1 = mahe.mch_header_size; */
	mp->filename[15] = '\0';
	/* also create the RUNHEADER stuff */
	zeromem((AVOID *)rhdp, sizeof (DATARUN));
	strncpy(rhdp->run_code, mahe.mch_filename, 7);
	rhdp->run_code[7] = '\0';
	strncpy(rhdp->run_notes, mahe.mch_file_comments, 63);
	rhdp->run_notes[63] = '\0';
	for (cp = rhdp->run_notes; '\0' != *cp; cp++)
		if (*cp == '\n') *cp = ';';
	mahe.mch_filename[63] = '\0';
	for (i = 0, cp = mahe.mch_filename; (i < 63) && ('\0' != *cp);
	 i++, cp++) if (('0' <= *cp) && (*cp <= '9')) break;
	if ((i < 62) && ('\0' != *cp)) rhdp->run_id = atoi(cp);
	rhdp->run_volxray = mahe.mch_source_voltage / 1000;
	rhdp->run_curxray = mahe.mch_source_current / 1000;
	rhdp->run_chvol = mahe.mch_source_bias / 1000;
	rhdp->run_stepsize = -(mahe.mch_rotation_range * 96) / 1000;
	rhdp->run_nframes = 360;	/* temporary */
	rhdp->run_fmmin = 1;	rhdp->run_fmmax = 360;
	/* The BioCARS version of the data acquisition software appears
	 to identify omega as the starting motor even when it's really phi */
	if (mahe.mch_start_omega == mahe.mch_end_omega)
		mahe.mch_rotation_axis = 4;
	rhdp->run_motor = motorconv[mahe.mch_rotation_axis];
	rhdp->run_nshifts = 150;	/* arbitrary */
	rhdp->run_nchams = mahe.mch_nfastimages;
	rhdp->run_temp = 300;	/* since Mar doesn't record it */
	rhdp->run_width = mahe.mch_nfast;
	rhdp->run_height = mahe.mch_nslow;
	rhdp->run_lambda = ((float)mahe.mch_source_wavelength) * 1.e-5;
	rhdp->run_startpos[0] = ((float)mahe.mch_start_omega) * 1.e-3;
	rhdp->run_startpos[1] = ((float)mahe.mch_start_chi) * 1.e-3;
	rhdp->run_startpos[2] = ((float)mahe.mch_start_phi) * 1.e-3;
	if ((0.5 <= readexpos) && (readexpos < 8.64e7)) 
		rhdp->run_expos = readexpos * 1.e-3;
	else	rhdp->run_expos = 10;
	rhdp->run_d0pos[0] = mahe.mch_start_xtal_to_detector * 1.e-4;
	rhdp->run_d0pos[1] = 1.e-7 * (((double)(mahe.mch_beam_y)) * 1.e-3 -
		0.5 * ((double)(mahe.mch_nfast))) *
		((double)(mahe.mch_pixelsize_y));
	rhdp->run_d0pos[2] = 1.e-7 * (((double)(mahe.mch_beam_x)) * 1.e-3 -
		0.5 * ((double)(mahe.mch_nslow))) *
		((double)(mahe.mch_pixelsize_x));
	rhdp->run_d0ang[0] = mahe.mch_start_twotheta * 1.e-3;
	rhdp->run_d0ang[1] = mahe.mch_detector_rotx;
	rhdp->run_d0ang[2] = mahe.mch_detector_roty;
	rhdp->run_d1pos[1] = mahe.mch_pixelsize_x * 1.e-6; /* pixel size, mm*/
	rhdp->run_d1pos[2] = mahe.mch_pixelsize_y * 1.e-6; /* pixel size, mm*/
	rhdp->run_stepreal = -mahe.mch_rotation_range * 1.e-3;
	if (fabs(rhdp->run_stepreal) < FUZZ)
	  { /* if mch_rotation_range isn't set right, get it from differences */
		if ((delmov = (int)(mahe.mch_end_phi - mahe.mch_start_phi))
		 != 0) rhdp->run_stepreal = -((double)delmov) * 1.e-3;
		else if ((delmov = (int)(mahe.mch_end_omega -
		 mahe.mch_start_omega)) != 0)
			rhdp->run_stepreal = -((double)delmov) * 1.e-3;
		rhdp->run_stepsize = 96. * rhdp->run_stepreal;
	  }
	tantwothmax = 1.e-3 * ((float)mahe.mch_pixelsize_x) *
	 ((float)mahe.mch_nfast) /
	 (2. * ((float)mahe.mch_start_xtal_to_detector));
	sinthmax = sin(0.5 * atan(tantwothmax));
	rhdp->run_ssqmax = 4. * sinthmax * sinthmax;
	if (rhdp->run_ssqmax > FUZZ) rhdp->run_reslim = 1.e-5 *
	 ((float)mahe.mch_source_wavelength) / sqrt(rhdp->run_ssqmax);
	else rhdp->run_reslim = 2.;
	/* correct sign convention for omega runs */
	if (mahe.mch_rotation_axis == 1)
	  {
		rhdp->run_stepreal *= -1.;
		rhdp->run_stepsize *= -1;
		rhdp->run_d0ang[0] *= -1.;
	  }
	return EOK;
}

static int copysome(char *out, char *in, int maxch)
{ /* copies at most maxch characters from in to out and returns 1 */
	if (strlen(in) > maxch) *(in + maxch) = '\0';
	strcpy(out, in);	return 1;
}

static int appendsome(char *out, char *in, int maxch)
{ /* appends characters from in onto out, with a maximum size of maxch */
	char	*cpo;
	int	wasnl;
	size_t	outlen, inuse;

	outlen = strlen(out);
		if (outlen > maxch - 1) return 1;
	if (outlen)
	  {
		if (outlen > maxch - 1) return 1;
		cpo = out + outlen - 1;
		inuse = maxch - outlen - 1;
		if (*cpo == '\n')
		  {
			wasnl = 1;	cpo++;
		  }
		else { 
			cpo++;	*cpo++ = '\n';	wasnl = 0;
		     }
	  }
	else {
		cpo = out;	inuse = maxch;	wasnl = 0;
	     }
	if (strlen(in) > inuse) *(in + inuse) = '\0';
	strcpy(cpo, in);	if (wasnl) strcat(cpo, "\n");	return 1;
}

static int stuffmarxgu(int ival, xgulong *xgulp)
{ /* If necessary, this does the byteswap required for recording an xgulong
	header value. It then puts it in the right place and returns 1 */
	int	ulittle;
	xgulong	tval;

	ulittle = islittle();
	tval = (ulittle) ? ival : swawb((xglong)ival);
	*xgulp = tval;	return 1;
}

static int stuffmarxgl(int ival, xglong *xglp)
{ /* If necessary, this does the byteswap required for recording an
	xglong header value. It then puts it in the right place and returns 1 */
	int	ulittle;
	xglong	tval;

	ulittle = islittle();
	tval = (ulittle) ? ival : lswawb((xglong)ival, 1);
	*xglp = tval;	return 1;
}

static char *marstamp(void)
{ /* This returns the current time in Mar timestamp format */
	time_t		tl0;
	static char	cti[26], cto[16];

	tl0 = NOW;	strcpy(cti, ctime(&tl0));
	zeromem((AVOID *)cto, 16);
	/* months are ugly */
	cto[0] = 0;
	if (cti[4] == 'J')
		cto[1] = (cti[5] == 'a') ? 1 : ((cti[6] == 'l') ? 7 : 6);
	else if (cti[4] == 'F') cto[1] = 2;
	else if (cti[4] == 'M') cto[1] = (cti[6] == 'r') ? 3 : 5;
	else if (cti[4] == 'A') cto[1] = (cti[5] == 'p') ? 4 : 8;
	else if (cti[4] == 'S') cto[1] = 9;
	else if (cti[4] == 'O') cto[1] = 0;
	else {
		cto[0] = 1;
		cto[1] = (cti[4] == 'O') ? 0 : ((cti[4] == 'N') ? 1 : 2);
	     }
	cto[0] += '0';	cto[1] += '0';
	cto[2] = cti[8];	cto[3] = cti[9];
	cto[4] = cti[11];	cto[5] = cti[12];
	cto[6] = cti[14];	cto[7] = cti[15];
	cto[8] = cti[20];	cto[9] = cti[21];
	cto[10] = cti[22];	cto[11] = cti[23];
	cto[12] = '.';	cto[13] = cti[17];	cto[14] = cti[18];
	cto[15] = '\0';	return cto;
}

static int matchmar(char *eltname, char *contents, MARCCDHEAD *mchp)
{ /* This alters a specified element of a Mar header.
	It returns 1 if successful, 0 otherwise */

	int		ival, ul;
	char		*eltl10;

	ival = atoi(contents);
	eltl10 = eltname + strlen(eltname) - 10;
	ul = islittle();
	if (!strncmp(eltname, "beam_", 5))
	  {
		if (!strcmp(eltname, "beam_x"))
			return stuffmarxgl(ival, &(mchp->mch_beam_x));
		else if (!strcmp(eltname, "beam_y"))
			return stuffmarxgl(ival, &(mchp->mch_beam_y));
		else return 0;
	  }
	else if (!strncmp(eltname, "start_", 6))
	  {
		if (!strcmp(eltname, "start_twotheta"))
			return stuffmarxgl(ival, &(mchp->mch_start_twotheta));
		else if (!strcmp(eltname, "start_omega"))
			return stuffmarxgl(ival, &(mchp->mch_start_omega));
		else if (!strcmp(eltname, "start_chi"))
			return stuffmarxgl(ival, &(mchp->mch_start_chi));
		else if (!strcmp(eltname, "start_kappa"))
			return stuffmarxgl(ival, &(mchp->mch_start_kappa));
		else if (!strcmp(eltname, "start_phi"))
			return stuffmarxgl(ival, &(mchp->mch_start_phi));
		else if (!strcmp(eltname, "start_delta"))
			return stuffmarxgl(ival, &(mchp->mch_start_delta));
		else if (!strcmp(eltname, "start_gamma"))
			return stuffmarxgl(ival, &(mchp->mch_start_gamma));
		else if (!strcmp(eltname, "start_xtal_to_detector")) return
		 stuffmarxgl(ival, &(mchp->mch_start_xtal_to_detector));
		else return 0;
	  }
	else if (!strncmp(eltname, "end_", 4))
	  {
		if (!strcmp(eltname, "end_twotheta"))
			return stuffmarxgl(ival, &(mchp->mch_end_twotheta));
		else if (!strcmp(eltname, "end_omega"))
			return stuffmarxgl(ival, &(mchp->mch_end_omega));
		else if (!strcmp(eltname, "end_chi"))
			return stuffmarxgl(ival, &(mchp->mch_end_chi));
		else if (!strcmp(eltname, "end_kappa"))
			return stuffmarxgl(ival, &(mchp->mch_end_kappa));
		else if (!strcmp(eltname, "end_phi"))
			return stuffmarxgl(ival, &(mchp->mch_end_phi));
		else if (!strcmp(eltname, "end_delta"))
			return stuffmarxgl(ival, &(mchp->mch_end_delta));
		else if (!strcmp(eltname, "end_gamma"))
			return stuffmarxgl(ival, &(mchp->mch_end_gamma));
		else if (!strcmp(eltname, "end_xtal_to_detector"))
		 return stuffmarxgl(ival, &(mchp->mch_end_xtal_to_detector));
		else return 0;
	  }
	else if (!strncmp(eltname, "rotation_", 9))
	  {
		if (!strcmp(eltname, "rotation_axis"))
			return stuffmarxgl(ival, &(mchp->mch_rotation_axis));
		else if (!strcmp(eltname, "rotation_range"))
			return stuffmarxgl(ival, &(mchp->mch_rotation_range));
		else return 0;
	  }
	else if (!strncmp(eltname, "detector_", 9))
	  {
		if (!strcmp(eltname, "detector_rotx"))
			return stuffmarxgl(ival, &(mchp->mch_detector_rotx));
		else if (!strcmp(eltname, "detector_roty"))
			return stuffmarxgl(ival, &(mchp->mch_detector_roty));
		else if (!strcmp(eltname, "detector_rotz"))
			return stuffmarxgl(ival, &(mchp->mch_detector_rotz));
		else return 0;
	  }
	else if (!strcmp(eltname, "mean_bias"))
	  {
		return stuffmarxgl(ival, &(mchp->mch_mean_bias));
	  }
	else if (!strcmp(eltname, "photons_per_100adu"))
	  {
		return stuffmarxgl(ival, &(mchp->mch_photons_per_100adu));
	  }
	else if (!strncmp(eltname, "source_", 7))
	  {
		if (!strcmp(eltname, "source_type"))
			return stuffmarxgl(ival, &(mchp->mch_source_type));
		else if (!strcmp(eltname, "source_dx"))
			return stuffmarxgl(ival, &(mchp->mch_source_dx));
		else if (!strcmp(eltname, "source_dy"))
			return stuffmarxgl(ival, &(mchp->mch_source_dy));
		else if (!strcmp(eltname, "source_wavelength"))
		 return stuffmarxgl(ival, &(mchp->mch_source_wavelength));
		else if (!strcmp(eltname, "source_power"))
			return stuffmarxgl(ival, &(mchp->mch_source_power));
		else if (!strcmp(eltname, "source_voltage"))
			return stuffmarxgl(ival, &(mchp->mch_source_voltage));
		else if (!strcmp(eltname, "source_current"))
			return stuffmarxgl(ival, &(mchp->mch_source_current));
		else if (!strcmp(eltname, "source_bias"))
			return stuffmarxgl(ival, &(mchp->mch_source_bias));
		else if (!strcmp(eltname, "source_polarization_x"))
		 return stuffmarxgl(ival, &(mchp->mch_source_polarization_x));
		else if (!strcmp(eltname, "source_polarization_y"))
		 return stuffmarxgl(ival, &(mchp->mch_source_polarization_y));
		else return 0;
	  }
	else if (!strncmp(eltname, "optics_", 7))
	  {
		if (!strcmp(eltname, "optics_type"))
			return stuffmarxgl(ival, &(mchp->mch_optics_type));
		else if (!strcmp(eltname, "optics_dx"))
			return stuffmarxgl(ival, &(mchp->mch_optics_dx));
		else if (!strcmp(eltname, "optics_dy"))
			return stuffmarxgl(ival, &(mchp->mch_optics_dy));
		else if (!strcmp(eltname, "optics_wavelength"))
		 return stuffmarxgl(ival, &(mchp->mch_optics_wavelength));
		else if (!strcmp(eltname, "optics_dispersion"))
		 return stuffmarxgl(ival, &(mchp->mch_optics_dispersion));
		else if (!strcmp(eltname, "optics_crossfire_x"))
		 return stuffmarxgl(ival, &(mchp->mch_optics_crossfire_x));
		else if (!strcmp(eltname, "optics_crossfire_y"))
		 return stuffmarxgl(ival, &(mchp->mch_optics_crossfire_y));
		else if (!strcmp(eltname, "optics_angle"))
			return stuffmarxgl(ival, &(mchp->mch_optics_angle));
		else if (!strcmp(eltname, "optics_polarization_x"))
		 return stuffmarxgl(ival, &(mchp->mch_optics_polarization_x));
		else if (!strcmp(eltname, "optics_polarization_y"))
		 return stuffmarxgl(ival, &(mchp->mch_optics_polarization_y));
		else if (!strcmp(eltname, "optics_transmission"))
		 return stuffmarxgl(ival, &(mchp->mch_optics_transmission));
		else return 0;
	  }
	else if (!strncmp(eltname, "file", 4))
	  {
		if (!strcmp(eltname, "file_saved"))
			return stuffmarxgu(ival, &(mchp->mch_file_saved));
		else if (!strcmp(eltname, "filetitle"))
			return copysome(mchp->mch_filetitle, contents, 127);
		else if (!strcmp(eltname, "filepath"))
			return copysome(mchp->mch_filepath, contents, 127);
		else if (!strcmp(eltname, "filename"))
			return copysome(mchp->mch_filename, contents, 63);
		else if (!strcmp(eltname, "file_comments")) return appendsome(
			mchp->mch_file_comments, contents, 511);
		else return 0;
	  }
	else if ((strlen(eltname) > 10) && !strncmp(eltl10, "_timestamp", 10))
	  {
		if (!strcmp(eltname, "header_timestamp")) return copysome(
			mchp->mch_header_timestamp, contents, 31);
		else if (!strcmp(eltname, "save_timestamp")) return copysome(
			mchp->mch_save_timestamp, contents, 31);
		else return 0;
	   }
	else if (!strcmp(eltname, "dataset_comments"))
		return appendsome(mchp->mch_dataset_comments, contents, 511);
	else return 0;
}

int edmarccd(char *imname, char *edname)
{ /* This alters the Mar header in the file with name imname,
	using editing commands found in the file with name edname.
	edname contains Mar header elements written in the form
	headerelement=value
		e.g.
	rotation_range=250
		would change mchp->mch_rotation_range to 250.
	Only the header elements found in the list contained within `matchmar'
	are currently allowed to be edited; obviously others could be added. */

	int			fd, nchanged;
	size_t			marhsize;
	char			*cpi, *cpo, *contents;
	static char		linecon[514], eltname[64];
	static FILE		*fpmar, *fpte;
	static MARCCDHEAD	mahe;

	errno = 0;
	if (NULL == (fpmar = xgopen(imname, "r+"))) /* open Mar file */
	  {
		fprintf(stderr,
		 "Cannot open %s as MarCCD image file; errno = %d\n",
			imname, errno);
		return EOPENERR;
	  }
	if (NULL == (fpte = xgopen(edname, "r"))) /* open editing file */
	  {
		fprintf(stderr,
		 "Cannot open %s as mar header editing file; errno = %d\n",
		 edname, errno);
		return EOPENERR;
	  }
	fd = fileno(fpmar);
	if (-1 == lseek(fd, TIFFLENGTH, 0))
	  {
		fprintf(stderr,
		 "Error seeking to end of Tiff header in %s; errno = %d\n",
			imname, errno);
		return ESEEKERR;
	  }
	marhsize = sizeof (MARCCDHEAD);
	if (1 != fread((AVOID *)&mahe, marhsize, 1, fpmar))
	  {
		fprintf(stderr,
		 "Error reading in MarCCD header in %s; errno = %d\n",
			imname, errno);
		return EREADERR;
	  }
	nchanged = 0;
	while (NULL != fgets(linecon, 512, fpte)) /* read lines from file */
	     {
		zeromem(eltname, 64);	/* blank out element-name array */
		for (cpi = linecon; *cpi <= ' '; cpi++) ; /* ignore blanks */
		for (cpo = eltname; '\0' != *cpi; cpi++, cpo++)
		   {
			if ((*cpi == '=') || (*cpi == ':')) break;
			*cpo = *cpi;
		   }
		*cpo = '\0'; /* null-terminate element name */
		contents = cpi + 1;	/* remember beginning of contents */
		/* find end of contents string. Remove \n at end if present */
		for (cpi = contents; '\0' != *cpi; cpi++) ;
		if (*(cpi - 1) == '\n') *(cpi - 1) = '\0';
		if (*contents != '\n' && *contents != '\0') /*contents needed*/
		  {
			for (cpi = eltname; '\0' != *cpi; cpi++) /*lowercase */
				if (('A' <= *cpi) && (*cpi <= 'Z'))
					*cpi += 'a' - 'A';
			nchanged += matchmar(eltname, contents, &mahe);
		  }
	     }
	(void)fclose(fpte);	fpte = NULL;
	if (nchanged <= 0)
	  {
		fprintf(stderr, "No header elements changed\n");
		return EOK;
	  }
	strcpy(mahe.mch_header_timestamp, marstamp());	nchanged++;
	if (-1 == lseek(fd, TIFFLENGTH, 0))
	  {
		fprintf(stderr,
		 "Error seeking to end of Tiff header in %s; errno = %d\n",
			imname, errno);
		return ESEEKERR;
	  }
	/* for some reason, we need to use the write primitive rather
	 than the fwrite here. fseek versus lseek, I guess. Phooey. */
	if (marhsize != write(fd, (AVOID *)&mahe, marhsize))
	/* if (1 != fwrite((AVOID *)&mahe, marhsize, 1, fpmar)) */
	  {
		fprintf(stderr,
		 "Error writing modified MarCCD header to %s; errno = %d\n",
			imname, errno);
		(void)fclose(fpmar);	return EWRITERR;
	  }
	printf("%3d header elements modified\n", nchanged);
	(void)fclose(fpmar);	return EOK;
}

/* end of marccd.c */
