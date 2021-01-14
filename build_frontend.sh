#!/bin/bash

npm --prefix app/client/ run build
rm -vrf bin/static/
mkdir -p bin/frontend_static/
cp -vr app/client/build/* bin/frontend_static/
