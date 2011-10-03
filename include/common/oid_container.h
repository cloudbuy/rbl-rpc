#ifndef     RBL_HOME_EM_OID_CONTAINER_H
#define     RBL_HOME_EM_OID_CONTAINER_H
#include <stdexcept>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/intrusive/set.hpp>
#include <boost/type_traits.hpp>
#include <boost/static_assert.hpp>
#include <boost/integer_traits.hpp>
#include <vector>

namespace rubble { namespace common { 

  // uninstantiatable generic template
  template<typename char_type, unsigned length>
  class OidConstrainedString
  {
  private:
    OidConstrainedString();
  };

  // instantiatable specialization for UTF-8/ASCII
  template<unsigned length>
  class OidConstrainedString<char,length> 
  {
    typedef OidConstrainedString<char,length> my_type;
  public:
    OidConstrainedString();
    OidConstrainedString(const char * char_);
    OidConstrainedString(const std::string & str);
    
    inline bool operator<  (const my_type & rhs)  const;
    inline bool operator== (const my_type & rhs)  const;
    inline bool operator>  (const my_type & rhs)  const;

    inline const char * c_str()                   const;
    inline bool is_initialized()                  const;
  private:
    void construct_(const std::string & str);
    char str_[length];
  };

  template<unsigned length>
  std::ostream & operator<< 
  (std::ostream & os, const OidConstrainedString<char,length> & oid)
  {
    os << oid.c_str();
    return os; 
  }

  template< typename str_type, typename size_type>
  class OidType
  {
    BOOST_STATIC_ASSERT(boost::integer_traits<size_type>::is_integer == true);
    const static size_type MAX = boost::integer_traits<size_type>::const_max;

  public:
    typedef str_type name_type;
    typedef size_type ordinal_type;    

    OidType();
    explicit inline OidType( const std::string & str_in, 
                             const boost::uint32_t ordinal_in);

    inline bool operator==(const OidType<str_type,size_type> & rhs)         const;

    inline const str_type & name()                                          const;     
    inline const size_type ordinal()                                        const; 

    inline void set_name(const name_type & name_in);
    inline void set_ordinal(const size_type & ordinal_in);
  private:
    str_type name_;
    size_type ordinal_;
  };

  template<typename str_type, typename size_type>
  std::ostream & operator<<
  (std::ostream & os, const OidType<str_type, size_type> & oid)
  {
    os << oid.ordinal() << ':' << oid.name().c_str();
  }


  namespace
  {
      namespace intrusive = boost::intrusive;
  }

  template<typename identifier_type, typename _entry_type>
  class OidContainerEntryType: 
    public intrusive::set_base_hook<>
  {
    typedef OidContainerEntryType<identifier_type,_entry_type> my_type;
  public:
    typedef _entry_type basic_entry_type;

    inline OidContainerEntryType();
    explicit inline 
    OidContainerEntryType(const identifier_type & id, const _entry_type & et);
    //TODO comment here, seems this one converts two entry_types which have a 
    // conversion operator specified. it's a complex monstrosity to look at. 
    template<typename ENTRY>
    operator OidContainerEntryType<identifier_type, ENTRY>()              const;

    inline bool operator<  (const my_type & rhs)                          const;
    inline bool operator== (const my_type & rhs)                          const;
    inline bool operator>  (const my_type & rhs)                          const;

    inline bool is_initialized() const;
    inline const typename identifier_type::name_type & name()             const;
    inline const typename identifier_type::ordinal_type ordinal()         const;
    inline const _entry_type & entry()                                    const;
    inline _entry_type & entry();
    inline const identifier_type & Id()                                   const; 
    
    inline void set_identifier(const identifier_type & id_in);
    inline void set_entry(const basic_entry_type & entry_in);
  private:
    identifier_type id_;
    _entry_type     entry_;
  };

  enum OP_RESPONSE {
    OP_NO_ERROR = 0,
    OP_ORDINAL_USED,
    OP_NAME_USED,
    OP_ALLREADY_CONTAINS_ENTRY,
    OP_ORDINAL_OVERFLOW
  };

  //TODO  fix capatilization in EventAtordinal (captiral oh!)
  //TODO  REMOVE OPERATOR[] and replace with EntryWith...
  //TODO: REMOVE SIZE_T
  //TODO: NEST THE ENUMS INTO THE CLASSES THEY BELONG TO
  template<typename _identifier_type, typename _entry_type>
  class OidContainer
  {
  public:
    typedef _identifier_type                            identifier_type;
    typedef _entry_type                                 basic_entry_type;
    typedef OidContainerEntryType<  identifier_type, 
                                    basic_entry_type>   entry_type;
    typedef typename _identifier_type::name_type        name_type;
    typedef typename _identifier_type::ordinal_type     ordinal_type;
    typedef std::vector<entry_type>                     vector_type;

    OidContainer();    
    OidContainer(const OidContainer & rhs);
    OidContainer& operator=(const OidContainer & oid);
    template<typename Id_T, typename Entry_T>
    void SlicingPopulate(OidContainer<Id_T,Entry_T> & target)             const;
    
inline void clear();
    
    inline const vector_type & get_entries()                              const;
    inline const std::size_t size()                                       const;
    inline const std::size_t occupied_size()                              const;
    inline const entry_type * EntryAtordinal (boost::uint32_t ordinal)    const;
    inline entry_type * EntryAtordinal (boost::uint32_t ordinal);
    inline const entry_type * EntryWithName( const name_type & name_in)   const;
    inline entry_type * EntryWithName( const name_type & name_in);
    inline OP_RESPONSE ContainsEither(const identifier_type & id)         const;
    const basic_entry_type * operator[] (const name_type & name)          const;
    const basic_entry_type * operator[] (const ordinal_type & ordinal)    const;
    
    basic_entry_type * operator[] (const name_type & name)          ;
    basic_entry_type * operator[] (const ordinal_type & ordinal)    ;

    OP_RESPONSE SetEntry( const entry_type & entry);
    OP_RESPONSE SetEntry( const identifier_type & ,const basic_entry_type & );
  protected:
    typedef intrusive::set< entry_type > name_index_set;    
    
    struct name_key_finder; // functor
    
    void resize_if_needed_(boost::uint32_t ordinal);
    void regen_name_index_();
    
    std::vector<entry_type> entries_;
    name_index_set name_index_;
  };
//---------------------------------------------------------------------------//
} }
#include "oid_container-inl.h"
#endif
