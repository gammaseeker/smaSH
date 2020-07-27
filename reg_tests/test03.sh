#!/bin/bash

cd /home/jjiemjitpolc/hw3-jjiemjitpolc
RED='\033[0;31m'
ORANGE='\033[0;33m'
GREEN='\033[0;32m'
NC='\033[0m'

echo -e "${ORANGE} Redirection regression tests${NC}"

sleep 1
echo -e "${ORANGE} Test 1 redirect stdout${NC}"
./reg_tests/redir_cmd/redir_out.sh
if [ $? -eq 0 ]; then
    echo -e "${GREEN} PASSED!${NC}"
else
    echo -e "${RED} FAILED! ${NC}"
    exit 1
fi

sleep 1
echo -e "${ORANGE} Test 2 redirect stderr${NC}"
./reg_tests/redir_cmd/redir_err.sh
if [ $? -eq 0 ]; then
    echo -e "${GREEN} PASSED!${NC}"
else
    echo -e "${RED} FAILED! ${NC}"
    exit 1
fi

sleep 1
echo -e "${ORANGE} Test 3 redirect stdin${NC}"
./reg_tests/redir_cmd/redir_in.sh
if [ $? -eq 0 ]; then
    echo -e "${GREEN} PASSED!${NC}"
else
    echo -e "${RED} FAILED! ${NC}"
    exit 1
fi

sleep 1
echo -e "${ORANGE} Test 4 redirect stdin and stdout${NC}"
./reg_tests/redir_cmd/redir_in_out.sh
if [ $? -eq 0 ]; then
    echo -e "${GREEN} PASSED!${NC}"
else
    echo -e "${RED} FAILED! ${NC}"
    exit 1
fi


sleep 1
echo -e "${ORANGE} Test 5 redirect stderr and stdout${NC}"
./reg_tests/redir_cmd/redir_out_err.sh
if [ $? -eq 0 ]; then
    echo -e "${GREEN} PASSED!${NC}"
else
    echo -e "${RED} FAILED! ${NC}"
    exit 1
fi

sleep 1
echo -e "${ORANGE} Test 6 redirect stdin and stderr and stdout${NC}"
./reg_tests/redir_cmd/redir_in_out_err.sh
if [ $? -eq 0 ]; then
    echo -e "${GREEN} PASSED!${NC}"
else
    echo -e "${RED} FAILED! ${NC}"
    exit 1
fi
echo -e "${ORANGE}End test03.sh${NC}"
