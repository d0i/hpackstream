// supports non-huffman encoding

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>

int encmax(int b){
  switch (b){
  case 0:
    return 0x00;
  case 1:
    return 0x01;
  case 2:
    return 0x03;
  case 3:
    return 0x07;
  case 4:
    return 0x0f;
  case 5:
    return 0x1f;
  case 6:
    return 0x3f;
  case 7:
    return 0x7f;
  default:
    assert(0); // should not happen
    return -1;
  }
}


// prefix_len = leading number of bits for the first octet
// returns number of bytes consumed
// section 4.1.1
int enc_integer(u_int8_t *buf, int buf_len, int64_t value, int prefix_len){
  int max_first = encmax(8-prefix_len);
  int count = 0;
  if (value < max_first){
    buf[count++] |= value;
    return count;
  } else {
    buf[count++] |= max_first;
    value -= max_first;
    while (value > 0x7f && count < buf_len-1){
      buf[count++] = 0x80 | (value % 0x80);
      value /= 0x80;
    }
    if (count >= buf_len-1){
      fprintf(stderr, "enc_interger: interger overflow\n");
      return -1;
    }
    buf[count++] = value;
    return count;
  }
}

// next_read_pp is pointer-pointer to return where to read next
// section 4.1.1
int64_t dec_integer(u_int8_t *data, int data_len, int prefix_len, u_int8_t **next_read_pp){
  int64_t value;
  long m;
  int max_first = encmax(8-prefix_len);
  int count = 0;
  int b;

  value = data[count++] & max_first;
  if (value == max_first){
    m = 1L;
    do {
      b = data[count++];
      value += (b & 0x7f) * m;
      m = m << 7;
    } while (b & 128 == 128 && count < data_len && count < 9);
    // 9 * 7 == 63 is the maximum count it can decode
    if (count == 9){
      fprintf(stderr, "dec_integer: integer overflow\n");
      return -1;
    }
  }
  *next_read_pp = data+count;
  return value;
}

// section 4.1.2
int enc_string(u_int8_t *buf, int buf_len, u_int8_t *s, int s_len){
  int i;
  u_int8_t *wp;
  buf[0] = 0;
  if ((i = enc_integer(buf, buf_len, s_len, 1)) < 0){
    return -1;
  }
  wp = buf + i;
  if (wp + s_len  > buf + buf_len){
    fprintf(stderr, "enc_string: insufficient buffer\n");
    return -1;
  }
  memcpy(wp, s, s_len);// no trailing \0
  return i+s_len;
}

// buffer will have trailing \0, but it returns string length (\0 excluded)
// section 4.1.2
// FIXME: should it be zero-copy?
int dec_string(u_int8_t *data, int data_len, u_int8_t *buf, int buf_len){
  int64_t slen;
  u_int8_t *s;
  if (data[0] & 0x80 == 0x80){
    fprintf(stderr, "dec_string: huffman encoding is not supported\n");
    return -1;
  }

  slen = dec_integer(data, data_len, 1, &s);
  if (slen+1 > buf_len){
    fprintf(stderr, "dec_string: insufficient buffer\n");
    return -1;
  }
  memcpy(buf, s, slen);
  buf[slen] = '\0';
  return slen;
}

#ifdef ENCODING_TEST

int main(int argc, char **argv){

  u_int8_t buf[128];
  int buflen = 128;
  u_int8_t *rp;
  int r;
  int64_t v;
  int i;

  // section 4.1.1.1
  memset(buf, 0, sizeof(buf));
  buf[0] = 0xc0; // 11000000
  r = enc_integer(buf, sizeof(buf), 10, 3);
  assert(r == 1);
  assert(buf[0] == 0xc0 | 0x0a);
  
  v = dec_integer(buf, sizeof(buf), 3, &rp);
  assert(v == 10);
  assert(rp == buf + 1);

  // section 4.1.1.2
  memset(buf, 0, sizeof(buf));
  buf[0] = 0xc0;
  r = enc_integer(buf, sizeof(buf), 1337, 3);
  assert(r == 3);
  assert(buf[0] == 0xc0 | 0x1f);
  assert(buf[1] == 0x9a);
  assert(buf[2] == 0x0a);
  
  v = dec_integer(buf, sizeof(buf), 3, &rp);
  assert(v == 1337);
  assert(rp == buf + 3);

  // section 4.1.1.3
  memset(buf, 0, sizeof(buf));
  r = enc_integer(buf, sizeof(buf), 42, 0);
  assert(r == 1);
  assert(buf[0] == 0x2a);

  v = dec_integer(buf, sizeof(buf), 0, &rp);
  assert(v == 42);
  assert(rp == buf + 1);


  for (i = 0; i < 65536; i++){
    buf[0] = 0;
    r = enc_integer(buf, sizeof(buf), i, 0);
    v = dec_integer(buf, sizeof(buf), 0, &rp);
    assert(i == v);
    assert(rp == buf + r);

    buf[0] = 0;
    r = enc_integer(buf, sizeof(buf), i, 7);
    v = dec_integer(buf, sizeof(buf), 0, &rp);
    assert(i == v);
    assert(rp == buf + r);
  }

  fprintf(stderr, "test for string is not yet.\n");
  assert(0);
}
#endif

