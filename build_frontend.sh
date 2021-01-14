#!/bin/bash

npm --prefix app/client/ run build
rm -vrf bin/static/
cp -vr app/client/build/ bin/static
