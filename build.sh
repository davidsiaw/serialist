#!/bin/bash

pushd parser
bundle exec ruco serialist.ruco 
make
popd
bundle exec bin/serialist-gen tests/empty.format