#!/bin/python
import os, sys, subprocess, signal
import json
import time, datetime
from datetime import timedelta
from dateutil.parser import parse as dtparse
from argparse import ArgumentParser

process_list = []
processing = True

def parse_gps(jsonf, mp4f, max_sec_span, outputdir):
    json_file = json.load(open(jsonf))

    sec_cache = 0
    for frame_data in json_file:
        sec_cache = sec_cache + 1
        if sec_cache == max_sec_span and processing:
            mp4_to_imgs(mp4f, frame_data['starttime'], outputdir)
            img_file = 'img_' + frame_data['starttime'] + '.jpg'
            gps_to_img(frame_data['lat'], frame_data['lon'], outputdir + img_file)
            sec_cache = 0
       
def mp4_to_imgs(mp4f, starttime, outputdir):
    img_file = "img_%s.jpg" % starttime
    proc = subprocess.run('ffmpeg -i %s -ss %s -vframes 1 %s -y' % (mp4f, starttime, outputdir + img_file), stdout=subprocess.PIPE, shell=True)
#    os.system('ffmpeg -i %s -ss %s -vframes 1 %s -y' % (mp4f, starttime, outputdir + img_file))

    # get creation_time from metadata of mp4 file
    mdata_output = subprocess.getoutput('ffprobe -v quiet %s -print_format json -show_entries stream=index,codec_type:stream_tags=creation_time:format_tags=creation_time' % mp4f)
    cdate_json = json.loads(mdata_output)
    for cdate_data in cdate_json:
        if cdate_json['streams']:
            # 'streams' available
            for stream_item in cdate_json['streams']:
                if stream_item['codec_type'] == 'video' or stream_item['codec_type'] == 'audio':
                    cdate_timecode = stream_item['tags']['creation_time']
                    break
                else:
                    print('No creation_time-tag found in "streams"')
        elif cdate_json['format']['tags']['creation_time']:
            cdate_timecode = cdate_json['format']['tags']['creation_time']
        else:
            print("No creation_time found")

    cdate_starttime = dtparse(cdate_timecode) + timedelta(seconds=float(starttime))
    proc = subprocess.run("exiftool -DateTimeOriginal='%s' '%s' " % (cdate_starttime, outputdir + img_file), shell=True)


def gps_to_img(lat, lon, filename):
    process_list.append(subprocess.run('exiftool -gpslatitude="%s" -gpslatituderef="%s" -gpslongitude="%s" -gpslongituderef="%s" "%s"' % (lat, lat, lon, lon, filename), shell=True))
    process_list.append(subprocess.run('exiftool -delete_original !', shell=True))


def signal_handler(signal, frame):
    print("Received Ctrl-C")
    processing = False
    for proc in process_list:
        proc.kill()
    sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)

ap = ArgumentParser(description="Convert geotagged mp4 to geotagged jpg")
if __name__ == '__main__':
    ap.add_argument("-i", "--mp4", help="Set your input mp4 file with gps metadata")
    ap.add_argument("-s", "--sec", help="Set the interval for exported images in seconds: 'Every 5 secs, one image'", type=int, default=5)
    ap.add_argument("-j", "--json", help="Set your input json file, that you created with the c-program 'gpmf-parser'")
    ap.add_argument("-o", "--outputdir", help="Set your output folder for the created images", default=os.path.dirname(os.path.realpath(__file__)))
    args = ap.parse_args()

    if not os.path.exists(args.mp4):
        sys.exit('ERROR: mp4 file %s was not found!' % args.mp4)
    elif not os.path.exists(args.json):
        sys.exit('ERROR: json file %s was not found!' % args.json)
    elif args.sec and os.path.exists(args.outputdir):
        print(args.json + ' of ' + args.mp4)
        print("Images of every %s seconds to %s" % (args.sec, args.outputdir + os.path.sep))
        parse_gps(args.json, args.mp4, args.sec, args.outputdir + os.path.sep)
    else:
        ap.print_help()

