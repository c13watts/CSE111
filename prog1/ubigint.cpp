// $Id: ubigint.cpp,v 1.8 2020-01-06 13:39:55-08 - - $

#include <cctype>
#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
#include <algorithm>
using namespace std;

#include "ubigint.h"
#include "debug.h"

ubigint::ubigint (unsigned long that): ubig_value (that) {
   if (that == 0) {
      ubig_value.push_back(0);
   } else {
      while (that > 0) {
         ubig_value.push_back((that % 10));
         that /= 10;
      }
   }
}

ubigint::ubigint (const string& that): ubig_value(0) {
   DEBUGF ('~', "that = \"" << that << "\"");
   for (char digit: that) {
      if (not isdigit (digit)) {
         throw invalid_argument ("ubigint::ubigint(" + that + ")");
      }
      udigit_t val = digit - '0';
      ubig_value.push_back(val);
   }
   std::reverse(ubig_value.begin(), ubig_value.end());
}

ubigint ubigint::operator+ (const ubigint& that) const {
   ubigint fNumber = *this;
   ubigint sNumber = that;
   if (this->ubig_value.size() < that.ubig_value.size()) {
      sNumber = *this;
      fNumber = that;
   }
   udigit_t a = 0;
   uint b = 0;
   ubigint result;
   for (udigit_t digit:fNumber.ubig_value) {
      udigit_t sumValue;
      if (b >= sNumber.ubig_value.size()) {
         sumValue = 0;
      } else {
         sumValue = sNumber.ubig_value[b];
      }
      udigit_t val = sumValue + digit + a;
      a = val / 10;
      val %= 10;
      result.ubig_value.push_back(val);
      ++b;
   }
   if (a > 0) {
      result.ubig_value.push_back(a);
   }
   while (result.ubig_value.size() > 1 
   and result.ubig_value.back() == 0) {
      result.ubig_value.pop_back();
   }
   return result;
}

ubigint ubigint::operator- (const ubigint& that) const {
   if (*this < that) throw domain_error ("ubigint::operator-(a<b)");
   ubigint result;
   ubigint fNumber = *this;
   ubigint sNumber = that;

   udigit_t a = 0;
   uint b = 0;
   for (udigit_t digit:fNumber.ubig_value) {
      udigit_t diffValue;
      if (b >= sNumber.ubig_value.size()) {
         diffValue = 0;
      } else {
         diffValue = sNumber.ubig_value[b];
      }
        udigit_t val;
        if (digit < (diffValue + a)) {
            val = (digit + 10) - a - diffValue;
            a = 1;
        } else {
            val = digit - diffValue - a;
            a = 0;
        }
      ++b;
      result.ubig_value.push_back(val);
   }
   while (result.ubig_value.size() > 1 
   and result.ubig_value.back() == 0) {
      result.ubig_value.pop_back();
   }
   return result;
}

ubigint ubigint::operator* (const ubigint& that) const {
   ubigint result = ubigint(0);
   ubigint fNumber = *this;
   ubigint sNumber = that;

   if (this->ubig_value.size() < that.ubig_value.size()) {
      sNumber = *this;
      fNumber = that;
   }
   uint place = 0;
   for (udigit_t digitTwo:sNumber.ubig_value) {
      ubigint cp;
      for (uint i = 0; i < place; ++i) {
         cp.ubig_value.push_back(0);
      }
      uint a = 0;
      for (udigit_t digitOne:fNumber.ubig_value) {
         udigit_t product = digitOne * digitTwo;
         udigit_t val = product % 10;
         udigit_t placeVal = val + a;
         a = (product / 10);
         if (placeVal > 10) {
            udigit_t newPlaceVal = placeVal % 10;
            a = a + placeVal / 10;
            cp.ubig_value.push_back(newPlaceVal);
         } else {
            cp.ubig_value.push_back(placeVal);
         }
      }
      if (a > 0) {
         cp.ubig_value.push_back(a);
      }
      result = result + cp;
      ++place;
   }
   return result;
}

void ubigint::multiply_by_2() {
   ubigint multTwo = ubigint(2);
   ubigint result = multTwo * *this;
   this->ubig_value = result.ubig_value;
}

void ubigint::divide_by_2() {
   ubigvalue_t result;
   auto rit = this->ubig_value.rbegin();
   udigit_t rem = 0;
   for (; rit != this->ubig_value.rend(); ++rit) {
      udigit_t quo = ((rem * 10) + *rit) / 2;
      rem = ((rem * 10) + *rit) % 2;
      result.push_back(quo);
   }
   std::reverse(result.begin(), result.end());
   while (result.size() > 1 and result.back() == 0) {
      result.pop_back();
   }
   this->ubig_value = result;
}

struct quo_rem { ubigint quotient; ubigint remainder; };
quo_rem udivide (const ubigint& dividend, const ubigint& divisor_) {
   // NOTE: udivide is a non-member function.
   ubigint divisor {divisor_};
   ubigint zero {0};
   if (divisor == zero) throw domain_error ("udivide by zero");
   ubigint power_of_2 {1};
   ubigint quotient {0};
   ubigint remainder {dividend}; // left operand, dividend
   while (divisor < remainder) {
      divisor.multiply_by_2();
      power_of_2.multiply_by_2();
   }
   while (power_of_2 > zero) {
      if (divisor <= remainder) {
         remainder = remainder - divisor;
         quotient = quotient + power_of_2;
      }
      divisor.divide_by_2();
      power_of_2.divide_by_2();
   }
   return {.quotient = quotient, .remainder = remainder};
}

ubigint ubigint::operator/ (const ubigint& that) const {
   return udivide (*this, that).quotient;
}

ubigint ubigint::operator% (const ubigint& that) const {
   return udivide (*this, that).remainder;
}

bool ubigint::operator== (const ubigint& that) const {
   if (this->ubig_value.size() != that.ubig_value.size()) {
      return false;
   }
   for (uint i = 0; i < this->ubig_value.size(); ++i) {
      if (this->ubig_value[i] != that.ubig_value[i]) {
         return false;
      }
   }
   return true;
}

bool ubigint::operator< (const ubigint& that) const {
   if (this->ubig_value.size() < that.ubig_value.size()) {
      return true;
   }
   if (this->ubig_value.size() > that.ubig_value.size()) {
      return false;
   }
   for (int i = this->ubig_value.size() - 1; i >= 0; --i) {
      if (this->ubig_value[i] < that.ubig_value[i]) {
         return true;
         }
      if (this->ubig_value[i] > that.ubig_value[i]) {
         return false;
      }
   }
   return false;
}

ostream& operator<< (ostream& out, const ubigint& that) { 
   std::string s;
   auto rit = that.ubig_value.rbegin();
   int b = 0;
   for (; rit != that.ubig_value.rend(); ++rit) {
      if (b == 69) {
         s += "\\\n";
         b = 0;
      }
      s += std::to_string(*rit);
      ++b;
   }
   return out << s;
}

