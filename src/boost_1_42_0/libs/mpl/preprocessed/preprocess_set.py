
# Copyright Aleksey Gurtovoy 2001-2006
#
# Distributed under the Boost Software License, Version 1.0. 
# (See accompanying file LICENSE_1_0.txt or copy at 
# http://www.boost.org/LICENSE_1_0.txt)
#
# See http://www.boost.org/libs/mpl for documentation.

# $Id: preprocess_set.py,v 1.1.1.1 2010/10/21 06:24:10 williamkao Exp $
# $Date: 2010/10/21 06:24:10 $
# $Revision: 1.1.1.1 $

import preprocess
import os.path

preprocess.main(
      [ "plain" ]
    , "set"
    , os.path.join( "boost", "mpl", "set", "aux_", "preprocessed" )
    )
