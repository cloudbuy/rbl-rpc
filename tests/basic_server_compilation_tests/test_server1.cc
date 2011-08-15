#include <rpc/backend/BackEndBase.h>
#include <boost/thread.hpp>
#include <boost/scoped_array.hpp>

using namespace rubble::rpc;

struct notification_object
{
  notification_object() : ready(false){}
  bool ready;
  boost::mutex mutex;
  boost::condition_variable cond;

  void reset()
  {
    ready = false;
  }
};

struct hello
{
  void operator()()
  {
    boost::unique_lock<boost::mutex> lock(no->mutex);
    std::cout << boost::this_thread::get_id() << "  hello world" << std::endl;
    no->ready = true;
    no->cond.notify_one();
  }
  notification_object * no;
};

void dispatch(int count)
{
  
  BackEnd b(basic_protocol::SOURCE_RELAY,basic_protocol::TARGET_MARSHALL);
  
  b.pool_size(4);
  b.start();

  hello h1; 
  hello h2;

  notification_object no1;  
  notification_object no2;  

  h1.no =  & no1;
  h2.no =  & no2;
/*
  for(int i=0;i < count/2; ++i)
  {
    b.invoke(h1);
    b.invoke(h2);

    boost::unique_lock<boost::mutex> lock(h1.no->mutex);
    if(!h1.no->ready)
      h1.no->cond.wait(lock);
    
    boost::unique_lock<boost::mutex> lock2(h2.no->mutex);
    if(!h2.no->ready)
      h2.no->cond.wait(lock2);

    h1.no->reset();
    h2.no->reset();
  }
*/
}

int main()
{
  dispatch(40);
}
