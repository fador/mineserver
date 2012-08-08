#ifndef _STDTIME_H
#define _STDTIME_H

#include <ctime>
#include <stdint.h>
#include <unistd.h>
#include <cassert>
#include <string>
#include <iostream>
#include <sstream>
#include <cmath>

#include <chrono>

#ifdef __WIN32__
#include <windows.h>

#endif

#ifdef __unix__
#include <sys/time.h>
#endif

class Time {
public:
    Time() {}
    Time(uint32_t sec,uint32_t usec):m_sec(sec),m_usec(usec){}

    Time  operator-(const Time& t) const {
        uint32_t sec = m_sec - t.m_sec;
        int usec = m_usec - t.m_usec;
        if(usec<0){
            int delta = std::ceil(double(-usec)/1000000);
            sec  -= delta;
            usec += delta*1000000;
        }
        return Time(sec,usec);
    }
    Time  operator+(const Time& t) const {
        uint32_t sec = m_sec + t.m_sec;
        uint32_t usec = m_usec + t.m_usec;
        if(usec>1000000){
            sec++;
            usec-=1000000;
        }
        return Time(sec,usec);
    }

    Time operator/(unsigned i) const{
        double sec = m_sec + double(m_usec)/1000000;
        sec/= i;
        return Time(std::floor(sec),
                    std::fmod(sec,1)*1000000);
    }
    bool         operator<(const Time& t) const {
        if(m_sec < t.m_sec) return true;
        else if(m_sec == t.m_sec){
            if(m_usec < t.m_usec) return true;
        }
        else return false;
    }
    bool         operator>(const Time& t) const {
        if(m_sec > t.m_sec) return true;
        else if(m_sec == t.m_sec){
            if(m_usec > t.m_usec) return true;
        }
        else return false;
    }
    bool         operator<=(const Time& t) const {
        if(m_sec < t.m_sec) return true;
        else if(m_sec == t.m_sec){
            if(m_usec < t.m_usec) return true;
            else if(m_usec == m_usec) return true;
        }
        else return false;
    }
    bool         operator>=(const Time& t) const {
        if(m_sec > t.m_sec) return true;
        else if(m_sec == t.m_sec){
            if(m_usec > t.m_usec) return true;
            else if(m_usec == m_usec) return true;
        }
        else return false;
    }

    /// SLEEP
    static void sleep(const Time& t){
        usleep(t.total_usec());
    }

    static void sleep_until(const Time& until){
        Time tnow = now();
        assert(until > tnow);
        sleep(until - tnow);
    }

    static void context_switch(){
        usleep(0);
    }

    static Time now(){
#ifdef __WIN32__
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
        time_t t = m_sec;
        ptm = gmtime(&t);

        _out(ptm->tm_year - 70,"years ");
        _out(ptm->tm_yday,"days ");
        _out(ptm->tm_hour,"hours ");
        _out(ptm->tm_min,"mins ");
        _out(ptm->tm_sec,"s ");

        if(uint32_t ms = msec())
        s<<ms<<"ms ";
        s<<usec()<<"us";//<<u8"µs";
        return s.str();
    }

    std::string UTC(){
        std::stringstream s;

        struct tm* ptm;
        time_t t = m_sec;
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
        s<<':'<<usec();//<<u8"µs";
        return s.str();
    }

    uint32_t sec() const{  return m_sec;}
    uint32_t msec() const{ return m_usec/1000;}
    uint32_t usec() const{ return m_usec % 1000;}

    uint64_t total_sec() const{  return m_sec;}
    uint64_t total_msec() const{ return m_sec*1000 + m_usec/1000;}
    uint64_t total_usec() const{ return m_sec*1000000 + m_usec;}
private:
    friend std::ostream& operator<<(std::ostream& strm, const Time& t);

    uint32_t m_sec;
    uint32_t m_usec;
};

#define __out(X,Y); if(X) strm<<X<<Y;
inline std::ostream& operator<<(std::ostream& strm, const Time& t){

    struct tm* ptm;
    time_t tp = t.m_sec;
    ptm = gmtime(&tp);

    __out(ptm->tm_year - 70,"years ");
    __out(ptm->tm_yday,"days ");
    __out(ptm->tm_hour,"hours ");
    __out(ptm->tm_min,"mins ");
    __out(ptm->tm_sec,"s ");

    if(uint32_t ms = t.msec())
    strm<<ms<<"ms ";
    strm<<t.usec()<<"us";//<<u8"µs";
    return strm;
}

#undef _out
#undef __out

#ifdef __WIN32__
    extern Time systemboot;
#endif
#endif // _TIME_H

