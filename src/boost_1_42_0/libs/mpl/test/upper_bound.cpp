
// Copyright Aleksey Gurtovoy 2001-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: upper_bound.cpp,v 1.1.1.1 2010/10/21 06:24:10 williamkao Exp $
// $Date: 2010/10/21 06:24:10 $
// $Revision: 1.1.1.1 $

#include <boost/mpl/upper_bound.hpp>

#include <boost/mpl/distance.hpp>
#include <boost/mpl/list_c.hpp>
#include <boost/mpl/less.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/aux_/test.hpp>

MPL_TEST_CASE()
{
    typedef list_c<int,1,2,3,3,3,5,8> numbers;
    typedef upper_bound< numbers, int_<3> >::type iter;

    MPL_ASSERT_RELATION( (mpl::distance< begin<numbers>::type,iter >::value), ==, 5 );
    MPL_ASSERT_RELATION( deref<iter>::type::value, ==, 5 );
}
