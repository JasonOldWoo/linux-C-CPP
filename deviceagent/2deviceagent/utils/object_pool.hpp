#ifndef __OBJECT_POOL_H__
#define __OBJECT_POOL_H__

namespace zl_device_agent {
class object_pool_access
{
public:
  template <typename Object>
  static Object* create()
  {
	  return new Object;
  }

  template <typename Object>
  static void destroy(Object* o)
  {
	  delete o;
  }

  template <typename Object>
  static Object*& next(Object* o)
  {
	  return o->next_;
  }

  template <typename Object>
  static Object*& prev(Object* o)
  {
	  return o->prev_;
  }
};

template <typename Object>
class object_pool
{
public:
  object_pool()
    : live_list_(0),
      free_list_(0)
  {
  }

  ~object_pool()
  {
    destroy_list(live_list_);
    destroy_list(free_list_);
  }

  Object* first()
  {
    return live_list_;
  }

  Object* alloc()
  {
    Object* o = free_list_;
    if (o)
      free_list_ = object_pool_access::next(free_list_);
    else
      o = object_pool_access::create<Object>();

    object_pool_access::next(o) = live_list_;
    object_pool_access::prev(o) = 0;
    if (live_list_)
      object_pool_access::prev(live_list_) = o;
    live_list_ = o;

    return o;
  }

  void free(Object* o)
  {
    if (live_list_ == o)
      live_list_ = object_pool_access::next(o);

    if (object_pool_access::prev(o))
    {
      object_pool_access::next(object_pool_access::prev(o))
        = object_pool_access::next(o);
    }

    if (object_pool_access::next(o))
    {
      object_pool_access::prev(object_pool_access::next(o))
        = object_pool_access::prev(o);
    }

    object_pool_access::next(o) = free_list_;
    object_pool_access::prev(o) = 0;
    free_list_ = o;
  }

private:
  void destroy_list(Object* list)
  {
    while (list)
    {
      Object* o = list;
      list = object_pool_access::next(o);
      object_pool_access::destroy(o);
    }
  }

  Object* live_list_;

  Object* free_list_;
};

}	// namespace zl_device_agent

#endif	// __object_pool_h__
