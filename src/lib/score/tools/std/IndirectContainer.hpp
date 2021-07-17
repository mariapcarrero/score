#pragma once
#include <array>
#include <memory>
#include <vector>

namespace score
{
template <typename base_iterator_t>
struct indirect_iterator
{
  using self_type = indirect_iterator;
  using iterator = self_type;
  using const_iterator = self_type;
  using value_type = std::remove_reference_t<decltype(
      *std::declval<typename base_iterator_t::value_type>())>;
  using reference = value_type&;
  using pointer = value_type*;
  using iterator_category = std::forward_iterator_tag;
  using difference_type = int;

  base_iterator_t it;

  self_type operator++()
  {
    ++it;
    return *this;
  }
  self_type operator++(int)
  {
    self_type i = *this;
    it++;
    return i;
  }

  value_type& operator*() { return **it; }
  value_type* operator->() { return *it; }
  bool operator==(const self_type& rhs) const { return it == rhs.it; }
  bool operator!=(const self_type& rhs) const { return it != rhs.it; }
  bool operator<(const self_type& rhs) const { return it < rhs.it; }
};

template <typename T>
indirect_iterator<T> make_indirect_iterator(const T& it)
{
  return indirect_iterator<T>{it};
}

template <typename base_iterator_t>
struct indirect_ptr_iterator
{
  using self_type = indirect_ptr_iterator;
  using iterator = self_type;
  using const_iterator = self_type;
  using value_type
      = std::remove_reference_t<decltype(*std::declval<base_iterator_t>())>;
  using reference = value_type&;
  using pointer = value_type*;
  using iterator_category = std::forward_iterator_tag;
  using difference_type = int;

  base_iterator_t it;

  self_type operator++()
  {
    ++it;
    return *this;
  }
  self_type operator++(int)
  {
    self_type i = *this;
    it++;
    return i;
  }

  auto& operator*() { return **it; }
  auto operator->() { return it; }
  bool operator==(const self_type& rhs) const { return it == rhs.it; }
  bool operator!=(const self_type& rhs) const { return it != rhs.it; }
  bool operator<(const self_type& rhs) const { return it < rhs.it; }
};

template <typename T>
indirect_ptr_iterator<T> make_indirect_ptr_iterator(const T& it)
{
  return indirect_ptr_iterator<T>{it};
}

template <typename base_iterator_t>
struct indirect_map_iterator
{
  using self_type = indirect_map_iterator;
  using iterator = self_type;
  using const_iterator = self_type;
  using value_type = std::remove_reference_t<decltype(
      *std::declval<typename base_iterator_t::value_type::second_type>())>;
  using reference = value_type&;
  using pointer = value_type*;
  using iterator_category = std::forward_iterator_tag;
  using difference_type = int;

  base_iterator_t it;

  self_type operator++()
  {
    ++it;
    return *this;
  }
  self_type operator++(int)
  {
    self_type i = *this;
    it++;
    return i;
  }

  auto& operator*() { return *it->second; }
  auto operator->() { return it->second.get(); }
  bool operator==(const self_type& rhs) const { return it == rhs.it; }
  bool operator!=(const self_type& rhs) const { return it != rhs.it; }
  bool operator<(const self_type& rhs) const { return it < rhs.it; }
};

template <typename T>
indirect_map_iterator<T> make_indirect_map_iterator(const T& it)
{
  return indirect_map_iterator<T>{it};
}

template <typename T, typename U = std::allocator<T*>>
class IndirectContainer : std::vector<T*, U>
{
public:
  using ctnr_t = std::vector<T*, U>;
  using ctnr_t::ctnr_t;
  using value_type = T;

  auto begin() noexcept { return make_indirect_ptr_iterator(ctnr_t::begin()); }
  auto end() noexcept { return make_indirect_ptr_iterator(ctnr_t::end()); }
  auto begin() const noexcept { return make_indirect_ptr_iterator(ctnr_t::begin()); }
  auto end() const noexcept { return make_indirect_ptr_iterator(ctnr_t::end()); }

  auto rbegin() noexcept { return make_indirect_ptr_iterator(ctnr_t::rbegin()); }
  auto rend() noexcept { return make_indirect_ptr_iterator(ctnr_t::rend()); }
  auto rbegin() const noexcept { return make_indirect_ptr_iterator(ctnr_t::rbegin()); }
  auto rend() const noexcept { return make_indirect_ptr_iterator(ctnr_t::rend()); }

  auto cbegin() const noexcept { return make_indirect_ptr_iterator(ctnr_t::cbegin()); }
  auto cend() const noexcept { return make_indirect_ptr_iterator(ctnr_t::cend()); }

  auto size() const noexcept { return ctnr_t::size(); }
  auto empty() const noexcept { return ctnr_t::empty(); }

  auto push_back(T* ptr) { return ctnr_t::push_back(ptr); }

  auto& front() const noexcept { return *ctnr_t::front(); }
  auto& back() const noexcept { return *ctnr_t::back(); }

  auto& operator[](int pos) noexcept { return *ctnr_t::operator[](pos); }
  auto& operator[](int pos) const noexcept { return *ctnr_t::operator[](pos); }
};

template <class Container>
class IndirectContainerWrapper
{
public:
  Container& container;

  auto begin() { return make_indirect_iterator(container.begin()); }
  auto end() { return make_indirect_iterator(container.end()); }
  auto begin() const { return make_indirect_iterator(container.begin()); }
  auto end() const { return make_indirect_iterator(container.end()); }
  auto cbegin() const { return make_indirect_iterator(container.cbegin()); }
  auto cend() const { return make_indirect_iterator(container.cend()); }
};

template <typename T>
auto wrap_indirect(T& container)
{
  return IndirectContainerWrapper<T>{container};
}

template <typename T, int N>
class IndirectArray
{
  std::array<T*, N> array;

public:
  using value_type = T;
  template <typename... Args>
  IndirectArray(Args&&... args)
      : array{{std::forward<Args>(args)...}}
  {
  }

  auto begin() { return make_indirect_ptr_iterator(array.begin()); }
  auto end() { return make_indirect_ptr_iterator(array.end()); }
  auto begin() const { return make_indirect_ptr_iterator(array.begin()); }
  auto end() const { return make_indirect_ptr_iterator(array.end()); }
  auto cbegin() const { return make_indirect_ptr_iterator(array.cbegin()); }
  auto cend() const { return make_indirect_ptr_iterator(array.cend()); }

  auto& operator[](int pos) { return *array[pos]; }
  auto& operator[](int pos) const { return *array[pos]; }
};

template <typename Map_T>
class IndirectMap
{
public:
  auto begin() { return make_indirect_iterator(map.begin()); }
  auto begin() const { return make_indirect_iterator(map.begin()); }

  auto cbegin() { return make_indirect_iterator(map.cbegin()); }
  auto cbegin() const { return make_indirect_iterator(map.cbegin()); }

  auto end() { return make_indirect_iterator(map.end()); }
  auto end() const { return make_indirect_iterator(map.end()); }

  auto cend() { return make_indirect_iterator(map.cend()); }
  auto cend() const { return make_indirect_iterator(map.cend()); }

  auto empty() const { return map.empty(); }

  template <typename K>
  auto find(K&& key)
  {
    return map.find(std::forward<K>(key));
  }

  template <typename E>
  auto insert(E&& elt)
  {
    return map.insert(std::forward<E>(elt));
  }

protected:
  Map_T map;
};

template <typename Map_T>
class IndirectUnorderedMap
{
  using base_iterator_t = typename Map_T::iterator;
  using base_const_iterator_t = typename Map_T::const_iterator;

public:
  using value_type = typename base_iterator_t::value_type;
  IndirectUnorderedMap() { }

  auto begin() { return make_indirect_map_iterator(map.begin()); }
  auto begin() const { return make_indirect_map_iterator(map.begin()); }

  auto cbegin() { return make_indirect_map_iterator(map.cbegin()); }
  auto cbegin() const { return make_indirect_map_iterator(map.cbegin()); }

  auto end() { return make_indirect_map_iterator(map.end()); }
  auto end() const { return make_indirect_map_iterator(map.end()); }

  auto cend() { return make_indirect_map_iterator(map.cend()); }
  auto cend() const { return make_indirect_map_iterator(map.cend()); }

  auto empty() const { return map.empty(); }

  template <typename K>
  auto find(K&& key) const
  {
    return make_indirect_map_iterator(map.find(std::forward<K>(key)));
  }

  template <typename E>
  auto insert(E&& elt)
  {
    return map.insert(std::forward<E>(elt));
  }

protected:
  Map_T map;

private:
  IndirectUnorderedMap(const IndirectUnorderedMap&) = delete;
  IndirectUnorderedMap(IndirectUnorderedMap&&) = delete;
  IndirectUnorderedMap& operator=(const IndirectUnorderedMap&) = delete;
  IndirectUnorderedMap& operator=(IndirectUnorderedMap&&) = delete;
};
}
