#!/bin/bash

cd /home/jjiemjitpolc/hw3-jjiemjitpolc
RED='\033[0;31m'
ORANGE='\033[0;33m'
GREEN='\033[0;32m'
NC='\033[0m'

echo -e "${ORANGE} Simple command regression tests${NC}"

sleep 1
echo -e "${ORANGE} Test 1 ls command${NC}"
./reg_tests/simple_cmd/ls_test.sh
if [ $? -eq 0 ]; then
    echo -e "${GREEN} PASSED!${NC}"
else
    echo -e "${RED} FAILED! ${NC}"
    exit 1
fi

sleep 1
echo -e "${ORANGE} Test 2 pwd command${NC}"
./reg_tests/simple_cmd/pwd_test.sh
if [ $? -eq 0 ]; then
    echo -e "${GREEN} PASSED!${NC}"
else
    echo -e "${RED} FAILED! ${NC}"
    exit 1
fi

sleep 1
echo -e "${ORANGE} Test 3 cd command${NC}"
./reg_tests/simple_cmd/cd_test.sh
if [ $? -eq 0 ]; then
    echo -e "${GREEN} PASSED!${NC}"
else
    echo -e "${RED} FAILED! ${NC}"
    exit 1
fi

sleep 1
echo -e "${ORANGE} Test 4 echo command${NC}"
./reg_tests/simple_cmd/echo_test.sh
if [ $? -eq 0 ]; then
    echo -e "${GREEN} PASSED!${NC}"
else
    echo -e "${RED} FAILED! ${NC}"
    exit 1
fi

sleep 1
echo -e "${ORANGE} Test 5 ls -la command${NC}"
./reg_tests/simple_cmd/ls_flag_test.sh
if [ $? -eq 0 ]; then
    echo -e "${GREEN} PASSED!${NC}"
else
    echo -e "${RED} FAILED! ${NC}"
    exit 1
fi

sleep 1
echo -e "${ORANGE}End test01.sh${NC}"
