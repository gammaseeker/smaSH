#!/bin/bash

cd /home/jjiemjitpolc/hw3-jjiemjitpolc
RED='\033[0;31m'
ORANGE='\033[0;33m'
GREEN='\033[0;32m'
NC='\033[0m'

echo -e "${ORANGE} Variable regression tests${NC}"

sleep 1
echo -e "${ORANGE} Test 1 echo \$PATH${NC}"
./reg_tests/var_cmd/path_test.sh
if [ $? -eq 0 ]; then
    echo -e "${GREEN} PASSED!${NC}"
else
    echo -e "${RED} FAILED! ${NC}"
    exit 1
fi

sleep 1
echo -e "${ORANGE} Test 2 set var and echo${NC}"
./reg_tests/var_cmd/var_test.sh
if [ $? -eq 0 ]; then
    echo -e "${GREEN} PASSED!${NC}"
else
    echo -e "${RED} FAILED! ${NC}"
    exit 1
fi

sleep 1
echo -e "${ORANGE} Test 3 run pwd and echo \$?${NC}"
./reg_tests/var_cmd/ret_test.sh
if [ $? -eq 0 ]; then
    echo -e "${GREEN} PASSED!${NC}"
else
    echo -e "${RED} FAILED! ${NC}"
    exit 1
fi

sleep 1
echo -e "${ORANGE}End test02.sh${NC}"
