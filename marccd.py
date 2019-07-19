import struct

def header(inputfile):

    tiff_length = 1024
    
    vars = {}

    print 'Reading header:'

    with open(inputfile, 'rb') as f:
        print '-skipping tiff header'
        trash = f.read(tiff_length) #get rid of tiff stuff

        print '-reading file format crap (first 256 bytes)'
        vars['header_type'] = struct.unpack('I',f.read(4))[0]
        vars['header_name'] = f.read(16)
        vars['header_major_version'] = struct.unpack('I',f.read(4))[0]
        vars['header_minor_version'] = struct.unpack('I',f.read(4))[0]
        vars['header_byte_order'] = struct.unpack('I',f.read(4))[0]
        vars['data_byte_order'] = struct.unpack('I',f.read(4))[0]
        vars['header_size'] = struct.unpack('I',f.read(4))[0]
        vars['frame_type'] = struct.unpack('I',f.read(4))[0]
        vars['magic_number']  = struct.unpack('I',f.read(4))[0]
        vars['compression_type'] = struct.unpack('I',f.read(4))[0]
        vars['compression1'] = struct.unpack('I',f.read(4))[0]
        vars['compression2'] = struct.unpack('I',f.read(4))[0]
        vars['compression3'] = struct.unpack('I',f.read(4))[0]
        vars['compression4'] = struct.unpack('I',f.read(4))[0]
        vars['compression5'] = struct.unpack('I',f.read(4))[0]
        vars['compression6'] = struct.unpack('I',f.read(4))[0]
        vars['nheaders'] = struct.unpack('I',f.read(4))[0]
        vars['nfast'] = struct.unpack('I',f.read(4))[0]
        vars['nslow'] = struct.unpack('I',f.read(4))[0]
        vars['depth'] = struct.unpack('I',f.read(4))[0]
        vars['record_length'] = struct.unpack('I',f.read(4))[0]
        vars['significant_bits'] = struct.unpack('I',f.read(4))[0]
        vars['data_type'] = struct.unpack('I',f.read(4))[0]
        vars['saturated_value'] = struct.unpack('I',f.read(4))[0]
        vars['sequence'] = struct.unpack('I',f.read(4))[0]
        vars['nimages'] = struct.unpack('I',f.read(4))[0]
        vars['orgin'] = struct.unpack('I',f.read(4))[0]
        vars['orientation'] = struct.unpack('I',f.read(4))[0]
        vars['view_direction'] = struct.unpack('I',f.read(4))[0]
        vars['overflow_location'] = struct.unpack('I',f.read(4))[0]
        vars['over_8_bits'] = struct.unpack('I',f.read(4))[0]
        vars['over_16_bits'] = struct.unpack('I',f.read(4))[0]
        vars['multiplexed'] = struct.unpack('I',f.read(4))[0]
        vars['nfastimages'] = struct.unpack('I',f.read(4))[0]
        vars['nslowimages'] = struct.unpack('I',f.read(4))[0]
        vars['background_applied'] = struct.unpack('I',f.read(4))[0]
        vars['bias_applied'] = struct.unpack('I',f.read(4))[0]
        vars['flatfield_applied'] = struct.unpack('I',f.read(4))[0]
        vars['distortion_applied'] = struct.unpack('I',f.read(4))[0]
        vars['original_header_type'] = struct.unpack('I',f.read(4))[0]
        vars['file_saved'] = struct.unpack('I',f.read(4))[0]
        vars['reserve1'] = f.read(80)
        
        print '-reading data statistics (128 bytes)'
        vars['total_counts'] = [struct.unpack('I',f.read(4))[0],
        struct.unpack('I',f.read(4))[0]]
        vars['special_counts1'] = [struct.unpack('I',f.read(4))[0],
        struct.unpack('I',f.read(4))[0]]
        vars['special_counts2'] = [struct.unpack('I',f.read(4))[0],
        struct.unpack('I',f.read(4))[0]]
        vars['min'] = struct.unpack('I',f.read(4))[0]
        vars['max'] = struct.unpack('I',f.read(4))[0]
        vars['mean'] = struct.unpack('I',f.read(4))[0]
        vars['rms'] = struct.unpack('I',f.read(4))[0]
        vars['p10'] = struct.unpack('I',f.read(4))[0]
        vars['p90'] = struct.unpack('I',f.read(4))[0]
        vars['stats_uptodate'] = struct.unpack('I',f.read(4))[0]
        vars['pixel_noise'] = []
        for x in range (9):
            vars['pixel_noise'].append(struct.unpack('I',f.read(4))[0])
        reserve2 = f.read(40)
        
        print '-reading more statistics'
        vars['percentile'] = []
        for x in range (128):
            vars['percentile'].append(struct.unpack('H',f.read(2))[0])
        
        print '-reading goniostat parameters'
        vars['xtal_to_detector'] = struct.unpack('i',f.read(4))[0]
        vars['beam_x'] = struct.unpack('i',f.read(4))[0]
        vars['beam_y'] = struct.unpack('i',f.read(4))[0]
        vars['integration_time'] = struct.unpack('i',f.read(4))[0]
        vars['exposure_time'] = struct.unpack('i',f.read(4))[0]
        vars['readout_time'] = struct.unpack('i',f.read(4))[0]
        vars['nreads'] = struct.unpack('i',f.read(4))[0]
        vars['start_twotheta'] = struct.unpack('i',f.read(4))[0]
        vars['start_omega'] = struct.unpack('i',f.read(4))[0]
        vars['start_chi'] = struct.unpack('i',f.read(4))[0]
        vars['start_kappa'] = struct.unpack('i',f.read(4))[0]
        vars['start_phi'] = struct.unpack('i',f.read(4))[0]
        vars['start_delta'] = struct.unpack('i',f.read(4))[0]
        vars['start_gamma'] = struct.unpack('i',f.read(4))[0]
        vars['start_xtal_to_detector'] = struct.unpack('i',f.read(4))[0]
        vars['end_twotheta'] = struct.unpack('i',f.read(4))[0]
        vars['end_omega'] = struct.unpack('i',f.read(4))[0]
        vars['end_chi'] = struct.unpack('i',f.read(4))[0]
        vars['end_kappa'] = struct.unpack('i',f.read(4))[0]
        vars['end_phi'] = struct.unpack('i',f.read(4))[0]
        vars['end_delta'] = struct.unpack('i',f.read(4))[0]
        vars['end_gamma'] = struct.unpack('i',f.read(4))[0]
        vars['end_xtal_to_detector'] = struct.unpack('i',f.read(4))[0]
        vars['rotation_axis'] = struct.unpack('i',f.read(4))[0]
        vars['rotation_range'] = struct.unpack('i',f.read(4))[0]
        vars['detector_rotx'] = struct.unpack('i',f.read(4))[0]
        vars['detector_roty'] = struct.unpack('i',f.read(4))[0]
        vars['detector_rotz'] = struct.unpack('i',f.read(4))[0]
        vars['reserve3'] = f.read(16)
        
        print '-reading detector parameters'
        vars['detector_type'] = struct.unpack('i',f.read(4))[0]
        vars['pixelsize_x'] = struct.unpack('i',f.read(4))[0]
        vars['pixelsize_y'] = struct.unpack('i',f.read(4))[0]
        vars['mean_bias'] = struct.unpack('i',f.read(4))[0]
        vars['photons_per_100adu'] = struct.unpack('i',f.read(4))[0]
        vars['measured_bias'] = []
        for x in range (9):
            vars['measured_bias'].append(struct.unpack('i',f.read(4))[0])
        vars['measured_temperature'] = []
        for x in range (9):
            vars['measured_temperature'].append(struct.unpack('i',f.read(4))[0])
        vars['measured_pressure'] = []
        for x in range (9):
            vars['measured_temperature'].append(struct.unpack('i',f.read(4))[0])
        
        print '-reading x-ray source and optics parameters'
        vars['source_type'] = struct.unpack('i',f.read(4))[0]
        vars['source_dx'] = struct.unpack('i',f.read(4))[0]
        vars['source_dy'] = struct.unpack('i',f.read(4))[0]
        vars['source_wavelength'] = struct.unpack('i',f.read(4))[0]
        vars['source_power'] = struct.unpack('i',f.read(4))[0]
        vars['source_voltage'] = struct.unpack('i',f.read(4))[0]
        vars['source_current'] = struct.unpack('i',f.read(4))[0]
        vars['source_bias'] = struct.unpack('i',f.read(4))[0]
        vars['source_polarization_x'] = struct.unpack('i',f.read(4))[0]
        vars['source_polarization_y'] = struct.unpack('i',f.read(4))[0]
        vars['reserve_source'] = f.read(16)
        vars['optics_type'] = struct.unpack('i',f.read(4))[0]
        vars['optics_dx'] = struct.unpack('i',f.read(4))[0]
        vars['optics_dy'] = struct.unpack('i',f.read(4))[0]
        vars['optics_wavelength'] = struct.unpack('i',f.read(4))[0]
        vars['optics_distortion'] = struct.unpack('i',f.read(4))[0]
        vars['optics_crossfire_x'] = struct.unpack('i',f.read(4))[0]
        vars['optics_crossfire_y'] = struct.unpack('i',f.read(4))[0]
        vars['optics_angle'] = struct.unpack('i',f.read(4))[0]
        vars['optics_polarization_x'] = struct.unpack('i',f.read(4))[0]
        vars['optics_polarization_y'] = struct.unpack('i',f.read(4))[0]
        vars['optics_transmission'] = struct.unpack('i',f.read(4))[0]
        vars['reserve_optics'] = f.read(12)
        vars['reserve5'] = f.read(16)
        
        print '-reading file parameters'
        vars['filetitle'] = f.read(128)
        vars['filepath'] = f.read(128)
        vars['filename'] = f.read(64)
        vars['acquire_timestamp'] = f.read(32)
        vars['header_timestamp'] = f.read(32)
        vars['save_timestamp'] = f.read(32)
        vars['file_comments'] = f.read(512)
        vars['reserve6'] = f.read(96)
        
        print '-reading dataset parameters'
        vars['dataset_comments'] = f.read(512)
        
        print '-reading rest of padding to 3072 bytes'
        vars['pad'] = f.read(512)
    f.closed
    print "---returning dictionary"
    return vars

def image(inputfile):
    
    header_length = 4096
    
    #image is 3072 x 3072
    #each of these pixels is a 2 byte unsigned short
    print "Reading image"
    with open(inputfile, 'rb') as f:
        pixel_array = []
        
        header_stuff = f.read(header_length)
        
        for x in range (3072):
            if x % 300 == 0:
                print ".",
            temp_array = []
            for y in range (3072):
                temp_array.append(struct.unpack('H', f.read(2))[0])
            pixel_array.append(temp_array)
        
        print "\n---returning image"    
        return pixel_array
    f.closed