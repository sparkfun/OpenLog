/*
 * Copyright © Micromega Corporation 2002-2003. All rights reserved
 */

package stamp.math;
import  stamp.core.*;

/**
 * This class provides support for 32-bit integers.
 * <p>
 * Support is provided for both signed and unsigned numbers. The range of
 * values that can be stored is as follows:
 * <pre>
 * 32-bit signed number:   -2,147,483,648 to 2,147,483,647
 * 32-bit unsigned number:  0 to 4,294,967,295</pre>
 * <p>
 * The 32-bit integer is stored as two 16-bit fields which can be accessed
 * directly.
 * <br>Methods are provided to support the following operations:
 * <ul>
 * <li>create 32-bit integer objects</li>
 * <li>set the value</li>
 * <li>convert a String or StringBuffer to a 32-bit value</li>
 * <li>add, subtract</li>
 * <li>multiply, divide (signed and unsigned), remainder</li>
 * <li>shift left, shift right</li>
 * <li>absolute value, negate</li>
 * <li>compare (signed and unsigned), equality</li>
 * <li>convert 32-bit integer to string (signed and unsigned)</li>
 * </ul>
 *
 * @author Cam Thompson, Micromega Corporation
 * @version 1.0 - February 10, 2003 (AppNote011)
 */

public class Int32 {

  /**
   * high 16 bits of 32-bit integer.
   */
  public int high;

  /**
   * low 16 bits of 32-bit integer.
   */
  public int low;

  //-------------------- constructors -----------------------------------------

  /**
   * Create a new 32-bit integer initialized to zero.
   */
  public Int32() {
    high = 0;
    low = 0;
  }

  /**
   * Create a new 32-bit integer initialized to value of another 32-bit integer.
   * @param num 32-bit integer
   */
  public Int32(Int32 num) {
    set(num);
  }

  /**
   * Create a new 32-bit integer initialized to the 16-bit value.
   * @param low low 16-bits of initial value
   * (sign extends to high 16-bits)
   */
  public Int32(int low) {
    set(low);
  }

  /**
   * Create a new 32-bit integer initialized to 32-bit value.
   * @param high high 16-bits of initial value
   * @param low low 16-bits of initial value
   */
  public Int32(int high, int low) {
    set(high, low);
  }

  //-------------------- set value --------------------------------------------

  /**
   * Sets the 32-bit integer equal to the value of another 32-bit integer.
   * @param num 32-bit integer
   */
  public void set(Int32 num) {
    set(num.high, num.low);
  }

  /**
   * Sets the 32-bit integer to a 16-bit value (sign extended).
   * @param low low 16-bits of value
   * (sign extends to high 16-bits)
   */
  public void set(int low) {
    set (((low < 0) ? -1: 0), low);
  }

  /**
   * Sets the 32-bit integer to a 32-bit value.
   * @param high high 16-bits of value
   * @param low low 16-bits of value
   */
  public void set(int high, int low) {
    this.high = high;
    this.low = low;
  } // end set

  //-------------------- set value (from String) ------------------------------

  /**
   * Sets the 32-bit integer to the converted value of the String.
   * The conversion skips leading whitespace and stops at first non-decimal
   * character.
   * @param s String containing 32-bit integer
   */
  public void set(String s) {
    int   i;

    // set the 32-bit value to zero
    high = 0;
    low = 0;

    // get the length of the string
    int len = s.length();
    char ch = ' ';
    boolean sign = false;

    // skip whitespace
    for (i = 0; i < len; i++) {
      ch = s.charAt(i);
      if ((ch != ' ') && (ch != '\t') && (ch != '\n')) break;
    }

    // check for sign
    if (ch == '-') {
      sign = true;
      i++;
    }

    // convert the number
    while (i < len) {
      ch = s.charAt(i++);
      if (ch < '0' || ch > '9') break;
      this.multiply(10);
      this.add(ch - '0');
    }

    // check for sign and negate
    if (sign) this.negate();

  } // end set(String)

  //-------------------- set value (from StringBuffer) ------------------------

  /**
   * Sets the 32-bit integer to the converted value of the StringBuffer.
   * The conversion skips leading whitespace and stops at first non-decimal
   * character.
   * @param sb StringBuffer containing 32-bit integer
   */
  public void set(StringBuffer sb) {
    set(sb.toString());
  } //end set(StringBuffer)

  //-------------------- add routine ------------------------------------------

  /**
   * Adds another 32-bit integer to the 32-bit integer.
   * @param num 32-bit integer
   */
  public void add(Int32 num) {
    add(num.high, num.low);
  }

  /**
   * Adds a 16-bit value (sign extended) to the 32-bit integer.
   * @param low 16-bit value
   * (sign extends to high 16-bits)
   */
  public void add (int low) {
    add (((low < 0) ? -1: 0), low);
  }

  /**
   * Adds a 32-bit value to the 32-bit integer.
   * @param high high 16-bits of value
   * @param low low 16-bits of value
   */
  public void add(int high, int low) {
    this.low = this.low + low;
    if (CPU.carry()) this.high++;
    this.high = this.high + high;
  } // end add

  //-------------------- subtract routine ------------------------------------

  /**
   * Subtracts another 32-bit integer from the 32-bit integer.
   * @param num 32-bit integer.
   */
  public void subtract(Int32 num) {
    subtract(num.high, num.low);
  }

  /**
   * Subtracts a 16-bit value (sign extended) from the 32-bit integer.
   * @param low 16-bit value
   * (sign extends to high 16-bits)
   */
  public void subtract(int low) {
    subtract(((low < 0) ? -1: 0), low);
  }

  /**
   * Subtracts a 32-bit value from the 32-bit integer.
   * @param high high 16-bits of value
   * @param low low 16-bits of value
   */
  public void subtract(int high, int low) {
    this.high = this.high - high;
    if (low != 0) {
      this.low = this.low - low;
      if (!CPU.carry()) this.high--;
    }
  } // end subtract

  //-------------------- divide routine ---------------------------------------

  /**
   * Divides the 32-bit integer by another 32-bit integer.
   * @param num 32-bit integer
   */
  public void divide(Int32 num) {
    divide(num.high, num.low);
  }

  /**
   * Divides the 32-bit integer by a 16-bit value.
   * @param low 16-bit value
   * (sign extends to high 16-bits)
   */
  public void divide(int low) {
    divide(((low < 0) ? -1: 0), low);
  }

  /**
   * Divides the 32-bit integer by a 32-bit value.
   * @param high high 16-bits of value
   * @param low low 16-bits of value
   */
  public void divide(int high, int low) {
    boolean sign1 = false;
    boolean sign2 = false;
    int     tmpLow;

    // check if dividend is negative
    if (this.high < 0) {
      sign1 = true;
      this.negate();
    }

    // check if divisor is negative
    if (high < 0) {
      sign2 = true;
      high = ~high;
      low = (~low) + 1;
      if (low == 0) high++;
    }

    // perform unsigned divide
    udivide(high, low);

    // set the sign of the quotient
    if (sign1 != sign2) this.negate();

    // set the sign of the remainder
    if (sign1) {
      remHigh = ~remHigh;
      remLow = (~remLow) + 1;
      if (remLow == 0) remHigh++;
    }
  } // end divide

  //-------------------- unsigned divide routine ------------------------------

  /**
   * Divides the 32-bit unsigned integer by another 32-bit unsigned integer.
   * Note: the divisor is restricted to 31 bits to allow for an optimization
   * in the divide routine.
   * @param num 32-bit integer
   */
  public void udivide(Int32 num) {
    udivide(num.high, num.low);
  }

  /**
   * Divides the 32-bit integer by a 16-bit value (unsigned divide).
   * @param low 16-bit value
   * (high 16-bits set to 0)
   */
  public void udivide(int low) {
    udivide(0, low);
  }

  /**
   * Divides the 32-bit unsigned integer by a 32-bit unsigned value.
   * Note: the divisor is restricted to 31 bits to allow for an optimization
   * in the divide routine.
   * @param high high 16-bits of value
   * @param low low 16-bits of value
   */
  public void udivide(int high, int low) {

    // clear remainder
    remHigh = 0;
    remLow = 0;

    // check for zero divisor (quotient = maximum positive, remainder = 0)
    if (high == 0 && low == 0) {
      this.high = 0x7FFF;
      this.low = (short) 0xFFFF;
      return;
    }

    // compare divisor and dividend (32-bit unsigned compare)
    int carry = 0;
    int tmpHigh = ~high;
    int tmpLow = -low;
    if (tmpLow == 0) tmpHigh++;
    tmpLow = tmpLow + this.low;
    if (CPU.carry()) {
      tmpHigh++;
      if (tmpHigh == 0) carry = 1;
    }
    tmpHigh = tmpHigh + this.high;
    if (CPU.carry()) carry = 1;

    // if divisor > dividend (quotient = 0, remainder = dividend)
    if (carry == 0) {
      remHigh = this.high;
      remLow = this.low;
      this.high = 0;
      this.low = 0;
      return;
    }

    // if divisor == dividend (quotient = 1, remainder = 0)
    if ((tmpHigh == 0) && (tmpLow == 0)) {
      this.high = 0;
      this.low = 1;
      return;
    }

    // check if compatible with 16-bit internal divide
    if ((this.high == 0) && (high == 0) && (this.low > 0) && (low > 0)) {
      this.high = 0;
      remLow = this.low % low;
      this.low = this.low / low;
      return;
    }

    // left justify the dividend
    int bitCount = 32;
    if (this.high == 0) {
      this.high = this.low;
      bitCount = 16;
    }

    while (this.high > 0) {
      this.high = (this.high << 1) + (this.low >>> 15);
      this.low = this.low << 1;
      bitCount--;
    }

    // clear the quotient
    int quotHigh = 0;
    int quotLow = 0;

    //  process remaining bits in dividend
    for (int i = 0; i < bitCount; i++) {

      // rotate dividend left
      carry = this.high >>> 15;
      this.high = (this.high << 1) + (this.low >>> 15);
      this.low = this.low << 1;

      // rotate remainder left (with carry from dividend)
      remHigh = (remHigh << 1) + (remLow >>> 15);
      remLow = (remLow << 1) + carry;

      // subtract divisor from remainder
      tmpHigh = remHigh - high;
      tmpLow = remLow - low;
      if (!CPU.carry()) tmpHigh--;

      // if divisor < remainder update remainder
      // rotate bit into quotient
      quotHigh = (quotHigh << 1) + (quotLow >>> 15);
      if (tmpHigh >= 0) {
        remHigh = tmpHigh;
        remLow = tmpLow;
        quotLow = (quotLow << 1) + 1;
      }
      else {
        quotLow = (quotLow << 1);
      }
    } // end for

    // set 32-bit integer to quotient
    this.high = quotHigh;
    this.low = quotLow;

  } // end udivide

  /**
   * Sets another 32-bit integer to the remainder of the last divide or udivide.
   * @param num 32-bit integer will be set to remainder
   */
  public void remainder(Int32 num)
  {
    num.set(remHigh, remLow);
  }

  //-------------------- multiply routine -------------------------------------

  /**
   * Multiplies the 32-bit integer by another 32-bit integer.
   * @param num 32-bit integer
   */
  public void multiply(Int32 num) {
    multiply(num.high, num.low);
  }

  /**
   * Multiplies the 32-bit integer by a 16-bit value.
   * @param low 16-bit value
   * (sign extends to high 16-bits)
   */
  public void multiply(int low) {
    multiply(((low < 0) ? -1: 0), low);
  }

  /**
   * Multiplies the 32-bit integer by a 32-bit value.
   * @param high high 16-bits of value
   * @param low low 16-bits of value
   */
  public void multiply(int high, int low) {

    // check for multiplicand of zero
    if (this.high == 0) {
      if (this.low == 0) {
        return;
      }
      // check for multiplicand of one
      else if (this.low == 1) {
        this.high = high;
        this.low = low;
        return;
      }
    }

    // get multiplicand and clear product
    int multHigh = this.high;
    int multLow = this.low;
    this.high = 0;
    this.low = 0;
    int carry = 0;

    for (int i = 0; i < 32; i++) {
      // check for zero multiplier
      if ((high == 0) && (low == 0)) return;

      // shift multiplier right and conditionally add multiplicand
      carry = low & 1;
      low = (low >>> 1) + (high << 15);
      high = high >>> 1;
      if (carry != 0) {
        this.low = this.low + multLow;
        if (CPU.carry()) this.high++;
        this.high = this.high + multHigh;
      }

      // shift multiplicand left
      multHigh = (multHigh << 1) + (multLow >>> 15);
      multLow = multLow << 1;
    } // end for
  } // end multiply

  //-------------------- shift routines ---------------------------------------

  /**
   * Shifts the 32-bit integer right (0 enters MSB).
   * @param count number of bit positions to shift
   */
  public void shiftRight(int count) {

    // check for special cases
    if (count <= 0) {
      return;
    }
    else if (count > 32) {
      high = 0;
      low = 0;
      return;
    }
    else if (count > 16) {
      low = high;
      high = 0;
      count -= 16;
    }

    // shift remaining bits
    int cnt2 = 16 - count;
    low = (low >>> count) +
          ((high & ((short)0xFFFF >>> (16 - count))) << (16 - count));
    high = high >>> count;

  } // end shiftRight

  /**
   * Shifts the 32-bit integer left (0 enters LSB).
   * @param count number of bit positions to shift
   */
  public void shiftLeft(int count) {
    // check for special cases
    if (count <= 0) {
      return;
    }
    else if (count > 32) {
      high = 0;
      low = 0;
      return;
    }
    else if (count > 16) {
      high = low;
      low = 0;
      count -= 16;
    }

    // shift remaining bits
    high = (high << count) +
           ((low & (~((short)0xFFFF >>> count))) >>> (16 - count));
    low = low << count;

  } // end shiftLeft

  //-------------------- signed compare routines ------------------------------

  /**
   * Performs a signed comparison of the 32-bit integer to
   * another 32-bit integer.
   * @param num 32-bit integer
   * @return -1 if the 32-bit integer is less than another 32-bit integer
   * <br>&nbsp 0 if the 32-bit integer equals another 32-bit integer
   * <br>&nbsp 1 if the 32-bit integer is greater than another 32-bit integer
   */
  public int compare(Int32 num) {
    return(compare(num.high, num.low));
  }

  /**
   * Performs a signed comparison of the 32-bit integer to a 16-bit integer.
   * @param low 16-bit value (sign extends to high 16-bits)
   * @return -1 if the 32-bit integer is less than 16-bit value
   * <br>&nbsp 0 if the 32-bit integer equals the 16-bit value
   * <br>&nbsp 1 if the 32-bit integer is greater than 16-bit value
   */
  public int compare (int low) {
    return(compare (((low < 0) ? -1: 0), low));
  }

  /**
   * Performs a signed comparison of the 32-bit integer to a 32-bit value.
   * @param high high 16-bits of value
   * @param low low 16-bits of value
   * @return -1 if the 32-bit integer is less than 32-bit value
   * <br>&nbsp 0 if the 32-bit integer equals 32-bit value
   * <br>&nbsp 1 if the 32-bit integer is greater than 32-bit value
   */
  public int compare(int high, int low) {

    // check for equal values
    if ((this.high == high) && (this.low == low)) return(0);

    // compare the two values (32-bit signed compare)
    high = this.high - high;
    if (low != 0) {
      low = this.low - low;
      if (!CPU.carry()) high--;
    }
    return((high < 0) ? -1 : 1);

  } // end compare

  //-------------------- unsigned compare routines ----------------------------

  /**
   * Performs an unsigned comparison of the 32-bit integer to
   * another 32-bit integer.
   * @param num 32-bit integer
   * @return -1 if the 32-bit integer is less than another 32-bit integer
   * <br>&nbsp 0 if the 32-bit integer equals another 32-bit integer
   * <br>&nbsp 1 if the 32-bit integer is greater than another 32-bit integer
   */
  public int ucompare(Int32 num) {
    return(ucompare(num.high, num.low));
  }

  /**
   * Performs an unsigned comparison of the 32-bit integer to a 16-bit integer.
   * @param low 16-bit value (high 16-bits set to zero)
   * @return -1 if the 32-bit integer is less than 16-bit value
   * <br>&nbsp 0 if the 32-bit integer equals the 16-bit value
   * <br>&nbsp 1 if the 32-bit integer is greater than 16-bit value
   */
  public int ucompare (int low) {
    return(ucompare (0, low));
  }

  /**
   * Performs an unsigned comparison of the 32-bit integer to
   * another 32-bit integer.
   * @param high high 16-bits of value
   * @param low low 16-bits of value
   * @return -1 if the 32-bit integer is less than 32-bit value
   * <br>&nbsp 0 if the 32-bit integer equals 32-bit value
   * <br>&nbsp 1 if the 32-bit integer is greater than 32-bit value
   */
  public int ucompare(int high, int low) {

    // check for equal values
    if ((this.high == high) && (this.low == low)) return(0);

    // check for comparison value of zero
    if (high == 0 && low == 0)
      return((this.high == 0 && this.low == 0) ? 0 : 1);

    // compare the values (32-bit unsigned compare)
    high = ~high;
    low = -low;
    if (low == 0) high++;

    low = low + this.low;
    if (CPU.carry()) {
      high++;
      if (high == 0) return(1);
    }
    high = high + this.high;
    if (CPU.carry()) return(1);
    return(-1);
  } // end ucompare

  //-------------------- equals -----------------------------------------------

  /**
   * Checks if 32-bit integer is equal to another 32-bit integer.
   * @param num 32-bit integer
   * @return <code>true</code> if equal
   */
  public boolean equals(Int32 num) {
    if ((high == num.high) && (low == num.low))
      return(true);
    else
      return (false);
  } // end equals

  //-------------------- absolute value / negate ------------------------------

  /**
   * Sets the 32-bit integer to its absolute value.
   */
  public void abs() {
    if (high < 0) negate();
  } // end abs

  /**
   * Negates the 32-bit integer.
   */
  public void negate() {
    high = ~high;
    low = -low;
    if (low == 0) high++;
  } // end negate

  //-------------------- convert to string ------------------------------------

  /**
   * Convert the signed 32-bit integer to a String.
   */
  public String toString() {
    return(convertString(true));
  }

  /**
   * Convert the unsigned 32-bit integer to a String.
   */
  public String utoString() {
    return(convertString(false));
  }

//============================================================================
// Methods and fields below this point are private.
//============================================================================

  // remainder from last divide
  private static int  remHigh, remLow;

  // buffer for toString conversion
  private static StringBuffer sbuf = new StringBuffer(11);

  // divisors for string conversion
  private final static int [] divHigh  =
    { 0x3B9A, 0x05F5, 0x0098, 0x000F, 1, 0, 0, 0, 0 };
  private final static int [] divLow   =
    { (short)0xCA00, (short)0xE100, (short)0x9680,
      (short)0x4240, (short)0x86A0, 10000, 1000, 100, 10 };

  private String convertString(boolean signed) {

    // initialize string buffer
    sbuf.clear();

    // save this 32-bit value and remainder
    int tmpHigh = high;
    int tmpLow  = low;
    int tmpRemHigh = remHigh;
    int tmpRemLow  = remLow;

    //check for signed conversion and negative number
    if (signed && high < 0) {
      sbuf.append('-');
      this.negate();
    }

    // convert to string
    boolean zeroFlag = false;
    for (int i = 0; i < 9; i++) {
      this.udivide(divHigh[i], divLow[i]);
      if (zeroFlag || low > 0) {
        sbuf.append((char) ('0' + low));
        zeroFlag = true;
      }
      high = remHigh;
      low = remLow;
    }
    sbuf.append((char) ('0' + low));

    // restore the Int32 value and remainder
    high = tmpHigh;
    low = tmpLow;
    remHigh = tmpRemHigh;
    remLow = tmpRemLow;

    // return string
    return (sbuf.toString());

  } // end convertString
} // end Int32 class