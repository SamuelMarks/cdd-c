/* clang-format off */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit/jwt.h"
#include "classes/emit/struct.h"
#include "win_compat_sym.h"
/* clang-format on */

int write_struct_from_jwt_func(FILE *fp, const char *struct_name,
                               const struct StructFields *sf) {
  if (!fp || !struct_name || !sf)
    return EINVAL;

  /* Helper to Base64URL decode the middle part */
  fprintf(fp, "/**\n * @brief Constant-time Base64URL decode utility.\n */\n");
  fprintf(fp, "static int cdd_c_base64url_decode(const char *in, size_t "
              "in_len, unsigned char **out, size_t *out_len) {\n");
  fprintf(fp, "  static const unsigned char b64url_dec[256] = {\n");
  fprintf(
      fp,
      "    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,\n");
  fprintf(
      fp,
      "    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,\n");
  fprintf(
      fp,
      "    255,255,255,255,255,255,255,255,255,255,255,255,255, 62,255,255,\n");
  fprintf(
      fp,
      "     52, 53, 54, 55, 56, 57, 58, 59, 60, 61,255,255,255,255,255,255,\n");
  fprintf(
      fp,
      "    255,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,\n");
  fprintf(
      fp,
      "     15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,255,255,255,255, 63,\n");
  fprintf(
      fp,
      "    255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,\n");
  fprintf(
      fp,
      "     41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,255,255,255,255,255,\n");
  fprintf(
      fp,
      "    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,\n");
  fprintf(
      fp,
      "    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,\n");
  fprintf(
      fp,
      "    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,\n");
  fprintf(
      fp,
      "    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,\n");
  fprintf(
      fp,
      "    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,\n");
  fprintf(
      fp,
      "    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,\n");
  fprintf(
      fp,
      "    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,\n");
  fprintf(
      fp,
      "    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255\n");
  fprintf(fp, "  };\n");
  fprintf(fp, "  size_t i, j;\n");
  fprintf(fp, "  unsigned char *res;\n");
  fprintf(fp, "  size_t pad = 0;\n");
  fprintf(fp, "  size_t max_out_len;\n");
  fprintf(fp, "  if (in_len > 0 && in[in_len - 1] == '=') pad++;\n");
  fprintf(fp, "  if (in_len > 1 && in[in_len - 2] == '=') pad++;\n");
  fprintf(fp, "  max_out_len = (in_len * 3) / 4 - pad;\n");
  fprintf(fp, "  res = (unsigned char *)malloc(max_out_len + 1);\n");
  fprintf(fp, "  if (!res) return 1;\n");
  fprintf(fp, "  for (i = 0, j = 0; i < in_len;) {\n");
  fprintf(fp, "    unsigned int n = 0;\n");
  fprintf(fp, "    int k;\n");
  fprintf(fp, "    for (k = 0; k < 4; k++, i++) {\n");
  fprintf(fp,
          "      unsigned char c = (i < in_len) ? (unsigned char)in[i] : 0;\n");
  fprintf(fp, "      unsigned int b = (c == '=') ? 0 : b64url_dec[c];\n");
  fprintf(fp, "      if (b == 255) { free(res); return 1; }\n");
  fprintf(fp, "      n = (n << 6) | b;\n");
  fprintf(fp, "    }\n");
  fprintf(fp, "    if (j < max_out_len) res[j++] = (unsigned char)((n >> 16) & "
              "0xFF);\n");
  fprintf(fp, "    if (j < max_out_len) res[j++] = (unsigned char)((n >> 8) & "
              "0xFF);\n");
  fprintf(fp,
          "    if (j < max_out_len) res[j++] = (unsigned char)(n & 0xFF);\n");
  fprintf(fp, "  }\n");
  fprintf(fp, "  res[j] = '\\0';\n");
  fprintf(fp, "  *out = res;\n");
  fprintf(fp, "  *out_len = j;\n");
  fprintf(fp, "  return 0;\n");
  fprintf(fp, "}\n\n");

  /* The actual parser */
  fprintf(fp, "/**\n * @brief Extract JWT Payload and parse into %s.\n */\n",
          struct_name);
  fprintf(fp, "int %s_from_jwt(const char *jwt, struct %s **const out) {\n",
          struct_name, struct_name);
  fprintf(fp, "  const char *dot1;\n");
  fprintf(fp, "  const char *dot2;\n");
  fprintf(fp, "  size_t payload_len;\n");
  fprintf(fp, "  unsigned char *payload_json = NULL;\n");
  fprintf(fp, "  size_t payload_json_len = 0;\n");
  fprintf(fp, "  int rc;\n\n");
  fprintf(fp, "  if (!jwt || !out) return 1;\n");
  fprintf(fp, "  dot1 = strchr(jwt, '.');\n");
  fprintf(fp, "  if (!dot1) return 1;\n");
  fprintf(fp, "  dot2 = strchr(dot1 + 1, '.');\n");
  fprintf(fp, "  if (!dot2) return 1;\n");
  fprintf(fp, "  payload_len = (size_t)(dot2 - (dot1 + 1));\n");
  fprintf(fp, "  if (cdd_c_base64url_decode(dot1 + 1, payload_len, "
              "&payload_json, &payload_json_len) != 0) {\n");
  fprintf(fp, "    return 1;\n");
  fprintf(fp, "  }\n");
  fprintf(fp, "  rc = %s_from_json((const char *)payload_json, out);\n",
          struct_name);
  fprintf(fp, "  free(payload_json);\n");
  fprintf(fp, "  return rc;\n");
  fprintf(fp, "}\n");
  return 0;
}