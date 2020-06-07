#!/usr/bin/python
############################################################################
#
# Copyright (C) 2019 The Qt Company Ltd.
# Contact: https://www.qt.io/licensing/
#
# This file is part of Qt Creator.
#
# Commercial License Usage
# Licensees holding valid commercial Qt licenses may use this file in
# accordance with the commercial license agreement provided with the
# Software or, alternatively, in accordance with the terms contained in
# a written agreement between you and The Qt Company. For licensing terms
# and conditions see https://www.qt.io/terms-conditions. For further
# information use the contact form at https://www.qt.io/contact-us.
#
# GNU General Public License Usage
# Alternatively, this file may be used under the terms of the GNU
# General Public License version 3 as published by the Free Software
# Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
# included in the packaging of this file. Please review the following
# information to ensure the GNU General Public License requirements will
# be met: https://www.gnu.org/licenses/gpl-3.0.html.
#
############################################################################

import os
import sys
import xml.etree.ElementTree as ET

if len(sys.argv) != 3:
    print("Please provide a top level directory to scan and a file to write into.")
    sys.exit(1)

top_dir = sys.argv[1]
target_file = sys.argv[2]


def fix_value(value):
    value = value.replace('\"', '\\\"')
    value = value.replace('\n', '\\\n')
    return value


def parse_file(file_path):
    root = ET.parse(file_path).getroot()
    result = '#include <QtGlobal>\n\n'

    index = 0
    for i in ['.//description', './/displayname', './/category']:
        for e in root.findall(i):
            result += 'const char *a{} = QT_TRANSLATE_NOOP("Core::Internal::ExternalTool", "{}"); // {}\n'.format(index, fix_value(e.text), file_path)

            index += 1
    return result

result = ''

# traverse root directory, and list directories as dirs and files as files
for root, _, files in os.walk(top_dir):
    for file in files:
        if file.endswith('.xml'):
            result += parse_file(os.path.join(root, file))

with open(target_file, 'w') as header_file:
    header_file.write(result)
