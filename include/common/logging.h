#ifndef RBL_LOGGING_H
#define RBL_LOGGING_H

namespace rubble {

class LogProcessContext
{
  LOG_SERVICE_TYPE          m_service_type;
};

class LogMessage
{
  boost::posix_time::ptime  m_timestamp;
  LogProcessContext        m_process_context;
  std::string               m_log_message;
};

} // namespace rubble
#endif 
