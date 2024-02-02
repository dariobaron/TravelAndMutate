#!/bin/bash

#######################################
### 	please source this file: 	###
### 	$ source setup.sh 			###
#######################################

python -m venv env/

source env/bin/activate

python -m pip install -r requirements.txt

cd src

make

cd ..