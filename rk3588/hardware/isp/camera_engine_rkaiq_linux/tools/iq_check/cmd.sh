#!/bin/bash

ROOT_PROJ_DIR=/home/troy/team_git/camera_engine_rkaiq_linux
PROJECT_SOURCE_DIR=.
${PROJECT_SOURCE_DIR}/script/headerprocess.sh \
${ROOT_PROJ_DIR}/include/iq_parser_v2/RkAiqCalibDbTypesV2.h \
${ROOT_PROJ_DIR}/include/iq_parser_v2/j2s/j2s_generated.h

${PROJECT_SOURCE_DIR}/bin/parser ${ROOT_PROJ_DIR}/build2/iq_parser_v2/RkAiqCalibDbTypesV2_M4.h >> \
${ROOT_PROJ_DIR}/include/iq_parser_v2/j2s/j2s_generated.h