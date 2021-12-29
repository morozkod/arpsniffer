#!/bin/bash

OUI_FILE=oui
OUI_FILE_URL="https://gitlab.com/wireshark/wireshark/-/raw/master/manuf"
OUI_HEADER_FILE="include/__generated__oui_array.h"

get_oui_file()
{
    # Testing connection
    ping -c 1 gitlab.com &> /dev/null
    if [ $? != 0 ]
    then
        echo "Unable to reach the database"
        exit
    fi

    echo "Downloading oui file from gitlab.com/wireshark"

    wget -O "$OUI_FILE" "$OUI_FILE_URL"
    if [ $? != 0 ]
    then
        echo "Unable to download oui file"
        exit
    fi
}

parse_oui_file()
{
    echo "Parsing oui file"
    # Remove shell-style comments. Explanation below
    # ^[[:blank:]]*#/d
    # Delete all lines that starts with any number of
    # blank symbols, followed by hash
    #
    # s/#.*//
    # Substitute inline comment with nothing
    #
    # ^$/d
    # Delete empty lines
    sed -i '/^[[:blank:]]*#/d;s/#.*//;/^$/d' "$OUI_FILE"
}

if [ ! -f "$OUI_FILE" ]
then
    get_oui_file
    parse_oui_file
fi

if [ ! -f "$OUI_HEADER_FILE" ]
then
    echo "Creating $OUI_HEADER_FILE"
    python3 generate_oui_header.py "$OUI_FILE"
fi

echo "Setup finished"
