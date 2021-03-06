#
# File:
#      snapbackend/src/CMakeLists.txt
#
# Description:
#      The Snap! Websites Server. The server is the main() of the Snap!
#      environment. Most everything is however defined in the
#      libsnapwebsites.so library (see lib/*).
#
# Documentation:
#      See the CMake documentation.
#
# License:
#      Copyright (c) 2011-2018  Made to Order Software Corp.  All Rights Reserved
#
#      https://snapwebsites.org/
#      contact@m2osw.com
#
#      This program is free software; you can redistribute it and/or modify
#      it under the terms of the GNU General Public License as published by
#      the Free Software Foundation; either version 2 of the License, or
#      (at your option) any later version.
#     
#      This program is distributed in the hope that it will be useful,
#      but WITHOUT ANY WARRANTY; without even the implied warranty of
#      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#      GNU General Public License for more details.
#     
#      You should have received a copy of the GNU General Public License
#      along with this program; if not, write to the Free Software
#      Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
#

project(snapbackend)

snap_validate_xml( snapbackend-processes.xml watchdog-processes.xsd )

add_executable(${PROJECT_NAME}
    snapbackend.cpp
)

target_link_libraries(${PROJECT_NAME}
    snapwebsites
    libdbproxy
    ${ADVGETOPT_LIBRARIES}
    ${LOG4CPLUS_LIBRARIES}
    ${AS2JS_LIBRARIES}
    ${QTSERIALIZATION_LIBRARIES}
    ${QT_LIBRARIES}
    ${LIBTLD_LIBRARIES}
    dl
)

install(
    TARGETS
        ${PROJECT_NAME}

    RUNTIME DESTINATION
        sbin
)

install(
    FILES
        snapbackend-processes.xml
    DESTINATION
        share/snapwebsites/snapwatchdog/processes
)


# vim: ts=4 sw=4 et
