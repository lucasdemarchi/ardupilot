#!/bin/bash

set -ex

waf=modules/waf/waf-light

echo "Starting waf build for SITL..."
$waf build
