#!/usr/bin/env python

import setuptools
import setuptools_scm

setuptools.setup(
    version=setuptools_scm.get_version(),
    # use_scm_version=True,
    setup_requires=["setuptools_scm"],
)

# -----------------------------------------------------------------------------
# :author:    Pete R. Jemian
# :email:     prjemian@gmail.com
# :copyright: (c) 2014-2022, Pete R. Jemian
#
# Distributed under the terms of the Creative Commons Attribution 4.0 International Public License.
#
# The full license is in the file LICENSE, distributed with this software.
# -----------------------------------------------------------------------------
