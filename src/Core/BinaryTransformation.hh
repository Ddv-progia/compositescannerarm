#pragma once

#include <sstream>
#include <fstream>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "boost/filesystem.hpp"
#include "boost/format.hpp"
#include <boost/log/attributes.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include <db_cxx.h>

#include <Ice/BuiltinSequences.h>

#include <UCL/Exception.hh>
#include <UCL/Reflection/Binding/ElementName.hh>
#include <UCL/Reflection/Binding/ElementNameTransformer.hh>
#include <UCL/Reflection/Binding/BinaryKeyValue/Save.hh>
#include <UCL/Reflection/Binding/BinaryKeyValue/Save/BoostOptional.hh>
#include <UCL/Reflection/Binding/BinaryKeyValue/Save/ReflectedStructure.hh>
#include <UCL/Reflection/Binding/BinaryKeyValue/Save/StandardSequence.hh>
#include <UCL/Reflection/Binding/BinaryKeyValue/Save/FixedSizeSequence.hh>
#include <UCL/Reflection/Binding/BinaryKeyValue/Load/BoostOptional.hh>
#include <UCL/Reflection/Binding/BinaryKeyValue/Load/ReflectedStructure.hh>
#include <UCL/Reflection/Binding/BinaryKeyValue/Load/StandardSequence.hh>
#include <UCL/Reflection/Binding/BinaryKeyValue/Load/FixedSizeSequence.hh>
#include <UCL/Reflection/Binding/Tags/ConcurrentVector.hh>
#include <UCL/Reflection/Binding/Tags/ReflectedStructure.hh>
#include <UCL/Reflection/Binding/Tags/FixedSizeSequence.hh>
#include <UCL/Reflection/Binding/Tags/StdVector.hh>
#include <UCL/Reflection/Binding/Tags/StdList.hh>
#include <UCL/Reflection/Binding/Tags/TagOf.hh>
#include <UCL/Reflection/Binding/BinaryConversion.hh>
#include <UCL/Reflection/Binding/BinaryConversion/Bool.hh>
#include <UCL/Reflection/Binding/BinaryConversion/FixedSequence.hh>
#include <UCL/Reflection/Binding/BinaryConversion/Numeric.hh>
#include <UCL/Reflection/Binding/BinaryConversion/Enum.hh>
#include <UCL/Reflection/Binding/BinaryConversion/Sequence.hh>
#include <UCL/Reflection/Binding/BinaryConversion/StdString.hh>
#include <UCL/Reflection/Binding/BinaryKeyValue/Load.hh>
#include <UCL/Reflection/Binding/BinaryKeyValue/BerkeleyDB.hh>

//#include <Generic/Logging.hh>


namespace uts { namespace reflection { namespace binding {

  class BinaryConvertException : public virtual uts::Exception {};

  template<typename T>
  void convertToBinaryFile(T const& x, std::string const& pathname)
  {
    namespace rb = uts::reflection::binding;
    namespace fs = boost::filesystem;

    Db db(nullptr, 0);
    auto err = db.open(nullptr, pathname.c_str(), "temp", DB_BTREE, DB_CREATE, 0);
    if (err != 0) BOOST_THROW_EXCEPTION(BinaryConvertException() << uts::ErrInfo_Description("Ошибка открытия базы BerkleyDB"));
    rb::BerkeleyDBStore store(db);
    dynamicLet(rb::currentNameTransformer, rb::elementNameSubwordFormatter(rb::DefaultSubwordSplitter(), rb::CapitalizeSubwordFormatter(), "-"))
      .in([&]() { rb::SaveToBKVStore<T>::apply(x, store, "Binary"); });
    db.close(0);
  }

  template<typename T>
  auto convertToBinary(T const& x) -> Ice::ByteSeq
  {
    namespace fs = boost::filesystem;
    Ice::ByteSeq binary;

    auto path = fs::temp_directory_path() / (to_string(boost::uuids::random_generator()()) + ".db");
    convertToBinaryFile(x, path.string());
    
    std::ifstream data;
    data.open(path.string().c_str(), std::ios_base::binary);
    if (!data.is_open()) BOOST_THROW_EXCEPTION(BinaryConvertException() << uts::ErrInfo_Description("Ошибка открытия файла " + path.string()));
    auto size = fs::file_size(path);
    binary.resize(size);
    data.read(reinterpret_cast<char*>(binary.data()), size);
    data.close();
    fs::remove(path);

    return binary;
  }


  template<typename T>
  auto convertFromBinary(Ice::ByteSeq const& serializedSlice) -> T
  {
    namespace rb = uts::reflection::binding;
    namespace fs = boost::filesystem;

    auto path = fs::temp_directory_path() / (to_string(boost::uuids::random_generator()()) + ".db");

    std::ofstream data;
    data.open(path.string().c_str(), std::ios_base::binary);
    if (!data.is_open()) BOOST_THROW_EXCEPTION(BinaryConvertException() << uts::ErrInfo_Description("Ошибка открытия файла " + path.string()));
    
    data.write(reinterpret_cast<const char*>(serializedSlice.data()), serializedSlice.size());
    data.close();

    T t;
    Db db(nullptr, 0);
    auto err = db.open(nullptr, path.string().c_str(), "temp", DB_BTREE, DB_CREATE, 0);
    rb::BerkeleyDBStore store(db);
    dynamicLet(rb::currentNameTransformer, rb::elementNameSubwordFormatter(rb::DefaultSubwordSplitter(), rb::CapitalizeSubwordFormatter(), "-"))
      .in([&]() { t = rb::LoadFromBKVStore<T>::apply(store, "Binary"); });
    db.close(0);

    fs::remove(path);

    return t;
  }

} } }
