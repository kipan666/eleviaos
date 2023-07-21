#ifndef __TRACE__TRACE_HPP_
#define __TRACE__TRACE_HPP_

namespace trace {
class Trace {
public:
  static void init();

private:
  static inline Trace *trace;
};
} // namespace trace

#endif // __TRACE__TRACE_HPP_