#pragma once

#include <chrono>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <sstream>
#include <iomanip>
#include <cmath>
#include "models.h"

namespace util
{
  namespace chrono = std::chrono;

  using Clock = chrono::system_clock;
  using TimePoint = chrono::time_point<Clock>;

  inline TimePoint current_time()
  {
    return Clock::now();
  }

  inline chrono::milliseconds get_milli_seconds_timestamp(TimePoint time)
  {
    return chrono::duration_cast<chrono::milliseconds>(time.time_since_epoch());
  }

  inline chrono::seconds get_seconds_timestamp(TimePoint time)
  {
    return chrono::duration_cast<chrono::seconds>(time.time_since_epoch());
  }

  inline TimePoint hr_time()
  {
    return chrono::high_resolution_clock::now();
  }

  inline long time_diff_ms(TimePoint start, TimePoint end)
  {
    return chrono::duration_cast<chrono::milliseconds>(end - start).count();
  }

  inline long time_diff_ns(TimePoint start, TimePoint end)
  {
    return chrono::duration_cast<chrono::nanoseconds>(end - start).count();
  }

  inline double round_up(double x, double min_increment)
  {
    double remainder = std::fmod(x, min_increment);
    if (remainder == 0)
      return x;
    else
      return x + min_increment - remainder;
  }

  inline double round_down(double x, double min_increment)
  {
    double remainder = std::fmod(x, min_increment);
    if (remainder == 0)
      return x;
    else
      return std::floor(x / min_increment) * min_increment;
  }

  inline double round_nearest(double x, double min_increment)
  {
    double up = round_up(x, min_increment);
    double down = round_down(x, min_increment);
    return (std::fabs(x - down) > std::fabs(up - x)) ? up : down;
  }

  inline double round_side(double x, double min_increment, Models::Side side)
  {
    switch (side)
    {
    case Models::Side::Bid:
      return round_down(x, min_increment);
    case Models::Side::Ask:
      return round_up(x, min_increment);
    default:
      return round_nearest(x, min_increment);
    }
  }

  namespace encoding
  {
    inline std::string hmac(const std::string &secret, const std::string &msg)
    {
      HMAC_CTX *ctx = HMAC_CTX_new();
      unsigned char signed_msg[EVP_MAX_MD_SIZE];

      HMAC_Init_ex(ctx, secret.c_str(), (int)secret.length(), EVP_sha256(), nullptr);
      HMAC_Update(ctx, (unsigned char *)msg.c_str(), msg.length());
      unsigned int out_len;
      HMAC_Final(ctx, signed_msg, &out_len);
      HMAC_CTX_free(ctx);

      std::stringstream ss;
      for (unsigned int i = 0; i < out_len; i++)
      {
        ss << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(signed_msg[i]);
      }

      return (ss.str());
    }
  } // namespace encoding
} // namespace util