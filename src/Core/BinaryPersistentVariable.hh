/*
 * Generic/PersistentVariable.hh
 */

#pragma once

#include <boost/noncopyable.hpp>
#include <boost/exception/errinfo_file_name.hpp>
#include <db_cxx.h>
#include <pugixml.hpp>
#include <UCL/Exception.hh>
#include <UCL/Reflection/Binding/BinaryKeyValue/BerkeleyDB.hh>
#include <UCL/Reflection/Binding/BinaryKeyValue/Load.hh>
#include <UCL/Reflection/Binding/BinaryKeyValue/Save.hh>
#include <UCL/Reflection/Binding/ElementName.hh>
#include <UCL/Reflection/Binding/ElementNameTransformer.hh>


namespace Core {

  struct BinaryDumpException : uts::RuntimeException { };
  UTS_DEFINE_ERROR_INFO(Filename, std::string);

  /*!
   * Переменная, сохраняемая в файле.
   */
  template<typename T>
  class BinaryPersistentVariable : boost::noncopyable
  {
    T value;              //!< Значение переменной.
    std::string pathname; //!< Имя файла.
  public:
    BinaryPersistentVariable(BinaryPersistentVariable<T>&& x)
      : value(std::move(x.value)), pathname(std::move(x.pathname))
    { }

    BinaryPersistentVariable(std::string const& pathname)
      : pathname(pathname)
    { }

    template<typename U>
    BinaryPersistentVariable(U&& init, std::string const& pathname)
      : value(std::forward<U>(init)), pathname(pathname)
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

      try {
        Db db(nullptr, 0);
        auto err = db.open(nullptr, pathname.c_str(), "temp", DB_BTREE, DB_CREATE, 0);
        if (err != 0) BOOST_THROW_EXCEPTION(BinaryDumpException() 
                                            << ErrInfo_Filename(pathname) 
                                            << uts::ErrInfo_Description("Ошибка открытия базы BerkleyDB"));
        rb::BerkeleyDBStore store(db);

        dynamicLet(rb::currentNameTransformer, rb::elementNameSubwordFormatter(rb::DefaultSubwordSplitter(), rb::CapitalizeSubwordFormatter(), "-"))
          .in([&, this]() {
          try {
            this->value = rb::LoadFromBKVStore<T>::apply(store, "Binary");
            db.close(0);
          } catch (boost::exception& exc) {
            db.close(0);
            exc << boost::errinfo_file_name(pathname);
            throw;
          }
        });
      } catch (DbException& exc) {
        BOOST_THROW_EXCEPTION(BinaryDumpException() 
                              << ErrInfo_Filename(pathname)
                              << uts::ErrInfo_Description(exc.what()));
      }
    }

    auto save() const -> void
    {
      namespace rb = uts::reflection::binding;

      try {
        Db db(nullptr, 0);
        auto err = db.open(nullptr, pathname.c_str(), "temp", DB_BTREE, DB_CREATE, 0);
        if (err != 0) BOOST_THROW_EXCEPTION(BinaryDumpException() 
                                            << ErrInfo_Filename(pathname) 
                                            << uts::ErrInfo_Description("Ошибка открытия базы BerkleyDB"));
        rb::BerkeleyDBStore store(db);
        dynamicLet(rb::currentNameTransformer, rb::elementNameSubwordFormatter(rb::DefaultSubwordSplitter(), rb::CapitalizeSubwordFormatter(), "-"))
          .in([&,this]() {
            rb::SaveToBKVStore<T>::apply(this->value, store, "Binary");
          });

        db.close(0);
      } catch (DbException& exc) {
        BOOST_THROW_EXCEPTION(BinaryDumpException() 
                              << ErrInfo_Filename(pathname)
                              << uts::ErrInfo_Description(exc.what()));
      }
    }

    auto getFileName() -> const std::string { return pathname; }
    auto setFileName(std::string fileName) { pathname = fileName; }
  };

}
