#!/bin/bash

pushd parser
bundle exec ruco serialist.ruco 
make
popd
bundle exec exe/serialist-gen tests/empty.format