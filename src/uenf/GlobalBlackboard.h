$header

template<typename T> class GlobalBlackboard
{
public:
  static boost::shared_ptr<T> & const operator[](std::string & const key) const
  {
    boost::guarded_lock(mutex);
    return mapping[key];
  }

  static boost::shared_ptr<T> & operator[](std::string & const key)
  {
    return mapping[key];
  }

private:
  static std::map<std::string, boost::shared_ptr<T> > mapping;
};