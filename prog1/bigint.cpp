// $Id: bigint.cpp,v 1.2 2020-01-06 13:39:55-08 - - $

#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
using namespace std;

#include "bigint.h"
#include "debug.h"
#include "relops.h"

bigint::bigint (long that): uvalue (that), is_negative (that < 0) {
   DEBUGF ('~', this << " -> " << uvalue)
}

bigint::bigint (const ubigint& uvalue_, bool is_negative_):
                uvalue(uvalue_), is_negative(is_negative_) {
}

bigint::bigint (const string& that) {
   is_negative = that.size() > 0 and that[0] == '_';
   uvalue = ubigint (that.substr (is_negative ? 1 : 0));
}

bigint bigint::operator+ () const {
   return *this;
}

bigint bigint::operator- () const {
   return {uvalue, not is_negative};
}

bigint bigint::operator+ (const bigint& that) const {
   if (!this->is_negative and !that.is_negative) {
      bigint result;
      result.is_negative = false;
      result.uvalue = this->uvalue + that.uvalue;
      return result;
   }
   if (this->is_negative and that.is_negative) {
      bigint result;
      result.is_negative = true;
      result.uvalue = this->uvalue + that.uvalue;
      return result;
   }
   if (this->is_negative and !that.is_negative) {
      bigint result;
      if (this->uvalue < that.uvalue) {
         result.is_negative = false;
         result.uvalue = that.uvalue - this->uvalue;
      } else if (this->uvalue == that.uvalue) {
         result = bigint(0);
      } else {
         result.is_negative = true;
         result.uvalue = this->uvalue - that.uvalue;
      }
      return result;
   }
    bigint result;
    if (this->uvalue < that.uvalue) {
      result.is_negative = true;
      result.uvalue = that.uvalue - this->uvalue;
   } else if (this->uvalue == that.uvalue) {
      result = bigint(0);
   } else {
      result.is_negative = false;
      result.uvalue = this->uvalue - that.uvalue;
   }
   return result;
}

bigint bigint::operator- (const bigint& that) const {
   if (!this->is_negative and !that.is_negative) {
      bigint result;
      if (this->uvalue > that.uvalue) {
         result.uvalue = this->uvalue - that.uvalue;
         result.is_negative = false;
      } else if (this->uvalue == that.uvalue) {
         result = bigint(0);
      } else {
         result.uvalue = that.uvalue - this->uvalue;
         result.is_negative = true;
      }
      return result;
   }
   if (!this->is_negative and that.is_negative){
      bigint result;
      result.uvalue = this->uvalue + that.uvalue;
      result.is_negative = false;
      return result;
   }
   if (this->is_negative and !that.is_negative){
      bigint result;
      result.uvalue = this->uvalue + that.uvalue;
      result.is_negative = true;
      return result;
   }
   bigint result;
   if (this->uvalue > that.uvalue){
      result.uvalue = this->uvalue - that.uvalue;
      result.is_negative = true;
   } else if (this->uvalue == that.uvalue){
      result = bigint(0);
   } else {
      result.uvalue = that.uvalue - this->uvalue;
      result.is_negative = false;
   }
   return result;
}

bigint bigint::operator* (const bigint& that) const {
   bigint result;
   if (this->is_negative and that.is_negative){
      result.is_negative = false;
   } else if(!this->is_negative and !that.is_negative){
      result.is_negative = false;
   } else {
      result.is_negative = true;
   }
   result.uvalue = this->uvalue * that.uvalue;
   return result;
}

bigint bigint::operator/ (const bigint& that) const {
    bigint result;
    if (this->is_negative and that.is_negative){
      result.is_negative = false;
    } else if(!this->is_negative and !that.is_negative){
      result.is_negative = false;
    } else {
      result.is_negative = true;
    }
    result.uvalue = this->uvalue / that.uvalue;
    return result;
}

bigint bigint::operator% (const bigint& that) const {
   bigint result;
    if (this->uvalue == that.uvalue){
      result.is_negative = false;
    } else {
      result.is_negative = this->is_negative;
    }
    result.uvalue = this->uvalue % that.uvalue;
    return result;
}

bool bigint::operator== (const bigint& that) const {
   return is_negative == that.is_negative and uvalue == that.uvalue;
}

bool bigint::operator< (const bigint& that) const {
   if (is_negative != that.is_negative) return is_negative;
   return is_negative ? uvalue > that.uvalue
                      : uvalue < that.uvalue;
}

ostream& operator<< (ostream& out, const bigint& that) {
   return out << "bigint(" << (that.is_negative ? "-" : "+")
              << "," << that.uvalue << ")";
}

