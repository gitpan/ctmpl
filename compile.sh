#!/bin/bash

make clean && perl ./Makefile.PL && make install && /etc/init.d/httpd restart
