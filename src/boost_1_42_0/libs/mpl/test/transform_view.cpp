
// Copyright Aleksey Gurtovoy 2001-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: transform_view.cpp,v 1.1.1.1 2010/10/21 06:24:10 williamkao Exp $
// $Date: 2010/10/21 06:24:10 $
// $Revision: 1.1.1.1 $

#include <boost/mpl/transform_view.hpp>

#include <boost/mpl/max_element.hpp>
#include <boost/mpl/list.hpp>
#include <boost/mpl/sizeof.hpp>
#include <boost/mpl/aux_/test.hpp>


MPL_TEST_CASE()
{
    typedef list<int,long,char,char[50],double> types;
    typedef max_element<
          transform_view< types, sizeof_<_> >
        >::type iter;

    MPL_ASSERT_RELATION( deref<iter>::type::value, ==, 50 );
}