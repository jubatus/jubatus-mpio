//
// mp::functional
//
// Copyright (C) 2008-2010 FURUHASHI Sadayuki
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//
#ifndef MP_FUNCTIONAL_H__
#define MP_FUNCTIONAL_H__

#include <functional>

#ifdef MP_FUNCTIONAL_BOOST
#include <boost/tr1/functional.hpp>
namespace mp {
	using std::tr1::function;
}
#else
#ifdef MP_FUNCTIONAL_BOOST_ORG
#include <boost/function.hpp>
#include <boost/bind.hpp>
namespace mp {
	using boost::function;
}
#else
#if !defined(MP_FUNCTIONAL_STANDARD) && defined(__GLIBCXX__)
#include <tr1/functional>
namespace mp {
	using std::tr1::function;
}
#else
namespace mp {
	using std::function;
}
#endif
#endif
#endif

#endif /* mp/functional.h */

