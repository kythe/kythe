#!/bin/sh -e
chown -R $(stat "$1" -c %u:%g) "$1"
