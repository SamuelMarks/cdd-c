/* clang-format off */
#include "../../functions/emit/client_sig.c"
#include "greatest.h"
/* clang-format on */

TEST test_client_sig_utils(void) {
  int out;
  const struct OpenAPI_MediaType *mt_out;
  struct OpenAPI_MediaType mts[2];

  memset(&mts, 0, sizeof(mts));
  mts[0].name = "text/plain";
  mts[1].name = "application/json";

  /* test is_primitive_type */
  is_primitive_type("integer", &out);
  ASSERT_EQ(1, out);
  is_primitive_type("object", &out);
  ASSERT_EQ(0, out);

  /* test param_is_object_kv */
  {
    struct OpenAPI_Parameter p;
    memset(&p, 0, sizeof(p));
    p.in = OA_PARAM_IN_QUERY;
    p.type = "object";
    param_is_object_kv(&p, &out);
    ASSERT_EQ(1, out);
    p.in = OA_PARAM_IN_BODY;
    param_is_object_kv(&p, &out);
    ASSERT_EQ(0, out);
  }

  /* test media type utilities */
  ASSERT_EQ(1, media_type_has_prefix("APPLICATION/JSON", "application/"));
  ASSERT_EQ(0, media_type_has_prefix("text", "text/plain"));
  ASSERT_EQ(1, media_type_has_suffix("APPLICATION/JSON", "/json"));
  ASSERT_EQ(0, media_type_has_suffix("text", "application/json"));
  ASSERT_EQ(1, media_type_ieq("APPLICATION/JSON", "application/json"));
  ASSERT_EQ(0, media_type_ieq("text", "application/json"));
  ASSERT_EQ(0, media_type_ieq("application/json", "app"));

  ASSERT_EQ(0, media_type_ieq(NULL, "app"));
  ASSERT_EQ(0, media_type_ieq("app", NULL));

  ASSERT_EQ(1, media_type_is_json("application/json"));
  ASSERT_EQ(1, media_type_is_form("application/x-www-form-urlencoded"));
  ASSERT_EQ(1, media_type_is_text_plain("text/plain"));
  ASSERT_EQ(1, media_type_is_multipart("multipart/form-data"));
  ASSERT_EQ(1, media_type_is_multipart_form("multipart/form-data"));
  ASSERT_EQ(1, media_type_is_octet_stream("application/octet-stream"));

  ASSERT_EQ(CDD_C_SUCCESS, find_media_type(mts, 2, "text/plain", &mt_out));
  ASSERT(mt_out == &mts[0]);

  ASSERT_EQ(CDD_C_SUCCESS, find_media_type(mts, 2, "not/found", &mt_out));
  ASSERT(mt_out == NULL);

  ASSERT_EQ(CDD_C_SUCCESS, find_media_type(NULL, 2, "text/plain", &mt_out));
  ASSERT(mt_out == NULL);

  PASS();
}
