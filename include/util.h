#pragma once

#include <chrono>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <string>

namespace util
{
  namespace chrono = std::chrono;

  using Clock = chrono::system_clock;
  using TimePoint = chrono::time_point<Clock>;

  inline TimePoint current_time()
  {
    return Clock::now();
  }

  inline chrono::milliseconds get_ms_timestamp(TimePoint time)
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

  inline long time_diff_ns(TimePoint start, TimePoint end)
  {
    return chrono::duration_cast<chrono::nanoseconds>(end - start).count();
  }

  namespace encoding
  {

    namespace
    {
      struct HmacCtx
      {
        HMAC_CTX ctx;
        HmacCtx() { HMAC_CTX_init(&ctx); }
        ~HmacCtx() { HMAC_CTX_cleanup(&ctx); }
      };
    } // namespace

    inline std::string hmac(const std::string &secret,
                     std::string msg,
                     std::size_t signed_len)
    {
      static HmacCtx hmac;
      char signed_msg[64];

      HMAC_Init_ex(
          &hmac.ctx, secret.data(), (int)secret.size(), EVP_sha256(), nullptr);
      HMAC_Update(&hmac.ctx, (unsigned char *)msg.data(), msg.size());
      HMAC_Final(&hmac.ctx, (unsigned char *)signed_msg, nullptr);

      return {signed_msg, signed_len};
    }

    namespace
    {
      constexpr char hexmap[] = {'0',
                                 '1',
                                 '2',
                                 '3',
                                 '4',
                                 '5',
                                 '6',
                                 '7',
                                 '8',
                                 '9',
                                 'a',
                                 'b',
                                 'c',
                                 'd',
                                 'e',
                                 'f'};
    }

    inline std::string string_to_hex(unsigned char *data, std::size_t len)
    {
      std::string s(len * 2, ' ');
      for (std::size_t i = 0; i < len; ++i)
      {
        s[2 * i] = hexmap[(data[i] & 0xF0) >> 4];
        s[2 * i + 1] = hexmap[data[i] & 0x0F];
      }
      return s;
    }
  } // namespace encoding
} // namespace util