#ifndef _STDTIME_H
#define _STDTIME_H

#include <ctime>
#include <stdint.h>
#include <cassert>
#include <string>
#include <iostream>
#include <sstream>
#include <cmath>

#include <chrono>

#ifdef WIN32
#include <windows.h>
#endif

#ifdef __unix__
#include <sys/time.h>
#include <unistd.h>
#endif

class Time {
public:
  Time() {}

  explicit Time(int64_t usec):m_usec(usec){}
  explicit Time(int32_t sec,int32_t usec):m_usec(sec*1000000 + usec){}

  Time(const Time& rhs)
    :m_usec(rhs.m_usec) {}

  /// Math Operations
  Time operator+(const Time& t) const {
    return Time(m_usec + t.m_usec);
  }
  Time operator-(const Time& t) const {
    return Time(m_usec - t.m_usec);
  }
  Time operator*(const Time& t) const {
    return Time(m_usec * t.m_usec);
  }
  Time operator/(const Time& t) const {
    return Time(m_usec / t.m_usec);
  }

  Time& operator+=(const Time& rhs)
  {
    return (*this) = (*this) + rhs;
  }
  Time& operator-=(const Time& rhs)
  {
    return (*this) = (*this) - rhs;
  }
  Time& operator*=(const Time& rhs)
  {
    return (*this) = (*this) * rhs;
  }
  Time& operator/=(const Time& rhs)
  {
    return (*this) = (*this) / rhs;
  }

  /// Comparison
  bool operator<(const Time& t) const {
    return m_usec < t.m_usec;
  }
  bool operator>(const Time& t) const {
    return m_usec > t.m_usec;
  }

  /// SLEEP
  static void sleep(const Time& t){
#ifdef WIN32
    Sleep(t.total_usec());
#elif defined(__unix__)
    usleep(t.total_usec());
#endif
  }

  static void sleep_until(const Time& until){
    Time tnow = now();
    assert(until > tnow);
    sleep(until - tnow);
  }

  static void context_switch(){
    Time::sleep(Time(0,0));
  }

  static Time now(){
#ifdef WIN32
    LARGE_INTEGER temp;
    QueryPerformanceCounter(&temp);

    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);

    extern Time systemboot;

    return Time( temp.QuadPart / freq.QuadPart,
                 ( double(temp.QuadPart % freq.QuadPart)
                   / double(freq.QuadPart)) * 1000000) + systemboot;
#endif
#ifdef __unix__
    timeval tv;
    gettimeofday( &tv, 0);

    return Time( tv.tv_sec, tv.tv_usec);
#endif

    /*
        int64_t temp = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        return Time(temp/1000000,temp%1000000);
        */
  }

#define _out(X,Y); if(X) s<<X<<Y;
  operator std::string() const{
    std::stringstream s;

    struct tm* ptm;
    time_t t = total_sec();
    ptm = gmtime(&t);

    _out(ptm->tm_year - 70,"years ");
    _out(ptm->tm_yday,"days ");
    _out(ptm->tm_hour,"hours ");
    _out(ptm->tm_min,"mins ");
    _out(ptm->tm_sec,"s ");

    if(uint32_t ms = msec())
      s<<ms<<"ms ";
    //s<<usec()<<u8"µs";
    return s.str();
  }

  std::string UTC(){
    std::stringstream s;

    struct tm* ptm;
    time_t t = total_sec();
    ptm = gmtime(&t);
    //ptm->tm_year += 1900;

    //s<< std::put_time(ptm,"%c %Z");
    //_out(ptm->tm_year,"years ");
    //_out(ptm->tm_yday,"days ");
    //_out(ptm->tm_hour,"hours ");
    //_out(ptm->tm_min,"mins ");
    //_out(ptm->tm_sec,"s ");

    /// Light FIXME: change buf size to the max possible formatted date size
    char buf[1000];
    size_t l = strftime(buf,sizeof(buf), "%Y %B %d (%A) %X", ptm);

    s<< std::string(buf, l);
    if(uint32_t ms = msec())
      s<<':'<<ms;
    s<<':'<<usec();
    return s.str();
  }
#undef _out

  int32_t sec()  const{ return m_usec / 1000000;}
  int64_t msec() const{ return m_usec / 1000 % 1000;}
  int64_t usec() const{ return m_usec % 1000;}

  int64_t total_sec()  const{ return m_usec/1000000;}
  int64_t total_msec() const{ return m_usec/1000;}
  int64_t total_usec() const{ return m_usec;}
private:
  friend std::ostream& operator<<(std::ostream& strm, const Time& t);

  int64_t m_usec;
};

inline std::ostream& operator<<(std::ostream& strm, const Time& t){
  return (strm << (std::string)t);
}


#ifdef WIN32
extern Time systemboot;
#endif
#endif // _TIME_H

