#!/bin/sh

#param1: http or https
#param2: IP:port
#param3: target xml file

TEMP_FILE=temp.xml

#sed "s/\(<center_info>\).*\(<\/center_info>\)/\1${1}:\/\/${2}\/msgmgr\/message\/\2/g" ${3} >${TEMP_FILE}
#mv -f ${TEMP_FILE} ${3}
#sed "s/\(<daemon_package>daemon_package:\|viruslib_package:\|version_file_url:\).*\(\/[[:graph:]]\{1,\}\)\(<\/daemon_package>\|<\/viruslib_package>\|<\/version_file_url>\)/\1${1}:\/\/${2}\/update\2\3/g" ${3} >${TEMP_FILE}

sed "s/\(<center_info>\).*\(<\/center_info>\)/\1${1}:\/\/${2}\/msgmgr\/message\/\2/g" ${3} | sed "s/\(<daemon_package>daemon_package:\|viruslib_package:\|version_file_url:\).*\(\/[[:graph:]]\{1,\}\)\(<\/daemon_package>\|<\/viruslib_package>\|<\/version_file_url>\)/\1${1}:\/\/${2}\/update\2\3/g" >${TEMP_FILE}

mv -f ${TEMP_FILE} ${3}
rm -f ${TEMP_FILE}



