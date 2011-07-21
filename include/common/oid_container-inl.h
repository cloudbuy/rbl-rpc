#ifndef     RBL_HOME_EM_OID_CONTAINER_INL_H
#define     RBL_HOME_EM_OID_CONTAINER_INL_H

namespace rubble  { namespace common { 
  // OidConstrainedString ///////////////////////////////////////////////////
  template<unsigned length>
  inline OidConstrainedString<char,length>::OidConstrainedString()
  {
    str_[0]='\0';
  }
  template<unsigned length>
  inline OidConstrainedString<char,length>::OidConstrainedString
  (const char * char_)
  {
    std::string str(char_);
    construct_(str); 
  };
  template<unsigned length>
  inline OidConstrainedString<char,length>::OidConstrainedString
  (const std::string & str)
  {
    construct_(str);
  };
  template<unsigned length>
  inline const char * OidConstrainedString<char,length>::c_str() const 
  {
    if( ! is_initialized() )
      throw std::invalid_argument("the string is uninitialized");
     
    return str_;
  };
  template<unsigned length>
  inline bool OidConstrainedString<char,length>::is_initialized() const
  {
    return (str_[0]=='\0') ? false : true;
  }
  template<unsigned length>
  inline bool OidConstrainedString<char,length>::
  operator< (const OidConstrainedString<char,length> & rhs) const
  {
    return (strncmp(str_, rhs.c_str(),length) < 0) ? true : false;
  }
  template<unsigned length>
  inline bool OidConstrainedString<char,length>::operator== 
  (const OidConstrainedString<char,length> & rhs) const
  {
    return (strncmp(str_,rhs.c_str(),length) == 0) ? true : false;
  }
  template<unsigned length>
  inline bool OidConstrainedString<char,length>::operator> 
  (const OidConstrainedString<char,length> & rhs) const
  {
    return (strncmp(str_,rhs.c_str(),length) > 0) ? true : false;
  }

  template<unsigned length>
  void OidConstrainedString<char,length>::construct_(const std::string & str)
  {
    if(str.length()+1 > length)
        throw std::length_error
            ("identifier string larger than identifier limit");
    std::string str2(str);
    boost::to_lower(str2);

    strncpy(str_, str2.c_str(), length);
  }
  //-----------------------------------------------------------------------//

  // OidType ////////////////////////////////////////////////////////////////
  template< typename str_type, typename size_type>
  inline OidType<str_type,size_type>::OidType()
    : name_(),ordinal_() 
  {
  }

  template< typename str_type, typename size_type>
  inline OidType<str_type,size_type>::OidType( 
        const std::string & str_in, 
        const boost::uint32_t ordinal_in)
    : name_(str_in), ordinal_() 
  { 
    if( ordinal_in > MAX )
      throw std::out_of_range("ordinal exceeds limit");

    ordinal_ = ordinal_in;
  }

  template< typename str_type, typename size_type>
  inline bool OidType<str_type,size_type>::operator==
    (const OidType<str_type,size_type> & rhs) const
  {
    if( rhs.name_ == this->name_
      && rhs.ordinal_ == this->ordinal_)
      return true;
    else
      return false; 
  }

  template< typename str_type, typename size_type>
  inline const str_type & OidType<str_type,size_type>::name() const
  { 
    return name_; 
  }

  template< typename str_type, typename size_type>
  inline const size_type OidType<str_type,size_type>::ordinal() const
  { 
    return ordinal_; 
  }

  template< typename str_type, typename size_type>
  inline void OidType<str_type,size_type>::set_name(const name_type & name_in)
  {
    name_ = name_in;    
  }
  template< typename str_type, typename size_type>
  inline void OidType<str_type,size_type>::set_ordinal
  ( const size_type & ordinal_in )
  {
    ordinal_  = ordinal_in;
  }
  //-----------------------------------------------------------------------//

  // OidContainerEntryType //////////////////////////////////////////////////
  template<typename identifier_type, typename _entry_type>
  inline OidContainerEntryType<identifier_type, _entry_type>::
  OidContainerEntryType() 
    : id_(), entry_()
  {
  }

  template<typename identifier_type, typename _entry_type>
  inline OidContainerEntryType<identifier_type, _entry_type>::
  OidContainerEntryType ( const identifier_type & id, const _entry_type & et )
    : id_(id), entry_(et) 
  {
  } 


  template<typename identifier_type, typename _entry_type>
  inline bool OidContainerEntryType<identifier_type, _entry_type>::
  is_initialized() const 
  { 
    return id_.name().is_initialized(); 
  }

  template<typename identifier_type, typename _entry_type>
  inline bool OidContainerEntryType<identifier_type, _entry_type>::
  operator<  
  ( const OidContainerEntryType<identifier_type,_entry_type> & rhs ) const
  { 
    return id_.name() < rhs.id_.name() ; 
  }


  template<typename identifier_type, typename _entry_type>
  inline bool OidContainerEntryType<identifier_type, _entry_type>::
  operator== 
  ( const OidContainerEntryType<identifier_type,_entry_type> & rhs ) const
  { 
    return id_.name() == rhs.id_.name() ; 
  }

  template<typename identifier_type, typename _entry_type>
  inline bool OidContainerEntryType<identifier_type, _entry_type>::
  operator> 
  ( const OidContainerEntryType<identifier_type, _entry_type> & rhs ) const
  { 
    return id_.name() > rhs.id_.name() ; 
  }


  template<typename identifier_type, typename _entry_type>
    template<typename ENTRY>
  inline OidContainerEntryType<identifier_type, _entry_type>::
  operator OidContainerEntryType<identifier_type, ENTRY>() const
  {
    OidContainerEntryType<identifier_type, ENTRY> ret(id_,(ENTRY)entry_);
    return ret;
  }

  template<typename identifier_type, typename _entry_type>
  inline const typename identifier_type::name_type & 
  OidContainerEntryType<identifier_type, _entry_type>::name() const
  {
    return id_.name();   
  }

  template<typename identifier_type, typename _entry_type>
  inline const typename identifier_type::ordinal_type 
  OidContainerEntryType<identifier_type, _entry_type>::ordinal() const
  {
    return id_.ordinal();
  }

  template<typename identifier_type, typename _entry_type>
  inline const _entry_type &
  OidContainerEntryType<identifier_type, _entry_type>::entry() const
  {
    return entry_;
  }

  template<typename identifier_type, typename _entry_type>
  inline const identifier_type & 
  OidContainerEntryType<identifier_type, _entry_type>::Id() const
  {  
    return id_;
  }

  template<typename identifier_type, typename _entry_type>
  inline void OidContainerEntryType<identifier_type, _entry_type>::
  set_identifier(const identifier_type & id_in)
  { 
    id_.set_name(id_in.name());
    id_.set_ordinal(id_in.ordinal());
  }

  template<typename identifier_type, typename _entry_type>
  inline void OidContainerEntryType<identifier_type, _entry_type>::
  set_entry(const basic_entry_type & entry_in)
  {
    entry_ = entry_in;
  }
  //-----------------------------------------------------------------------//

  // OidContainer ///////////////////////////////////////////////////////////

  template<typename _identifier_type, typename _entry_type>
  inline OidContainer<_identifier_type, _entry_type>::
  OidContainer()
  : entries_(),name_index_() 
  { 
  }

  template<typename _identifier_type, typename _entry_type>
  inline OidContainer<_identifier_type, _entry_type>::
  OidContainer(const OidContainer & rhs)
  {
    name_index_.clear();
    entries_ = rhs.entries_;
    regen_name_index_();
  }

  template<typename _identifier_type, typename _entry_type>
  inline OidContainer<_identifier_type, _entry_type> & 
  OidContainer<_identifier_type, _entry_type>::
  operator=(const OidContainer & oid)
  {
    if(this != &oid)
    {
      name_index_.clear();
      entries_ = oid.entries_;
      regen_name_index_();
    }

    return *this; 
  }

  template<typename _identifier_type, typename _entry_type>
  template<typename Id_T, typename Entry_T>
  inline void OidContainer<_identifier_type, _entry_type>::
  SlicingPopulate(OidContainer<Id_T,Entry_T> & target) const
  {
    std::size_t sz = size();
    target.clear();

    for(int i=0; i < sz; ++i)
    {
        const entry_type * et = EntryAtordinal(i);
        if(et != NULL)
        {
          typename OidContainer<Id_T,Entry_T>::entry_type tet = (*et);
          target.SetEntry(tet);
        }
    }
  }

  template<typename _identifier_type, typename _entry_type>
  inline void OidContainer<_identifier_type, _entry_type>::
  clear()
  {
    name_index_.clear();
    entries_.clear();
  } 


  template<typename _identifier_type, typename _entry_type>
  inline const typename 
  OidContainer<_identifier_type, _entry_type>::vector_type & 
  OidContainer<_identifier_type, _entry_type>::
  get_entries() const
  {
    return entries_;
  }

  template<typename _identifier_type, typename _entry_type>
  inline const std::size_t OidContainer<_identifier_type, _entry_type>::
  size() const 
  {
    return entries_.size();
  }

  template<typename _identifier_type, typename _entry_type>
  inline const std::size_t OidContainer<_identifier_type, _entry_type>::
  occupied_size() const
  {
    return name_index_.size();
  }


  template<typename _identifier_type, typename _entry_type>
  inline const typename 
  OidContainer<_identifier_type, _entry_type>::entry_type * 
  OidContainer<_identifier_type, _entry_type>::
  EntryAtordinal(boost::uint32_t ordinal) const
  {   
    try
    {
      const entry_type * e = &(entries_.at(ordinal));
      if(e->is_initialized())
        return e;
      else
        return NULL; 
    }
    catch(std::out_of_range)
    {
      return NULL; 
    }
  }

  template<typename _identifier_type, typename _entry_type>
  inline const typename  
  OidContainer<_identifier_type, _entry_type>::entry_type *
  OidContainer<_identifier_type, _entry_type>::
  EntryWithName 
  (const typename identifier_type::name_type & name_in) const
  {
    typename name_index_set::const_iterator it =
        name_index_.find(name_in, name_key_finder());
    
    if( it == name_index_.end() )
      return NULL;
    else
    {
      return &(*it);
    }
  }


  template<typename _identifier_type, typename _entry_type>
  inline OP_RESPONSE OidContainer<_identifier_type, _entry_type>::
  ContainsEither(const identifier_type & id) const
  {
    bool ordinal_in_vector=false;
   
    if(occupied_size() == 0)
      return OP_NO_ERROR;

    if(size() < (id.ordinal()+1))
      ordinal_in_vector=false;
    else if( entries_.at( id.ordinal() ).is_initialized())
      ordinal_in_vector=true;
    
    typename name_index_set::const_iterator it =
      name_index_.find(id.name(), name_key_finder());
    
    bool entry_name_used=true;

    if( it == name_index_.end())
      entry_name_used=false;

    if( ordinal_in_vector != entry_name_used ) {
      if( ordinal_in_vector)
        return OP_ORDINAL_USED;
      else
        return OP_NAME_USED;
    }
    
    if(entry_name_used)
      return OP_ALLREADY_CONTAINS_ENTRY;
    
    return OP_NO_ERROR;
  }

  template<typename _identifier_type, typename _entry_type>
  inline const typename  OidContainer<_identifier_type, _entry_type>::
  basic_entry_type *
  OidContainer<_identifier_type, _entry_type>::
  operator[] (const name_type & name) const 
  {
    const entry_type * ent = EntryWithName(name);

    if( ent != NULL )
      return (& ( ent->entry() ) );
    else return NULL;
  }

  template<typename _identifier_type, typename _entry_type>
  inline typename  OidContainer<_identifier_type, _entry_type>::
  basic_entry_type *
  OidContainer<_identifier_type, _entry_type>::
  operator[] (const name_type & name)   
  {
    return const_cast<basic_entry_type *>(
      static_cast<const OidContainer<_identifier_type, _entry_type> *>
        (this)->operator[](name));

  }

  template<typename _identifier_type, typename _entry_type>
  inline const typename OidContainer<_identifier_type, _entry_type>::
  basic_entry_type *
  OidContainer<_identifier_type, _entry_type>::
  operator[] (const ordinal_type & ordinal) const
  {
    const entry_type * ent = EntryAtordinal(ordinal);
    if( ent != NULL) {
      return (& ( ent->entry() ) );
    }
    else return NULL;
  }
  
  template<typename _identifier_type, typename _entry_type>
  inline typename OidContainer<_identifier_type, _entry_type>::
  basic_entry_type *
  OidContainer<_identifier_type, _entry_type>::
  operator[] (const ordinal_type & ordinal)   
  {
    return const_cast<basic_entry_type *>(
      static_cast<const OidContainer<_identifier_type, _entry_type> *>
        (this)->operator[](ordinal));
  }

  template<typename _identifier_type, typename _entry_type>
  inline OP_RESPONSE OidContainer<_identifier_type, _entry_type>::
  SetEntry(const entry_type & entry)
  {
    resize_if_needed_(entry.ordinal() );

    OP_RESPONSE ret = ContainsEither(entry.Id());
    if (ret != OP_NO_ERROR)
      return ret;
    
    entries_.at(entry.ordinal()) = entry;
    name_index_.insert( entries_.at( entry.ordinal() )); 
    
    return OP_NO_ERROR;
  } 

  template<typename _identifier_type, typename _entry_type>
  inline OP_RESPONSE OidContainer<_identifier_type, _entry_type>::
  SetEntry( const identifier_type & id, const basic_entry_type & entry)
  {
    SetEntry( entry_type(id,entry) );
  }

  template<typename _identifier_type, typename _entry_type>
  inline void OidContainer<_identifier_type, _entry_type>::
  resize_if_needed_(boost::uint32_t ordinal)
  {
    std::size_t old_size = entries_.size();
    if( old_size < ordinal + 1) {
      name_index_.clear();
      entries_.resize(ordinal+1 );
      if(old_size != 0) 
        regen_name_index_();
    }
  }

  template<typename _identifier_type, typename _entry_type>
  inline void OidContainer<_identifier_type, _entry_type>::
  regen_name_index_()
  {
    // the intrusive container needs to be clear()'d before or vector
    // resizing will fail an assertion. therefore this isn't handled
    // here.
    typedef typename std::vector<entry_type>::iterator
      it_type;

    it_type beg = entries_.begin();
    it_type end = entries_.end();

                
    for(it_type it=beg;it != end; it++)
    {
      if(it->is_initialized())
        name_index_.insert(*it);        
    }        
  }

  // functor 
  template<typename _identifier_type, typename _entry_type>
  struct OidContainer<_identifier_type, _entry_type>::name_key_finder
  {
    bool operator()(    const typename identifier_type::name_type & name, 
                        const entry_type & entry) const
    {
      return ( name < entry.name() );
    }
    bool operator()(    const entry_type & entry,
                        const typename identifier_type::name_type & name) const
    {
      return ( entry.name() < name);
    }
  };
  //-----------------------------------------------------------------------//

} }

#endif
