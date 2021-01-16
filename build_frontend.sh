#!/bin/bash

app_folder=app/client
static_folder=bin/frontend_static

npm --prefix $app_folder install
npm --prefix $app_folder run build
rm -vrf $static_folder
mkdir -p $static_folder
cp -vr $app_folder/build/* $static_folder
