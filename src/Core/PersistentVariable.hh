/*
 * PersistentVariable.hh
 */

#pragma once

#include <boost/noncopyable.hpp>
#include <boost/exception/errinfo_file_name.hpp>
#include <pugixml.hpp>
#include <UCL/Exception.hh>
#include <UCL/Reflection/Binding/PugiXml/FromDom/FromDom.hh>
#include <UCL/Reflection/Binding/PugiXml/ToDom/ToDom.hh>
#include <UCL/Reflection/Binding/ElementName.hh>
#include <UCL/Reflection/Binding/ElementNameTransformer.hh>

#include <UCL/Reflection/Binding/StringConversion.hh>
#include <UCL/Reflection/Binding/StringConversion/StdString/Bool.hh>
#include <UCL/Reflection/Binding/StringConversion/StdString/Enum.hh>
#include <UCL/Reflection/Binding/StringConversion/StdString/Numeric.hh>

struct SettingsLoadErrorException : uts::RuntimeException { };
UTS_DEFINE_ERROR_INFO(Filename, std::string);

/*!
  * Переменная, сохраняемая в файле.
  */
template<typename T>
class PersistentVariable : boost::noncopyable
{
  T value;              //!< Значение переменной.
  std::string pathname; //!< Имя файла.
  std::string tag;      //!< Тег верхнего уровня.
public:
  PersistentVariable(PersistentVariable<T>&& x)
    : value(std::move(x.value)), pathname(std::move(x.pathname)), tag(std::move(x.tag))
  { }

  PersistentVariable(std::string const& pathname, std::string const& tag)
    : pathname(pathname), tag(tag)
  { }

  template<typename U>
  PersistentVariable(U&& init, std::string const& pathname, std::string const& tag)
    : value(std::forward<U>(init)), pathname(pathname), tag(tag)
  { }

  auto operator= (T const& newValue) -> T const&
  {
    value = newValue;
    return value;
  }

  auto operator-> () -> T* { return &value; }
  auto operator-> () const -> T const* { return &value; }

  auto operator* () -> T& { return value; }
  auto operator* () const -> T const& { return value; }

  auto load() -> void
  {
    namespace rb = uts::reflection::binding;

    pugi::xml_document doc;
    auto parseResult = doc.load_file(pathname.c_str());
    if (parseResult) {
      dynamicLet(rb::currentNameTransformer, rb::elementNameSubwordFormatter(rb::DefaultSubwordSplitter(), rb::CapitalizeSubwordFormatter(), "-"))
        .in([&, this]() {
          try {
            this->value = rb::pugixml::FromDomElement<T>::apply(doc.document_element());
          } catch (boost::exception& exc) {
            exc << boost::errinfo_file_name(pathname);
            throw;
          }
        });
    } else {
      BOOST_THROW_EXCEPTION(SettingsLoadErrorException() 
                            << ErrInfo_Filename(pathname)
                            << uts::ErrInfo_Description(parseResult.description()));
    }
  }

  auto save() const -> void
  {
    namespace rb = uts::reflection::binding;

    pugi::xml_document doc;
    dynamicLet(rb::currentNameTransformer, rb::elementNameSubwordFormatter(rb::DefaultSubwordSplitter(), rb::CapitalizeSubwordFormatter(), "-"))
      .in([&,this]() {
        rb::pugixml::ToDomElement<T>::apply(tag, this->value, doc);
      });

    doc.save_file(pathname.c_str());
  }
};
