/**
 * This is code is released under the
 * Apache License Version 2.0 http://www.apache.org/licenses/.
 *
 * (c) Daniel Lemire, http://lemire.me/en/
 * and Owen Kaser
 */

#ifndef UTIL
#define UTIL
#include "common.h"

#ifdef __linux__
#define USE_O_DIRECT
#endif

using namespace std;
//#define STATS
// taken from stackoverflow
#ifndef NDEBUG
#   define ASSERT(condition, message) \
    do { \
        if (! (condition)) { \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__ \
                      << " line " << __LINE__ << ": " << message << std::endl; \
            std::exit(EXIT_FAILURE); \
        } \
    } while (false)
#else
#   define ASSERT(condition, message) do { } while (false)
#endif

/**
 * Computes the greatest common divisor
 */
constexpr __attribute__ ((const))
uint32_t gcd(uint32_t x, uint32_t y) {
    return (x % y) == 0 ? y :  gcd(y,x % y);
}

template<uint32_t _x,uint32_t _y>
__attribute__ ((const))
uint32_t gcd() {
    uint32_t x = _x;
    uint32_t y = _y;
    while(true) {
        uint32_t c = x % y;
        if(c == 0 ) return y;
        x = y;
        y = c;
    }
}


__attribute__ ((const))
uint8_t * padTo32bits(uint8_t * inbyte) {
    return reinterpret_cast<uint8_t *> ((reinterpret_cast<uintptr_t> (inbyte)
            + 3) & ~3);
}

__attribute__ ((const))
const uint8_t * padTo32bits(const uint8_t * inbyte) {
    return reinterpret_cast<const uint8_t *> ((reinterpret_cast<uintptr_t> (inbyte)
            + 3) & ~3);
}

__attribute__ ((const))
uint8_t * padTo64bytes(uint8_t * inbyte) {
    return reinterpret_cast<uint8_t *> ((reinterpret_cast<uintptr_t> (inbyte)
            + 63) & ~63);
}

__attribute__ ((const))
const uint8_t * padTo64bytes(const uint8_t * inbyte) {
    return reinterpret_cast<const uint8_t *> ((reinterpret_cast<uintptr_t> (inbyte)
            + 63) & ~63);
}


template <class T>
bool  needPaddingTo64bytes(T * inbyte) {
    return reinterpret_cast<uintptr_t> (inbyte) & 63;
}

__attribute__ ((const))
bool needPaddingTo32Bits(uint8_t * inbyte) {
    return reinterpret_cast<uintptr_t> (inbyte) & 3;
}

__attribute__ ((const))
bool needPaddingTo32Bits(const uint8_t * inbyte) {
    return reinterpret_cast<uintptr_t> (inbyte) & 3;
}

__attribute__ ((const))
constexpr uint32_t gccbits(const uint32_t v) {
    return v == 0 ? 0 : 32 - __builtin_clz(v);
}

__attribute__ ((const))
bool divisibleby(size_t a, uint32_t x) {
    return (a % x == 0);
}

/**
 * compute the deltas, you do not want to use this
 * function if speed matters. This is only for convenience.
 */
template <class container>
container diffs(const container & in, const bool aredistinct) {
    container out;
    if (in.empty())
        return out;
    out.resize(in.size()-1);
    for (size_t k = 0; k < in.size() - 1; ++k)
        if (aredistinct)
            out.push_back(in[k + 1] - in[k] - 1);
        else
            out.push_back(in[k + 1] - in[k]);
    return out;
}

void checkifdivisibleby(size_t a, uint32_t x) {
    if (!divisibleby(a, x)) {
        ostringstream convert;
        convert << a << " not divisible by " << x;
        throw logic_error(convert.str());
    }
}


template<class iter>
void printme(iter i, iter b) {
    for (iter j = i; j != b; ++j)
        cout << *j << " ";
    cout << endl;
}


__attribute__ ((const))
uint32_t asmbits(const uint32_t v) {
    if (v == 0)
        return 0;
    uint32_t answer;
    __asm__("bsr %1, %0;" :"=r"(answer) :"r"(v));
    return answer + 1;
}

__attribute__ ((const))
uint32_t slowbits(uint32_t v) {
    uint32_t r = 0;
    while (v) {
        r++;
        v = v >> 1;
    }
    return r;
}

__attribute__ ((const))
uint32_t bits(uint32_t v) {
    uint32_t r(0);
    if (v >= (1U << 15)) {
        v >>= 16;
        r += 16;
    }
    if (v >= (1U << 7)) {
        v >>= 8;
        r += 8;
    }
    if (v >= (1U << 3)) {
        v >>= 4;
        r += 4;
    }
    if (v >= (1U << 1)) {
        v >>= 2;
        r += 2;
    }
    if (v >= (1U << 0)) {
        v >>= 1;
        r += 1;
    }
    return r;
}

constexpr uint32_t div_roundup(uint32_t v, uint32_t divisor) {
    return (v + (divisor - 1)) / divisor;
}

template<class iterator>
__attribute__ ((pure))
uint32_t maxbits(const iterator & begin, const iterator & end) {
    uint32_t accumulator = 0;
    for (iterator k = begin; k != end; ++k) {
        accumulator |= *k;
    }
    return gccbits(accumulator);
}



template<class iterator>
uint32_t slowmaxbits(const iterator & begin, const iterator & end) {
    uint32_t accumulator = 0;
    for (iterator k = begin; k != end; ++k) {
        const uint32_t tb = gccbits(*k);
        if (tb > accumulator)
            accumulator = tb;
    }
    return accumulator;
}

//basically, we can sometimes memoize the maxbits computation
// Since the first scan looks at b input words, the second looks
// at b/2, the third looks at b/3... (total related to harmonic numbers)
// it is probably only worthwhile to memoize the first maybe 20% prefix
// (rest can be "naively"  re-scanned if needed)
// also, a useful heuristic should be to start with however many
// bits are required for the first number in the sequence.  Or OR
// the first two or three values together (danger, what if you OR
// more than you'd actually use?)
// alternative heuristic is to start with however many bits you used for the
// last encoding.  See if it works.  Yes: start sequential scan downward.  No:
// start sequential scan upward.
// To be tried...

//template<class t>
//struct bitwise_or : public binary_function<t, t, t> {
  //t operator()(t x, t y) { return x|y; }
//};

// 
template<int b, class t, class iterator>
int greedy_bit_size_lookahead( const iterator &begin, 
       const iterator &end) {
  //  assert(end- begin <= b);
  vector<t> prefixOrBuffer(end-begin);  // consider a  preallocated buffer...
  
  partial_sum(begin, end, prefixOrBuffer.begin(),
	      [](t x, t y) { return x | y; }  // change dl's + to |
          //bitwise_or<t>()
          );
  // do the bitwise or-ing once only.

  // cout << " prefixOrBuffer is ";
  // for (int i=0; i < prefixOrBuffer.size(); ++i) cout << prefixOrBuffer[i] << " "                                                  ;
  //  cout << endl;
  
  if (end-begin == b) { // expected case, to help out compiler.  Should be unrolled
    for (int i=1; i < 31; ++i)
      if (prefixOrBuffer[ b/i -1] < (static_cast<t>(1)<<i)) return i;
    //assert(false); // cannot get here unless 32+ bits required
    return -1;
  }
  else { // general case, maybe less data than we could pack with 1-bit fields
    for (int i=1; i < 31; ++i) {
      uint64_t indexToCheck = b/i - 1;
      if (indexToCheck >= prefixOrBuffer.size())  
        indexToCheck = prefixOrBuffer.size()-1;

      if (prefixOrBuffer[indexToCheck] < (static_cast<t>(1)<<i)) return i;
    }
    // assert(false); 
    return -1;
  }
}


// assume the previous bit size is close to the required bit size
template<int b, class t, class iterator>
int greedy_bit_size_lookahead( const iterator &begin, 
			       const iterator &end, uint32_t previous_size) {
  
  uint32_t  span_length = end-begin;
  if (span_length == b) {  // work on the specialization later...
    // try previous size
    if (maxbits(begin, begin+(b/previous_size)) > previous_size) {
      // previous_size is too small; go until you find something bigger that works
      for (uint32_t i=previous_size+1; i < previous_size+32 /* was nothing */ ; ++i)  // upper bound is only to encourage compiler to unroll
	if (maxbits(begin, begin+(b/i)) <= i) return i;
      return -1; // impossible 
    }
    else { // previous_size works, but perhaps we can find something smaller that also works
      uint32_t i;
      for (i=previous_size-1; i /* > 0 */ != previous_size-32; --i) {
        if (i == 0) break;  // This funkiness is to encourage unrolling.
	if (maxbits(begin, begin+(b/i)) > i) break;
      }
      return i+1;  //either i=0 and we return 1....or i is the first too-small size
    }
  }
  else {
    // same thing with careful checks to avoid reading past end of buffer
    uint32_t endIdx = b/previous_size;
    if (endIdx >= span_length) endIdx=span_length;
    
    if (maxbits(begin, begin+endIdx) > previous_size) {
      for (uint32_t i=previous_size+1; ; ++i) {
	endIdx = b/i;
	if (endIdx >= span_length) endIdx=span_length;
	if (maxbits(begin, begin+endIdx) <= i) return i;
      } 
      return -1; // impossible
    }
    else { 
      uint32_t i;
      for (i=previous_size-1; i > 0; --i) {
	endIdx = b/i;
	if (endIdx >= span_length) endIdx=span_length;
	if (maxbits(begin, begin+endIdx) > i) break;
      }
      return i+1;  
    }
  }
}    






class BitWidthHistoGram {
public:
    vector < double > histo;
    BitWidthHistoGram():histo(33, 0) {}

    void display(string prefix ="") {
        double sum = 0;
        for (size_t k = 0; k < histo.size(); ++k)
            sum+= histo[k];
        if(sum==0) return ;
        for (size_t k = 0; k < histo.size(); ++k) {
                cout<<prefix << k << " " << histo[k] / sum << endl;
        }
    }
    template <class container>
    void eatIntegers(const container & rawdata) {
        for (uint32_t i = 0; i < rawdata.size() ; ++i) {
            histo[asmbits(rawdata[i])] += 1;
        }

    }

    template <class container>
    void eatDGaps(const container & rawdata) {
        if (rawdata.size() <= 1)
            return;
        for (uint32_t i = 0; i < rawdata.size() - 1; ++i) {
            assert(rawdata[i + 1] > rawdata[i]);
            uint32_t gap = rawdata[i + 1] - rawdata[i] - 1;
            assert(gap<rawdata[i + 1] );
            histo[asmbits(gap)] += 1;
        }

    }

};


#endif