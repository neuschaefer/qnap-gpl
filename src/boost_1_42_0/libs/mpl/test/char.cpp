
// Copyright Eric Niebler 2008
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: char.cpp,v 1.1.1.1 2010/10/21 06:24:10 williamkao Exp $
// $Date: 2010/10/21 06:24:10 $
// $Revision: 1.1.1.1 $

#include <boost/mpl/char.hpp>
#include <boost/preprocessor/repeat.hpp>

#include "integral_wrapper_test.hpp"


MPL_TEST_CASE()
{
#   define WRAPPER(T, i) char_<i>
    BOOST_PP_REPEAT(10, INTEGRAL_WRAPPER_TEST, char)
}