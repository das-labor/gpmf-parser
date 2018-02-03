# GPMF Introduction

[We only tested it on GoPro Hero 5 MP4 Files.]

## Dependencies
* python 3.5
* ffmpeg
* exiftool
* dateutils
* argparse

## Setup
* Install the dependencies (with pip or through your packet manager)
* Execute `cmake . && make` which creates an `gpmf-parser` binary

## Usage
To get the metadata of GPS and timecode out of the video file, choose your 
mp4-file and execute the binary. Pipe the json-formatted output into a textfile:
`./gpmf-parser test.mp4 > test.json`

After that, you can execute the python script, like:
```python3 gpmf-geoimg.py \
 --mp4 <path to your mp4-file> \
 --json <path to your gpmf-parser-created json-file> \
  -sec <seconds interval> \` default is 'every 5 seconds one image'
  --outputdir <directory for created images>
```

This will take a while, but then you have images with timestamps and GPS data 
from your GoPro video files.

Your images will be needed at http://openstreetcam.org. You can upload them
very easily via scripts: https://github.com/openstreetcam/upload-scripts

```
GoPro is trademark of GoPro, Inc.
```

