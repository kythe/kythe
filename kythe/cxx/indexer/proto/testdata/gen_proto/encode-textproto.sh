#!/bin/bash

SOURCE_FILE=$1
OUT_FILE="$SOURCE_FILE.pb"
PROTO_FILE=$2

gqui --outfile=rawproto:"$OUT_FILE" from textproto:"$SOURCE_FILE" proto proto2.GeneratedCodeInfo
(echo "" && echo "/* 2edd7046-a7df-4327-85b3-10393805a0ba" && cat $OUT_FILE |  base64 && echo "*/") >> $PROTO_FILE
