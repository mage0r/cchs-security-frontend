#!/bin/bash

source /etc/profile

nfc-list
nfc-list

# /usr/local/sbin/door-system

rm -rf /var/run/nfc

mkdir /var/run/nfc
ln -s /usr/local/sbin/door-system /var/run/nfc/run
supervise /var/run/nfc
