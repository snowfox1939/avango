// -*- Mode:C++ -*-

/************************************************************************\
*                                                                        *
* This file is part of Avango.                                           *
*                                                                        *
* Copyright 1997 - 2008 Fraunhofer-Gesellschaft zur Foerderung der       *
* angewandten Forschung (FhG), Munich, Germany.                          *
*                                                                        *
* Avango is free software: you can redistribute it and/or modify         *
* it under the terms of the GNU Lesser General Public License as         *
* published by the Free Software Foundation, version 3.                  *
*                                                                        *
* Avango is distributed in the hope that it will be useful,              *
* but WITHOUT ANY WARRANTY; without even the implied warranty of         *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the           *
* GNU General Public License for more details.                           *
*                                                                        *
* You should have received a copy of the GNU Lesser General Public       *
* License along with Avango. If not, see <http://www.gnu.org/licenses/>. *
*                                                                        *
* Avango is a trademark owned by FhG.                                    *
*                                                                        *
\************************************************************************/

#include <boost/python.hpp>
#include <avango/python/register_field.h>
#include <avango/osg/Image.h>
#include <osgDB/WriteFile>
#include "OSGImage.h"

using namespace boost::python;
using namespace av::python;

namespace boost
{
  namespace python
  {
    template <class T> struct pointee<av::Link<T> >
    {
      typedef T type;
    };
  }
}

void write_image_file(av::osg::Image* image, const std::string& name)
{
  osgDB::writeImageFile(*(image->getOsgImage()), name);
}

void init_OSGImage(void)
{
  // wrapping osg::Image functionality
  register_field<av::osg::SFImage>("SFImage");
  register_multifield<av::osg::MFImage>("MFImage");
  class_<av::osg::Image, av::Link<av::osg::Image>, bases<av::osg::Object>, boost::noncopyable >("Image", "docstring", no_init)
    .def("write_file", write_image_file)
    ;
}
