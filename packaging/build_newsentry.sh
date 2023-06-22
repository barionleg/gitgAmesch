#!/bin/bash

# Get CURRENT_VERSION
if [ $# -eq 1 ]; then
	CURRENT_VERSION=$1
elif [ $# -eq 0 ]; then
	CURRENT_VERSION=$(git log -1 --format=%ci | cut -b3,4,6,7,9,10)
else
	echo "ERR: parameters must number exactly 0 or 1"
	exit 1
fi


# Update changelog with CURRENT_VERSION
sed -i "/unreleasd/c\Version $CURRENT_VERSION" ../CHANGELOG
sed -i "/Current Version/c\Version $CURRENT_VERSION" ../CHANGELOG

# Generate partial changelog e.g. to post on RG.
CHANGELOG_PARTIAL=CHANGELOG_$CURRENT_VERSION.txt
cat ../CHANGELOG | sed '0,/^$/I!d' > ${CHANGELOG_PARTIAL}
echo Partial changelog written to: ${CHANGELOG_PARTIAL}

# Generate News (will need an edit in the year 2100!!!)
YEAR="20"$(echo $CURRENT_VERSION | cut -c 1,2)
MONTH=$(echo $CURRENT_VERSION | cut -c 3,4)
DAY=$(echo $CURRENT_VERSION | cut -c 5,6)
NEWS_FILE=$YEAR-$MONTH-$DAY"_New_Version_"$CURRENT_VERSION"_Now_Available.md"
cat > ${NEWS_FILE} << _EOF_
---
layout: news
title: New Version $CURRENT_VERSION Now Available
pubdate: $YEAR-$MONTH-$DAY
description: ""
changes:
_EOF_
sed 1d ${CHANGELOG_PARTIAL} | sed s/[+-o]\)/\ \ \ \ -\ \"/ | sed ':a;N;$!ba;s/\n/\"\n/g' >> ${NEWS_FILE} 
cat >>  ${NEWS_FILE} << _EOF_
downloadhint: Please find links to download on the <a href="/downloads/">Downloads</a> page.
---
_EOF_

echo News written to: ${NEWS_FILE}

