// -*- Mode:C++ -*-

/************************************************************************\
*                                                                        *
* This file is part of AVANGO.                                           *
*                                                                        *
* Copyright 1997 - 2010 Fraunhofer-Gesellschaft zur Foerderung der       *
* angewandten Forschung (FhG), Munich, Germany.                          *
*                                                                        *
* AVANGO is free software: you can redistribute it and/or modify         *
* it under the terms of the GNU Lesser General Public License as         *
* published by the Free Software Foundation, version 3.                  *
*                                                                        *
* AVANGO is distributed in the hope that it will be useful,              *
* but WITHOUT ANY WARRANTY; without even the implied warranty of         *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the           *
* GNU General Public License for more details.                           *
*                                                                        *
* You should have received a copy of the GNU Lesser General Public       *
* License along with AVANGO. If not, see <http://www.gnu.org/licenses/>. *
*                                                                        *
\************************************************************************/

#include <boost/python.hpp>

#include <avango/Link.h>
#include <avango/ContainerPool.h>
#include <avango/osg/viewer/View.h>

#include <osg/Matrix>
#include <osgViewer/View>
#include <osgViewer/ViewerEventHandlers>
#include <osg/Version>

#ifdef PCL_SUPPORT
  #include "UTILSPcd.h"
#endif

#include "../include/avango/utils/Init.h"
#include "../include/avango/utils/Bool2Or.h"
#include "../include/avango/utils/Bool2And.h"
#include "../include/avango/utils/Bool3Or.h"
#include "../include/avango/utils/Bool3And.h"
#include "../include/avango/utils/Trackball.h"
#include "../include/avango/utils/MultiValueField.h"
#include "../include/avango/utils/register_multivaluefield.h"

#include <iostream>
#include <iomanip>

#include <map>

using namespace boost::python;

// TODO can we move this somewhere else?
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

template < typename T, typename U >
std::multimap< U, T > converseMap( const std::map< T, U >& o )
{
  std::multimap< U, T > result;
  for ( typename std::map< T, U >::const_iterator begin( o.begin() ); begin != o.end(); ++begin )
    result.insert( make_pair( begin->second, begin->first ) );
  return result;
}

osg::Vec3 CalcHpr(const osg::Matrix& mat)
{
  osg::Vec3 hpr;
  if(mat(1,0)>0.998) {
    float heading = atan2(mat(0,2),mat(2,2));
    float attitude = 3.14/2.0;
    float bank =0;
    hpr = osg::Vec3(heading,attitude,bank);
    return hpr;
  }

  if(mat(1,0)<-0.998) {
    float heading = atan2(mat(0,2),mat(2,2));
    float attitude = -3.14/2.0;
    float bank =0;
    hpr = osg::Vec3(heading,attitude,bank);
    return hpr;
  }

  float heading = atan2(-mat(2,0),mat(0,0));
  float attitude = atan2(-mat(1,2),mat(1,1));
  float bank = sin(mat(1,0));
  hpr = osg::Vec3(heading,attitude,bank);
  return hpr;
}

void print_actual_registered_field_containers()
{
  const std::map< av::FieldContainer::IDType, av::FieldContainer* > & containers = av::ContainerPool::getContainerPool();
  std::map< av::FieldContainer::IDType, av::FieldContainer* >::const_iterator iter;
  std::map< std::string,int> m;
  int maxLength = 0;
  for(iter=containers.begin();iter!=containers.end();++iter)
  {
//    std::string type = ::av::ContainerPool::getNameByInstance(iter->second);
//    std::string type = iter->second->Name.getValue();
    std::string type = iter->second->getTypeId().getName();

    int l = type.length();
    if(l>maxLength)
      maxLength = l;
    std::map< std::string,int >::const_iterator find_iter = m.find(type);
    if(find_iter==m.end())
    {
      m[type] = 1;
    }
    else
    {
      m[type]++;
    }
  }

  std::ostringstream ss;
  std::multimap< int, std::string > conversedMap = converseMap( m );
  std::multimap< int, std::string >::iterator i;
  for(i=conversedMap.begin();i!=conversedMap.end();++i)
  {
    int l = maxLength - i->second.length();
    ss <<"\""<< i->second << "\" ";
    for(int n=0;n<l;++n)
      ss << " ";
    ss<< i->first << std::endl;
  }
  for(int n=0;n<maxLength+2;++n)
      ss << " ";
  ss << "-------"<<std::endl;
  std::string total_desc = "Total number of containers:";
  int l = maxLength - total_desc.length();
  ss <<"\""<< total_desc << "\" ";
  for(int n=0;n<l;++n)
    ss << " ";
  ss<< av::ContainerPool::getNumberOfContainers() << std::endl;
  std::cout << ss.str();
}

void init_MultiValueFields()
{
  av::python::register_multivaluefield<av::utils::MVFBool>("MVFBool");
  av::python::register_multivaluefield<av::utils::MVFDouble>("MVFDouble");
  av::python::register_multivaluefield<av::utils::MVFFloat>("MVFFloat");
  av::python::register_multivaluefield<av::utils::MVFInt>("MVFInt");
  av::python::register_multivaluefield<av::utils::MVFLong>("MVFLong");
  av::python::register_multivaluefield<av::utils::MVFUInt>("MVFUInt");
  av::python::register_multivaluefield<av::utils::MVFULong>("MVFULong");
  av::python::register_multivaluefield<av::utils::MVFString>("MVFString");

  // osg related multi value fields
  av::python::register_multivaluefield<av::utils::MVFMatrix>("MVFMatrix");
  av::python::register_multivaluefield<av::utils::MVFVec2>("MVFVec2");
  av::python::register_multivaluefield<av::utils::MVFVec3>("MVFVec3");
  av::python::register_multivaluefield<av::utils::MVFVec4>("MVFVec4");
  av::python::register_multivaluefield<av::utils::MVFQuat>("MVFQuat");
}


#if OSG_VERSION_MAJOR == 3
void addScreenCaptureHandler(av::osg::viewer::View * avView, std::string folder, std::string filename, int numFrames) {
  ::osgViewer::View * view = avView->getOsgView();
  osgViewer::ScreenCaptureHandler* scnsvr = new osgViewer::ScreenCaptureHandler(new osgViewer::ScreenCaptureHandler::WriteToFile(folder+"/"+filename,"png"));
  scnsvr->setFramesToCapture(numFrames);
  scnsvr->startCapture();
  view->addEventHandler(scnsvr);
}
#elif OSG_VERSION_MAJOR == 2
void addScreenCaptureHandler(av::osg::viewer::View * avView, std::string folder, std::string filename, int numFrames) {
  std::cerr << "addScreenCaptureHandler::ERROR Screen capture functionality is currently only supported with osg3.x" << std::endl;
}
#endif




BOOST_PYTHON_MODULE(_utils)
{
  av::utils::Init::initClass();

  class_<av::utils::Bool2Or, av::Link<av::utils::Bool2Or>, bases<av::FieldContainer>, boost::noncopyable >("Bool2Or", "docstring", no_init);
  class_<av::utils::Bool2And, av::Link<av::utils::Bool2And>, bases<av::FieldContainer>, boost::noncopyable >("Bool2And", "docstring", no_init);
  class_<av::utils::Bool3Or, av::Link<av::utils::Bool3Or>, bases<av::FieldContainer>, boost::noncopyable >("Bool3Or", "docstring", no_init);
  class_<av::utils::Bool3And, av::Link<av::utils::Bool3And>, bases<av::FieldContainer>, boost::noncopyable >("Bool3And", "docstring", no_init);

  class_<av::utils::Trackball, av::Link<av::utils::Trackball>, bases<av::FieldContainer>, boost::noncopyable >("Trackball", "docstring", no_init);

  def("calc_hpr", CalcHpr);

  def("print_registered_field_containers",print_actual_registered_field_containers);

  def("add_screen_capture_handler",addScreenCaptureHandler);

  av::utils::initMultiValueFields();
  av::utils::initMultiValueOSGFields();
  init_MultiValueFields();

#ifdef PCL_SUPPORT
  init_PCD();
#endif

}