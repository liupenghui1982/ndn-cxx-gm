/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2013 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * @author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 * @author: Zhenkai Zhu <zhenkai@cs.ucla.edu>
 * See COPYING for copyright and distribution information.
 */

#ifndef NDN_NAME_HPP
#define NDN_NAME_HPP

#include "common.hpp"
#include "name-component.hpp"

#include "encoding/block.hpp"
#include "encoding/encoding-buffer.hpp"

#include <boost/iterator/reverse_iterator.hpp>

namespace ndn {

/**
 * A Name holds an array of Name::Component and represents an NDN name.
 */
class Name : public ptr_lib::enable_shared_from_this<Name> {
public:
  /// @brief Error that can be thrown from the block
  struct Error : public name::Component::Error { Error(const std::string &what) : name::Component::Error(what) {} };

  typedef name::Component Component;

  typedef std::vector<Component>  component_container;

  typedef Component               value_type;
  typedef void                    allocator_type;
  typedef Component&              reference;
  typedef const Component         const_reference;
  typedef Component*              pointer;
  typedef const Component*        const_pointer;
  typedef Component*              iterator;
  typedef const Component*        const_iterator;
  
  typedef boost::reverse_iterator<iterator>       reverse_iterator;
  typedef boost::reverse_iterator<const_iterator> const_reverse_iterator;

  typedef component_container::difference_type difference_type;
  typedef component_container::size_type       size_type;
  
  /**
   * Create a new Name with no components.
   */
  Name()
    : m_nameBlock(Tlv::Name)
  {
  }

  /**
   * @brief Create Name object from wire block
   *
   * This is a more efficient equivalent for
   * @code
   *    Name name;
   *    name.wireDecode(wire);
   * @endcode
   */
  Name(const Block &wire)
  {
    m_nameBlock = wire;
    m_nameBlock.parse();
  }
  
  /**
   * Parse the uri according to the NDN URI Scheme and create the name with the components.
   * @param uri The URI string.
   */
  Name(const char* uri)
  {
    set(uri);
  }
  
  /**
   * Parse the uri according to the NDN URI Scheme and create the name with the components.
   * @param uri The URI string.
   */
  Name(const std::string& uri)
  {
    set(uri.c_str());
  }

  /**
   * @brief Fast encoding or block size estimation
   */
  template<bool T>
  size_t
  wireEncode(EncodingImpl<T> &block) const;
  
  const Block &
  wireEncode() const;

  void
  wireDecode(const Block &wire);
  
  /**
   * Parse the uri according to the NDN URI Scheme and set the name with the components.
   * @param uri The null-terminated URI string.
   */
  void 
  set(const char *uri);  

  /**
   * Parse the uri according to the NDN URI Scheme and set the name with the components.
   * @param uri The URI string.
   */
  void 
  set(const std::string& uri)
  {
    set(uri.c_str());
  }  
  
  /**
   * Append a new component, copying from value of length valueLength.
   * @return This name so that you can chain calls to append.
   */
  Name& 
  append(const uint8_t *value, size_t valueLength) 
  {
    m_nameBlock.push_back(Component(value, valueLength));
    return *this;
  }

  /**
   * Append a new component, copying from value of length valueLength.
   * @return This name so that you can chain calls to append.
   */
  template<class InputIterator>
  Name&
  append(InputIterator begin, InputIterator end)
  {
    m_nameBlock.push_back(Component(begin, end));
    return *this;
  }

  // /**
  //  * Append a new component, copying from value.
  //  * @return This name so that you can chain calls to append.
  //  */
  // Name& 
  // append(const Buffer& value) 
  // {
  //   m_nameBlock.push_back(value);
  //   return *this;
  // }
  
  Name& 
  append(const ConstBufferPtr &value)
  {
    m_nameBlock.push_back(value);
    return *this;
  }
  
  Name& 
  append(const Component &value)
  {
    m_nameBlock.push_back(value);
    return *this;
  }

  /**
   * @brief Append name component that represented as a string
   *
   * Note that this method is necessary to ensure correctness and unambiguity of
   * ``append("string")`` operations (both Component and Name can be implicitly
   * converted from string, each having different outcomes
   */
  Name& 
  append(const char *value)
  {
    m_nameBlock.push_back(Component(value));
    return *this;
  }
  
  Name&
  append(const Block &value)
  {
    if (value.type() == Tlv::NameComponent)
      m_nameBlock.push_back(value);
    else
      m_nameBlock.push_back(Block(Tlv::NameComponent, value));

    return *this;
  }
  
  /**
   * Append the components of the given name to this name.
   * @param name The Name with components to append.
   * @return This name so that you can chain calls to append.
   */
  Name&
  append(const Name& name);
    
  /**
   * Clear all the components.
   */
  void 
  clear()
  {
    m_nameBlock = Block(Tlv::Name);
  }
  
  /**
   * Get a new name, constructed as a subset of components.
   * @param iStartComponent The index if the first component to get.
   * @param nComponents The number of components starting at iStartComponent.
   * @return A new name.
   */
  Name
  getSubName(size_t iStartComponent, size_t nComponents) const;

  /**
   * Get a new name, constructed as a subset of components starting at iStartComponent until the end of the name.
   * @param iStartComponent The index if the first component to get.
   * @return A new name.
   */
  Name
  getSubName(size_t iStartComponent) const;
  
  /**
   * Return a new Name with the first nComponents components of this Name.
   * @param nComponents The number of prefix components.  If nComponents is -N then return the prefix up
   * to name.size() - N. For example getPrefix(-1) returns the name without the final component.
   * @return A new Name.
   */
  Name
  getPrefix(int nComponents) const
  {
    if (nComponents < 0)
      return getSubName(0, m_nameBlock.elements_size() + nComponents);
    else
      return getSubName(0, nComponents);
  }
  
  /**
   * Encode this name as a URI.
   * @return The encoded URI.
   */
  std::string 
  toUri() const;
  
  /**
   * Append a component with the encoded segment number.
   * @param segment The segment number.
   * @return This name so that you can chain calls to append.
   */  
  Name& 
  appendSegment(uint64_t segment)
  {
    m_nameBlock.push_back(Component::fromNumberWithMarker(segment, 0x00));
    return *this;
  }

  /**
   * Append a component with the encoded version number.
   * Note that this encodes the exact value of version without converting from a time representation.
   * @param version The version number.
   * @return This name so that you can chain calls to append.
   */  
  Name& 
  appendVersion(uint64_t version)
  {
    m_nameBlock.push_back(Component::fromNumberWithMarker(version, 0xFD));
    return *this;
  }

  /**
   * @brief Append a component with the encoded version number.
   * 
   * This version of the method creates version number based on the current timestamp
   * @return This name so that you can chain calls to append.
   */  
  Name& 
  appendVersion();
  
  /**
   * Check if this name has the same component count and components as the given name.
   * @param name The Name to check.
   * @return true if the names are equal, otherwise false.
   */
  bool
  equals(const Name& name) const;
  
  /**
   * Check if the N components of this name are the same as the first N components of the given name.
   * @param name The Name to check.
   * @return true if this matches the given name, otherwise false.  This always returns true if this name is empty.
   */
  bool 
  isPrefixOf(const Name& name) const;

  bool
  match(const Name& name) const
  {
    return isPrefixOf(name);
  }
  
  //
  // vector equivalent interface.
  //

  /**
   * @brief Check if name is emtpy
   */
  bool
  empty() const { return m_nameBlock.elements().empty(); }
  
  /**
   * Get the number of components.
   * @return The number of components.
   */
  size_t 
  size() const { return m_nameBlock.elements_size(); }

  /**
   * Get the component at the given index.
   * @param i The index of the component, starting from 0.
   * @return The name component at the index.
   */
  const Component& 
  get(ssize_t i) const
  {
    if (i >= 0)
      return reinterpret_cast<const Component&>(m_nameBlock.elements()[i]);
    else
      return reinterpret_cast<const Component&>(m_nameBlock.elements()[size() + i]);
  }

  const Component&
  operator [] (ssize_t i) const
  {
    return get(i);
  }
  
  const Component&
  at(ssize_t i) const
  {
    return get(i);
  }

  /**
   * Compare this to the other Name using NDN canonical ordering.  If the first components of each name are not equal, 
   * this returns -1 if the first comes before the second using the NDN canonical ordering for name components, or 1 if it comes after.
   * If they are equal, this compares the second components of each name, etc.  If both names are the same up to
   * the size of the shorter name, this returns -1 if the first name is shorter than the second or 1 if it is longer.  
   * For example, if you std::sort gives: /a/b/d /a/b/cc /c /c/a /bb .  This is intuitive because all names
   * with the prefix /a are next to each other.  But it may be also be counter-intuitive because /c comes before /bb 
   * according to NDN canonical ordering since it is shorter.  
   * @param other The other Name to compare with.
   * @return 0 If they compare equal, -1 if *this comes before other in the canonical ordering, or
   * 1 if *this comes after other in the canonical ordering.
   *
   * @see http://named-data.net/doc/0.2/technical/CanonicalOrder.html
   */
  int
  compare(const Name& other) const;

  /**
   * Append the component
   * @param component The component of type T.
   */
  template<class T> void
  push_back(const T &component)
  {
    append(component);
  }
  
  /**
   * Check if this name has the same component count and components as the given name.
   * @param name The Name to check.
   * @return true if the names are equal, otherwise false.
   */
  bool
  operator == (const Name &name) const { return equals(name); }

  /**
   * Check if this name has the same component count and components as the given name.
   * @param name The Name to check.
   * @return true if the names are not equal, otherwise false.
   */
  bool
  operator != (const Name &name) const { return !equals(name); }
  
  /**
   * Return true if this is less than or equal to the other Name in the NDN canonical ordering.
   * @param other The other Name to compare with.
   *
   * @see http://named-data.net/doc/0.2/technical/CanonicalOrder.html
   */
  bool
  operator <= (const Name& other) const { return compare(other) <= 0; }

  /**
   * Return true if this is less than the other Name in the NDN canonical ordering.
   * @param other The other Name to compare with.
   *
   * @see http://named-data.net/doc/0.2/technical/CanonicalOrder.html
   */
  bool
  operator < (const Name& other) const { return compare(other) < 0; }

  /**
   * Return true if this is less than or equal to the other Name in the NDN canonical ordering.
   * @param other The other Name to compare with.
   *
   * @see http://named-data.net/doc/0.2/technical/CanonicalOrder.html
   */
  bool
  operator >= (const Name& other) const { return compare(other) >= 0; }

  /**
   * Return true if this is greater than the other Name in the NDN canonical ordering.
   * @param other The other Name to compare with.
   *
   * @see http://named-data.net/doc/0.2/technical/CanonicalOrder.html
   */
  bool
  operator > (const Name& other) const { return compare(other) > 0; }
  
  //
  // Iterator interface to name components.
  //

  /**
   * Begin iterator (const).
   */
  const_iterator
  begin() const
  {
    return reinterpret_cast<const_iterator>(&*m_nameBlock.elements().begin());
  }

  /**
   * End iterator (const).
   *
   * @todo Check if this crash when there are no elements in the buffer
   */
  const_iterator
  end() const
  {
    return reinterpret_cast<const_iterator>(&*m_nameBlock.elements().end());
  }

  /**
   * Reverse begin iterator (const).
   */
  const_reverse_iterator
  rbegin() const
  {
    return const_reverse_iterator(end());
  }

  /**
   * Reverse end iterator (const).
   */
  const_reverse_iterator
  rend() const
  {
    return const_reverse_iterator(begin());
  }

private:
  mutable Block m_nameBlock;
};

std::ostream &
operator << (std::ostream &os, const Name &name);

inline std::string 
Name::toUri() const
{
  std::ostringstream os;
  os << *this;
  return os.str();
}

inline const Block &
Name::wireEncode() const
{
  if (m_nameBlock.hasWire())
    return m_nameBlock;

  EncodingEstimator estimator;
  size_t estimatedSize = wireEncode(estimator);
  
  EncodingBuffer buffer(estimatedSize, 0);
  wireEncode(buffer);

  m_nameBlock = buffer.block();
  m_nameBlock.parse();
  
  return m_nameBlock;
}

inline void
Name::wireDecode(const Block &wire)
{
  if (wire.type() != Tlv::Name)
    throw Tlv::Error("Unexpected TLV type when decoding Name");
  
  m_nameBlock = wire;
  m_nameBlock.parse();
}

template<bool T>
inline size_t
Name::wireEncode(EncodingImpl<T>& blk) const
{
  size_t total_len = 0;
  
  for (const_reverse_iterator i = rbegin (); 
       i != rend ();
       ++i)
    {
      total_len += i->wireEncode (blk);
    }

  total_len += blk.prependVarNumber (total_len);
  total_len += blk.prependVarNumber (Tlv::Name);
  return total_len;
}

} // namespace ndn

#endif
