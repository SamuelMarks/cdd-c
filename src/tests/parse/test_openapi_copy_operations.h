/**
 * @file test_openapi_copy_operations.h
 * @brief Unit tests for OpenAPI operation, path, callback, server, and link
 * deep-copy routines.
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_COPY_OPERATIONS_H
#define TEST_OPENAPI_COPY_OPERATIONS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
/* clang-format on */

/**
 * @brief Unit test for operation copy and free helpers.
 * @return Greatest test result.
 */
TEST test_openapi_copy_operations_helpers(void) {
  struct OpenAPI_Operation op_src, op_dst;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  memset(&op_src, 0, sizeof(op_src));
  memset(&op_dst, 0, sizeof(op_dst));
  ASSERT_EQ(CDD_C_SUCCESS, copy_operation_fields(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, copy_operation_fields(&op_dst, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, copy_operation_fields(NULL, &op_src));

  op_src.operation_id = strdup("op1");
  op_src.summary = strdup("op summary");
  op_src.description = strdup("op description");
  op_src.req_body_ref = strdup("#/components/requestBodies/Body1");
  op_src.req_body_extensions_json = strdup("{\"x-rb\":1}");
  op_src.external_docs.description = strdup("docs desc");
  op_src.external_docs.url = strdup("http://docs");
  op_src.external_docs.extensions_json = strdup("{\"x-doc\":1}");
  op_src.req_body.inline_type = strdup("string");
  op_src.req_body.content_type = strdup("application/json");
  op_src.req_body.ref_name = strdup("MyRef");
  op_src.n_security = 1;
  op_src.security = (struct OpenAPI_SecurityRequirementSet *)calloc(
      1, sizeof(struct OpenAPI_SecurityRequirementSet));
  if (op_src.security) {
    op_src.security[0].n_requirements = 1;
    op_src.security[0].requirements =
        (struct OpenAPI_SecurityRequirement *)calloc(
            1, sizeof(struct OpenAPI_SecurityRequirement));
    if (op_src.security[0].requirements) {
      op_src.security[0].requirements[0].scheme = strdup("Basic");
      op_src.security[0].requirements[0].n_scopes = 1;
      op_src.security[0].requirements[0].scopes =
          (char **)calloc(1, sizeof(char *));
      if (op_src.security[0].requirements[0].scopes) {
        op_src.security[0].requirements[0].scopes[0] = strdup("read");
      }
    }
    op_src.security[0].extensions_json = strdup("{\"x-s\":1}");
  }

  op_src.n_servers = 1;
  op_src.servers =
      (struct OpenAPI_Server *)calloc(1, sizeof(struct OpenAPI_Server));
  if (op_src.servers) {
    op_src.servers[0].url = strdup("http://srv");
  }

  op_src.n_responses = 1;
  op_src.responses =
      (struct OpenAPI_Response *)calloc(1, sizeof(struct OpenAPI_Response));
  if (op_src.responses) {
    op_src.responses[0].code = strdup("200");
  }

  op_src.n_callbacks = 1;
  op_src.callbacks =
      (struct OpenAPI_Callback *)calloc(1, sizeof(struct OpenAPI_Callback));
  if (op_src.callbacks) {
    op_src.callbacks[0].name = strdup("my_cb");
  }

  rc = copy_operation_fields(&op_dst, &op_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("op1", op_dst.operation_id);
  ASSERT_STR_EQ("op summary", op_dst.summary);
  ASSERT_STR_EQ("op description", op_dst.description);
  ASSERT_STR_EQ("#/components/requestBodies/Body1", op_dst.req_body_ref);
  ASSERT_STR_EQ("{\"x-rb\":1}", op_dst.req_body_extensions_json);

  g_cdd_last_alloc_err = free_operation(&op_dst);

  {
    int k;
    for (k = 1; k <= 30; ++k) {
      memset(&op_dst, 0, sizeof(op_dst));
      g_cdd_alloc_fail = k;
      rc = copy_operation_fields(&op_dst, &op_src);
      if (rc != CDD_C_SUCCESS) {
        ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      }
      g_cdd_last_alloc_err = free_operation(&op_dst);
    }
    g_cdd_alloc_fail = 0;
  }

  {
    int k;
    for (k = 1; k <= 30; ++k) {
      memset(&op_dst, 0, sizeof(op_dst));
      g_cdd_strdup_fail = k;
      rc = copy_operation_fields(&op_dst, &op_src);
      if (rc != CDD_C_SUCCESS) {
        ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      }
      g_cdd_last_alloc_err = free_operation(&op_dst);
    }
    g_cdd_strdup_fail = 0;
  }

  g_cdd_last_alloc_err = free_operation(&op_src);
  g_cdd_last_alloc_err = free_operation(NULL);

  PASS();
}

/**
 * @brief Unit test for operation copy OOM branches.
 * @return Greatest test result.
 */
TEST test_openapi_copy_operations_oom(void) {
  int k;
  struct OpenAPI_Parameter p_src;
  struct OpenAPI_Operation op_src, op_dst;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  for (k = 1; k <= 16; ++k) {
    char *op_tag = cdd_test_mut_str("tag1");
    memset(&p_src, 0, sizeof(p_src));
    p_src.name = cdd_test_mut_str("p");
    memset(&op_src, 0, sizeof(op_src));
    memset(&op_dst, 0, sizeof(op_dst));
    op_src.operation_id = cdd_test_mut_str("o");
    op_src.summary = cdd_test_mut_str("s");
    op_src.description = cdd_test_mut_str("d");
    op_src.req_body_ref = cdd_test_mut_str("r");
    op_src.req_body_extensions_json = cdd_test_mut_str("e");
    op_src.external_docs.extensions_json = cdd_test_mut_str("x");
    op_src.parameters = &p_src;
    op_src.n_parameters = 1;
    op_src.tags = &op_tag;
    op_src.n_tags = 1;
    g_cdd_alloc_fail = k;
    rc = copy_operation_fields(&op_dst, &op_src);
    g_cdd_alloc_fail = 0;
    if (rc != CDD_C_SUCCESS) {
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    }
    g_cdd_last_alloc_err = free_operation(&op_dst);
  }

  g_cdd_alloc_fail = 0;
  PASS();
}

/**
 * @brief Unit test for path, callback, request body, server, and link copy and
 * free helpers.
 * @return Greatest test result.
 */
TEST test_openapi_copy_operations_advanced(void) {
  struct OpenAPI_Path path_src, path_dst;
  struct OpenAPI_Callback cb_src, cb_dst;
  struct OpenAPI_RequestBody rb_src, rb_dst;
  struct OpenAPI_Server *srv_src, *srv_dst;
  struct OpenAPI_Link lnk_src, lnk_dst;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  /* 1. Path copy & free */
  memset(&path_src, 0, sizeof(path_src));
  memset(&path_dst, 0, sizeof(path_dst));
  ASSERT_EQ(CDD_C_SUCCESS, copy_path_fields(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, copy_path_fields(&path_dst, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, copy_path_fields(NULL, &path_src));

  path_src.route = strdup("/test");
  path_src.ref = strdup("#/components/pathItems/MyPath");
  path_src.summary = strdup("path summary");
  path_src.description = strdup("path description");
  path_src.extensions_json = strdup("{\"x-p\":1}");
  path_src.parameters =
      (struct OpenAPI_Parameter *)calloc(1, sizeof(struct OpenAPI_Parameter));
  if (path_src.parameters) {
    path_src.parameters[0].name = strdup("p1");
    path_src.n_parameters = 1;
  }
  path_src.servers =
      (struct OpenAPI_Server *)calloc(1, sizeof(struct OpenAPI_Server));
  if (path_src.servers) {
    path_src.servers[0].url = strdup("http://srv");
    path_src.n_servers = 1;
  }
  path_src.operations =
      (struct OpenAPI_Operation *)calloc(1, sizeof(struct OpenAPI_Operation));
  if (path_src.operations) {
    path_src.operations[0].operation_id = strdup("op1");
    path_src.n_operations = 1;
  }
  path_src.additional_operations =
      (struct OpenAPI_Operation *)calloc(1, sizeof(struct OpenAPI_Operation));
  if (path_src.additional_operations) {
    path_src.additional_operations[0].operation_id = strdup("add_op");
    path_src.n_additional_operations = 1;
  }

  rc = copy_path_fields(&path_dst, &path_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("/test", path_dst.route);
  ASSERT_STR_EQ("#/components/pathItems/MyPath", path_dst.ref);
  ASSERT_STR_EQ("path summary", path_dst.summary);
  ASSERT_STR_EQ("path description", path_dst.description);

  g_cdd_last_alloc_err = free_path_item(&path_dst);

  {
    int k;
    for (k = 1; k <= 25; ++k) {
      memset(&path_dst, 0, sizeof(path_dst));
      g_cdd_alloc_fail = k;
      rc = copy_path_fields(&path_dst, &path_src);
      if (rc != CDD_C_SUCCESS) {
        ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      }
      g_cdd_last_alloc_err = free_path_item(&path_dst);
    }
    g_cdd_alloc_fail = 0;
    for (k = 1; k <= 25; ++k) {
      memset(&path_dst, 0, sizeof(path_dst));
      g_cdd_strdup_fail = k;
      rc = copy_path_fields(&path_dst, &path_src);
      if (rc != CDD_C_SUCCESS) {
        ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      }
      g_cdd_last_alloc_err = free_path_item(&path_dst);
    }
    g_cdd_strdup_fail = 0;
  }

  g_cdd_last_alloc_err = free_path_item(&path_src);

  /* 2. Callback copy & free */
  memset(&cb_src, 0, sizeof(cb_src));
  memset(&cb_dst, 0, sizeof(cb_dst));
  ASSERT_EQ(CDD_C_SUCCESS, copy_callback_fields(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, copy_callback_fields(&cb_dst, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, copy_callback_fields(NULL, &cb_src));

  cb_src.name = strdup("cb1");
  cb_src.ref = strdup("#/components/callbacks/MyCb");
  cb_src.summary = strdup("cb summary");
  cb_src.description = strdup("cb description");
  cb_src.extensions_json = strdup("{\"x-cb\":1}");
  cb_src.paths = (struct OpenAPI_Path *)calloc(1, sizeof(struct OpenAPI_Path));
  if (cb_src.paths) {
    cb_src.paths[0].route = strdup("/hook");
    cb_src.n_paths = 1;
  }

  rc = copy_callback_fields(&cb_dst, &cb_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("cb1", cb_dst.name);
  ASSERT_STR_EQ("#/components/callbacks/MyCb", cb_dst.ref);
  ASSERT_STR_EQ("cb summary", cb_dst.summary);

  g_cdd_last_alloc_err = free_callback(&cb_dst);

  {
    int k;
    for (k = 1; k <= 25; ++k) {
      memset(&cb_dst, 0, sizeof(cb_dst));
      g_cdd_alloc_fail = k;
      rc = copy_callback_fields(&cb_dst, &cb_src);
      if (rc != CDD_C_SUCCESS) {
        ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      }
      g_cdd_last_alloc_err = free_callback(&cb_dst);
    }
    g_cdd_alloc_fail = 0;
    for (k = 1; k <= 25; ++k) {
      memset(&cb_dst, 0, sizeof(cb_dst));
      g_cdd_strdup_fail = k;
      rc = copy_callback_fields(&cb_dst, &cb_src);
      if (rc != CDD_C_SUCCESS) {
        ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      }
      g_cdd_last_alloc_err = free_callback(&cb_dst);
    }
    g_cdd_strdup_fail = 0;
  }

  g_cdd_last_alloc_err = free_callback(&cb_src);

  /* 3. RequestBody copy & free */
  memset(&rb_src, 0, sizeof(rb_src));
  memset(&rb_dst, 0, sizeof(rb_dst));
  ASSERT_EQ(CDD_C_SUCCESS, copy_request_body_fields(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, copy_request_body_fields(&rb_dst, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, copy_request_body_fields(NULL, &rb_src));

  rb_src.ref = strdup("#/components/requestBodies/MyRb");
  rb_src.description = strdup("rb description");
  rb_src.content_ref = strdup("#/components/mediaTypes/Mt");
  rb_src.extensions_json = strdup("{\"x-rb\":1}");
  rb_src.example_set = 1;
  rb_src.example.type = OA_ANY_STRING;
  rb_src.example.string = strdup("ex");

  /* Test copy_security_requirement_sets NULL paths */
  ASSERT_EQ(CDD_C_SUCCESS, copy_security_requirement_sets(NULL, NULL, NULL, 0));
  {
    struct OpenAPI_SecurityRequirementSet *dst_dummy = NULL;
    size_t count_dummy = 0;
    ASSERT_EQ(CDD_C_SUCCESS, copy_security_requirement_sets(
                                 &dst_dummy, &count_dummy, NULL, 0));
  }

  /* Set existing description on rb_dst to cover free on overwrite */
  rb_dst.description = strdup("existing");
  rc = copy_request_body_fields(&rb_dst, &rb_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("#/components/requestBodies/MyRb", rb_dst.ref);
  ASSERT_STR_EQ("rb description", rb_dst.description);

  g_cdd_last_alloc_err = free_request_body(&rb_dst);

  {
    int k;
    for (k = 1; k <= 25; ++k) {
      memset(&rb_dst, 0, sizeof(rb_dst));
      g_cdd_alloc_fail = k;
      rc = copy_request_body_fields(&rb_dst, &rb_src);
      if (rc != CDD_C_SUCCESS) {
        ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      }
      g_cdd_last_alloc_err = free_request_body(&rb_dst);
    }
    g_cdd_alloc_fail = 0;
    for (k = 1; k <= 25; ++k) {
      memset(&rb_dst, 0, sizeof(rb_dst));
      g_cdd_strdup_fail = k;
      rc = copy_request_body_fields(&rb_dst, &rb_src);
      if (rc != CDD_C_SUCCESS) {
        ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      }
      g_cdd_last_alloc_err = free_request_body(&rb_dst);
    }
    g_cdd_strdup_fail = 0;
  }

  g_cdd_last_alloc_err = free_request_body(&rb_src);

  /* 4. Server copy & free */
  srv_src = (struct OpenAPI_Server *)calloc(1, sizeof(struct OpenAPI_Server));
  srv_dst = (struct OpenAPI_Server *)calloc(1, sizeof(struct OpenAPI_Server));
  ASSERT_EQ(CDD_C_SUCCESS, copy_server_object(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, copy_server_object(srv_dst, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, copy_server_object(NULL, srv_src));

  if (srv_src && srv_dst) {
    srv_src->url = strdup("https://example.com");
    srv_src->description = strdup("prod server");
    srv_src->name = strdup("prod");
    srv_src->extensions_json = strdup("{\"x-srv\":1}");
    srv_src->variables = (struct OpenAPI_ServerVariable *)calloc(
        1, sizeof(struct OpenAPI_ServerVariable));
    if (srv_src->variables) {
      srv_src->variables[0].name = strdup("port");
      srv_src->variables[0].default_value = strdup("443");
      srv_src->variables[0].description = strdup("port var");
      srv_src->variables[0].extensions_json = strdup("{\"x-var\":1}");
      srv_src->variables[0].n_enum_values = 1;
      srv_src->variables[0].enum_values = (char **)calloc(1, sizeof(char *));
      if (srv_src->variables[0].enum_values) {
        srv_src->variables[0].enum_values[0] = strdup("443");
      }
      srv_src->n_variables = 1;
    }

    rc = copy_server_object(srv_dst, srv_src);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("https://example.com", srv_dst->url);
    ASSERT_STR_EQ("prod server", srv_dst->description);

    g_cdd_last_alloc_err = openapi_free_servers_array(srv_dst, 1);
    srv_dst = NULL;

    {
      int k;
      for (k = 1; k <= 15; ++k) {
        struct OpenAPI_Server *tmp_srv =
            (struct OpenAPI_Server *)calloc(1, sizeof(struct OpenAPI_Server));
        g_cdd_alloc_fail = k;
        rc = copy_server_object(tmp_srv, srv_src);
        g_cdd_alloc_fail = 0;
        if (rc != CDD_C_SUCCESS) {
          ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
        }
        g_cdd_last_alloc_err = openapi_free_servers_array(tmp_srv, 1);
      }
      for (k = 1; k <= 15; ++k) {
        struct OpenAPI_Server *tmp_srv =
            (struct OpenAPI_Server *)calloc(1, sizeof(struct OpenAPI_Server));
        g_cdd_strdup_fail = k;
        rc = copy_server_object(tmp_srv, srv_src);
        if (rc != CDD_C_SUCCESS) {
          ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
        }
        g_cdd_last_alloc_err = openapi_free_servers_array(tmp_srv, 1);
      }
      g_cdd_strdup_fail = 0;
    }
  }

  g_cdd_last_alloc_err = openapi_free_servers_array(srv_src, 1);
  g_cdd_last_alloc_err = openapi_free_servers_array(NULL, 0);

  /* 5. Link copy & free */
  memset(&lnk_src, 0, sizeof(lnk_src));
  memset(&lnk_dst, 0, sizeof(lnk_dst));
  ASSERT_EQ(CDD_C_SUCCESS, copy_link_fields(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, copy_link_fields(&lnk_dst, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, copy_link_fields(NULL, &lnk_src));

  lnk_src.name = strdup("link1");
  lnk_src.ref = strdup("#/components/links/MyLink");
  lnk_src.operation_id = strdup("op1");
  lnk_src.operation_ref = strdup("#/paths/~1foo/get");
  lnk_src.summary = strdup("link summary");
  lnk_src.description = strdup("link description");
  lnk_src.extensions_json = strdup("{\"x-lnk\":1}");
  lnk_src.parameters =
      (struct OpenAPI_LinkParam *)calloc(1, sizeof(struct OpenAPI_LinkParam));
  if (lnk_src.parameters) {
    lnk_src.parameters[0].name = strdup("userId");
    lnk_src.parameters[0].value.type = OA_ANY_STRING;
    lnk_src.parameters[0].value.string = strdup("$response.body#/id");
    lnk_src.n_parameters = 1;
  }
  lnk_src.request_body_set = 1;
  lnk_src.request_body.type = OA_ANY_STRING;
  lnk_src.request_body.string = strdup("req");
  lnk_src.server_set = 1;
  lnk_src.server =
      (struct OpenAPI_Server *)calloc(1, sizeof(struct OpenAPI_Server));
  if (lnk_src.server)
    lnk_src.server->url = strdup("http://srv2");

  rc = copy_link_fields(&lnk_dst, &lnk_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("link summary", lnk_dst.summary);
  ASSERT_STR_EQ("link description", lnk_dst.description);

  g_cdd_last_alloc_err = free_link(&lnk_dst);

  {
    int k;
    for (k = 1; k <= 15; ++k) {
      memset(&lnk_dst, 0, sizeof(lnk_dst));
      g_cdd_alloc_fail = k;
      rc = copy_link_fields(&lnk_dst, &lnk_src);
      g_cdd_alloc_fail = 0;
      if (rc != CDD_C_SUCCESS) {
        ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      }
      g_cdd_last_alloc_err = free_link(&lnk_dst);
    }
    for (k = 1; k <= 15; ++k) {
      memset(&lnk_dst, 0, sizeof(lnk_dst));
      g_cdd_strdup_fail = k;
      rc = copy_link_fields(&lnk_dst, &lnk_src);
      if (rc != CDD_C_SUCCESS) {
        ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      }
      g_cdd_last_alloc_err = free_link(&lnk_dst);
    }
    g_cdd_strdup_fail = 0;
  }

  g_cdd_last_alloc_err = free_link(&lnk_src);

  PASS();
}

/**
 * @brief Unit test for edge-case branch coverage in openapi_copy_operations.c.
 * @return Greatest test result.
 */
TEST test_openapi_copy_operations_branches(void) {
  struct OpenAPI_Server s_srv_src;
  struct OpenAPI_ServerVariable svar_dummy;
  struct OpenAPI_Link s_lnk_src, s_lnk_dst;
  struct OpenAPI_SecurityRequirementSet sec_src, sec_dst;
  struct OpenAPI_SecurityRequirement sreq_dummy;
  struct OpenAPI_Callback s_cb_src, s_cb_dst;
  struct OpenAPI_Operation op_src, op_dst;
  struct OpenAPI_Path s_path_src, s_path_dst;
  struct OpenAPI_RequestBody s_rb_src, s_rb_dst;

  struct OpenAPI_MediaType dummy_mt;
  struct OpenAPI_Example dummy_ex;
  struct OpenAPI_LinkParam dummy_lp;
  struct OpenAPI_Path dummy_path;
  struct OpenAPI_Parameter dummy_param;
  struct OpenAPI_Server dummy_srv;
  struct OpenAPI_Operation dummy_op;
  char dummy_str_buf[8];
  char *dummy_str;
  char *dummy_str_arr[1];

  dummy_str_buf[0] = 'd';
  dummy_str_buf[1] = '\0';
  dummy_str = dummy_str_buf;
  dummy_str_arr[0] = NULL;

  memset(&dummy_mt, 0, sizeof(dummy_mt));
  memset(&dummy_ex, 0, sizeof(dummy_ex));
  memset(&dummy_lp, 0, sizeof(dummy_lp));
  memset(&dummy_path, 0, sizeof(dummy_path));
  memset(&dummy_param, 0, sizeof(dummy_param));
  memset(&dummy_srv, 0, sizeof(dummy_srv));
  memset(&dummy_op, 0, sizeof(dummy_op));
  memset(&svar_dummy, 0, sizeof(svar_dummy));
  memset(&sreq_dummy, 0, sizeof(sreq_dummy));

  /* 1. copy_server_object branches */
  {
    struct OpenAPI_Server *p_srv =
        (struct OpenAPI_Server *)calloc(1, sizeof(struct OpenAPI_Server));
    memset(&s_srv_src, 0, sizeof(s_srv_src));
    s_srv_src.variables = NULL;
    s_srv_src.n_variables = 1;
    ASSERT_EQ(CDD_C_SUCCESS, copy_server_object(p_srv, &s_srv_src));
    g_cdd_last_alloc_err = openapi_free_servers_array(p_srv, 1);

    p_srv = (struct OpenAPI_Server *)calloc(1, sizeof(struct OpenAPI_Server));
    memset(&s_srv_src, 0, sizeof(s_srv_src));
    s_srv_src.variables = &svar_dummy;
    s_srv_src.n_variables = 1;
    svar_dummy.name = NULL;
    svar_dummy.default_value = NULL;
    svar_dummy.enum_values = NULL;
    svar_dummy.n_enum_values = 1;
    ASSERT_EQ(CDD_C_SUCCESS, copy_server_object(p_srv, &s_srv_src));
    g_cdd_last_alloc_err = openapi_free_servers_array(p_srv, 1);

    p_srv = (struct OpenAPI_Server *)calloc(1, sizeof(struct OpenAPI_Server));
    memset(&s_srv_src, 0, sizeof(s_srv_src));
    s_srv_src.variables = &svar_dummy;
    s_srv_src.n_variables = 1;
    svar_dummy.name = dummy_str;
    svar_dummy.default_value = dummy_str;
    svar_dummy.enum_values = dummy_str_arr;
    svar_dummy.n_enum_values = 1;
    ASSERT_EQ(CDD_C_SUCCESS, copy_server_object(p_srv, &s_srv_src));
    g_cdd_last_alloc_err = openapi_free_servers_array(p_srv, 1);

    p_srv = (struct OpenAPI_Server *)calloc(1, sizeof(struct OpenAPI_Server));
    memset(&s_srv_src, 0, sizeof(s_srv_src));
    s_srv_src.variables = &svar_dummy;
    s_srv_src.n_variables = 1;
    svar_dummy.name = dummy_str;
    svar_dummy.default_value = dummy_str;
    svar_dummy.enum_values = &dummy_str;
    svar_dummy.n_enum_values = 0;
    ASSERT_EQ(CDD_C_SUCCESS, copy_server_object(p_srv, &s_srv_src));
    g_cdd_last_alloc_err = openapi_free_servers_array(p_srv, 1);
  }

  /* 2. copy_link_fields branches */
  memset(&s_lnk_src, 0, sizeof(s_lnk_src));
  memset(&s_lnk_dst, 0, sizeof(s_lnk_dst));
  s_lnk_src.parameters = NULL;
  s_lnk_src.n_parameters = 1;
  s_lnk_src.server_set = 1;
  s_lnk_src.server = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, copy_link_fields(&s_lnk_dst, &s_lnk_src));

  memset(&s_lnk_src, 0, sizeof(s_lnk_src));
  memset(&s_lnk_dst, 0, sizeof(s_lnk_dst));
  s_lnk_src.parameters = &dummy_lp;
  s_lnk_src.n_parameters = 1;
  dummy_lp.name = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, copy_link_fields(&s_lnk_dst, &s_lnk_src));
  g_cdd_last_alloc_err = free_link(&s_lnk_dst);

  /* 3. copy_security_requirement_sets branches */
  memset(&sec_src, 0, sizeof(sec_src));
  memset(&sec_dst, 0, sizeof(sec_dst));
  {
    struct OpenAPI_SecurityRequirementSet *sec_arr = NULL;
    size_t sec_cnt = 0;
    ASSERT_EQ(CDD_C_SUCCESS,
              copy_security_requirement_sets(&sec_arr, NULL, &sec_src, 1));
    ASSERT_EQ(CDD_C_SUCCESS,
              copy_security_requirement_sets(&sec_arr, &sec_cnt, &sec_src, 0));
  }
  sec_src.requirements = NULL;
  sec_src.n_requirements = 1;
  {
    struct OpenAPI_SecurityRequirementSet *dst_p = NULL;
    size_t cnt_p = 0;
    ASSERT_EQ(CDD_C_SUCCESS,
              copy_security_requirement_sets(&dst_p, &cnt_p, &sec_src, 1));
    g_cdd_last_alloc_err = free_security_requirement_set(dst_p);
    free(dst_p);
  }
  memset(&sec_src, 0, sizeof(sec_src));
  sec_src.requirements = &sreq_dummy;
  sec_src.n_requirements = 1;
  sreq_dummy.scheme = NULL;
  sreq_dummy.scopes = NULL;
  sreq_dummy.n_scopes = 1;
  {
    struct OpenAPI_SecurityRequirementSet *dst_p = NULL;
    size_t cnt_p = 0;
    ASSERT_EQ(CDD_C_SUCCESS,
              copy_security_requirement_sets(&dst_p, &cnt_p, &sec_src, 1));
    g_cdd_last_alloc_err = free_security_requirement_set(dst_p);
    free(dst_p);
  }
  sreq_dummy.scopes = dummy_str_arr;
  sreq_dummy.n_scopes = 1;
  {
    struct OpenAPI_SecurityRequirementSet *dst_p = NULL;
    size_t cnt_p = 0;
    ASSERT_EQ(CDD_C_SUCCESS,
              copy_security_requirement_sets(&dst_p, &cnt_p, &sec_src, 1));
    g_cdd_last_alloc_err = free_security_requirement_set(dst_p);
    free(dst_p);
  }
  memset(&sec_src, 0, sizeof(sec_src));
  sec_src.requirements = &sreq_dummy;
  sec_src.n_requirements = 0;
  {
    struct OpenAPI_SecurityRequirementSet *dst_p = NULL;
    size_t cnt_p = 0;
    ASSERT_EQ(CDD_C_SUCCESS,
              copy_security_requirement_sets(&dst_p, &cnt_p, &sec_src, 1));
    g_cdd_last_alloc_err = free_security_requirement_set(dst_p);
    free(dst_p);
  }

  /* 4. copy_callback_fields branches */
  memset(&s_cb_src, 0, sizeof(s_cb_src));
  memset(&s_cb_dst, 0, sizeof(s_cb_dst));
  s_cb_src.name = dummy_str;
  s_cb_dst.name = strdup("n");
  s_cb_src.ref = dummy_str;
  s_cb_dst.ref = strdup("r");
  s_cb_src.summary = dummy_str;
  s_cb_dst.summary = strdup("s");
  s_cb_src.description = dummy_str;
  s_cb_dst.description = strdup("d");
  s_cb_src.extensions_json = dummy_str;
  s_cb_dst.extensions_json = strdup("e");
  ASSERT_EQ(CDD_C_SUCCESS, copy_callback_fields(&s_cb_dst, &s_cb_src));
  g_cdd_last_alloc_err = free_callback(&s_cb_dst);

  memset(&s_cb_src, 0, sizeof(s_cb_src));
  memset(&s_cb_dst, 0, sizeof(s_cb_dst));
  ASSERT_EQ(CDD_C_SUCCESS, copy_callback_fields(&s_cb_dst, &s_cb_src));

  memset(&s_cb_src, 0, sizeof(s_cb_src));
  memset(&s_cb_dst, 0, sizeof(s_cb_dst));
  s_cb_src.n_paths = 1;
  s_cb_src.paths = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, copy_callback_fields(&s_cb_dst, &s_cb_src));

  memset(&s_cb_src, 0, sizeof(s_cb_src));
  memset(&s_cb_dst, 0, sizeof(s_cb_dst));
  s_cb_dst.paths = &dummy_path;
  s_cb_src.n_paths = 1;
  s_cb_src.paths = &dummy_path;
  ASSERT_EQ(CDD_C_SUCCESS, copy_callback_fields(&s_cb_dst, &s_cb_src));
  memset(&s_cb_dst, 0, sizeof(s_cb_dst));

  /* 5. copy_operation_fields branches */
  memset(&op_src, 0, sizeof(op_src));
  memset(&op_dst, 0, sizeof(op_dst));
  op_src.security = NULL;
  op_src.n_security = 1;
  op_src.parameters = NULL;
  op_src.n_parameters = 1;
  op_src.tags = NULL;
  op_src.n_tags = 1;
  op_src.req_body_media_types = NULL;
  op_src.n_req_body_media_types = 1;
  op_src.servers = NULL;
  op_src.n_servers = 1;
  op_src.responses = NULL;
  op_src.n_responses = 1;
  op_src.callbacks = NULL;
  op_src.n_callbacks = 1;
  ASSERT_EQ(CDD_C_SUCCESS, copy_operation_fields(&op_dst, &op_src));

  memset(&op_src, 0, sizeof(op_src));
  memset(&op_dst, 0, sizeof(op_dst));
  op_src.tags = dummy_str_arr;
  op_src.n_tags = 1;
  ASSERT_EQ(CDD_C_SUCCESS, copy_operation_fields(&op_dst, &op_src));
  g_cdd_last_alloc_err = free_operation(&op_dst);

  /* 6. copy_path_fields branches */
  memset(&s_path_src, 0, sizeof(s_path_src));
  memset(&s_path_dst, 0, sizeof(s_path_dst));
  s_path_src.route = dummy_str;
  s_path_dst.route = strdup("rt");
  s_path_src.ref = dummy_str;
  s_path_dst.ref = strdup("rf");
  s_path_src.summary = dummy_str;
  s_path_dst.summary = strdup("sm");
  s_path_src.description = dummy_str;
  s_path_dst.description = strdup("ds");
  s_path_src.extensions_json = dummy_str;
  s_path_dst.extensions_json = strdup("ex");
  s_path_src.parameters = NULL;
  s_path_src.n_parameters = 1;
  s_path_src.servers = NULL;
  s_path_src.n_servers = 1;
  s_path_src.operations = NULL;
  s_path_src.n_operations = 1;
  ASSERT_EQ(CDD_C_SUCCESS, copy_path_fields(&s_path_dst, &s_path_src));
  g_cdd_last_alloc_err = free_path_item(&s_path_dst);

  memset(&s_path_src, 0, sizeof(s_path_src));
  memset(&s_path_dst, 0, sizeof(s_path_dst));
  s_path_dst.parameters = &dummy_param;
  s_path_dst.servers = &dummy_srv;
  s_path_dst.operations = &dummy_op;
  s_path_dst.additional_operations = &dummy_op;
  s_path_src.n_parameters = 1;
  s_path_src.parameters = &dummy_param;
  s_path_src.n_servers = 1;
  s_path_src.servers = &dummy_srv;
  s_path_src.n_operations = 1;
  s_path_src.operations = &dummy_op;
  s_path_src.n_additional_operations = 1;
  s_path_src.additional_operations = &dummy_op;
  ASSERT_EQ(CDD_C_SUCCESS, copy_path_fields(&s_path_dst, &s_path_src));
  memset(&s_path_dst, 0, sizeof(s_path_dst));

  memset(&s_path_src, 0, sizeof(s_path_src));
  memset(&s_path_dst, 0, sizeof(s_path_dst));
  s_path_src.n_additional_operations = 1;
  s_path_src.additional_operations = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, copy_path_fields(&s_path_dst, &s_path_src));

  /* 7. copy_request_body_fields branches */
  memset(&s_rb_src, 0, sizeof(s_rb_src));
  memset(&s_rb_dst, 0, sizeof(s_rb_dst));
  s_rb_src.ref = dummy_str;
  s_rb_dst.ref = strdup("old_ref");
  s_rb_src.description = dummy_str;
  s_rb_dst.description = strdup("old_desc");
  s_rb_src.content_media_types = &dummy_mt;
  s_rb_src.n_content_media_types = 0;
  s_rb_src.examples = &dummy_ex;
  s_rb_src.n_examples = 0;
  ASSERT_EQ(CDD_C_SUCCESS, copy_request_body_fields(&s_rb_dst, &s_rb_src));
  g_cdd_last_alloc_err = free_request_body(&s_rb_dst);

  memset(&s_rb_src, 0, sizeof(s_rb_src));
  memset(&s_rb_dst, 0, sizeof(s_rb_dst));
  s_rb_src.description = dummy_str;
  s_rb_dst.description = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, copy_request_body_fields(&s_rb_dst, &s_rb_src));
  g_cdd_last_alloc_err = free_request_body(&s_rb_dst);

  memset(&s_rb_src, 0, sizeof(s_rb_src));
  memset(&s_rb_dst, 0, sizeof(s_rb_dst));
  s_rb_src.description = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, copy_request_body_fields(&s_rb_dst, &s_rb_src));
  g_cdd_last_alloc_err = free_request_body(&s_rb_dst);

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_COPY_OPERATIONS_H */
