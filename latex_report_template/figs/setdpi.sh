#!/bin/bash
# Strips the DPI value from the filename and encodes it into the file.
if [ ! -d converted ]; then
	mkdir converted
fi
for FILEVIEW in *_*DPI.png; do 
	SETDPI=`echo $FILEVIEW | sed 's/.*_\([0-9]*\)DPI.*/\1/'`
	FILENAMENEW=`echo $FILEVIEW | sed 's/\(.*\)_[0-9]*DPI\(.*\)/\1\2/'`
	SETPPCM=`echo "scale=2; 800*100/254" | bc`
	echo $FILEVIEW will be set to $SETPPCM px/cm $SETDPI DPI and saved as $FILENAMENEW
	COMMENT=`echo Exported from GigaMesh, original filename: $FILEVIEW`
	convert $FILEVIEW -units PixelsPerCentimeter -density $SETPPCM -set Comment "$COMMENT" $FILENAMENEW
	if [ $? ]; then
		mv $FILEVIEW converted
		echo Conversion done - moved to converted/$FILEVIEW.
	else
		echo Conversion failed.
	fi
done
