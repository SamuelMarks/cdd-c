/**
 * @file hydrate_router.c
 * @brief Implementation of dynamic runtime routing APIs for C-ORM struct
 * hydration.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "classes/parse/hydrate_router.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#if defined(_MSC_VER)
#define CDD_C_THREAD_LOCAL __declspec(thread)
#elif defined(__GNUC__) || defined(__clang__)
#define CDD_C_THREAD_LOCAL __thread
#else
#define CDD_C_THREAD_LOCAL
#endif

static CDD_C_THREAD_LOCAL char cdd_c_hydrate_error_msg[512] = {0};

const char *cdd_c_hydrate_router_get_last_error(void) {
  if (cdd_c_hydrate_error_msg[0] == '\0') {
    return NULL;
  }
  return cdd_c_hydrate_error_msg;
}

void cdd_c_hydrate_router_set_last_error(const char *msg) {
  if (!msg) {
    cdd_c_hydrate_error_msg[0] = '\0';
  } else {
    strncpy(cdd_c_hydrate_error_msg, msg, sizeof(cdd_c_hydrate_error_msg) - 1);
    cdd_c_hydrate_error_msg[sizeof(cdd_c_hydrate_error_msg) - 1] = '\0';
  }
}

int cdd_c_hydrate_router_init(cdd_c_hydrate_router_t *router) {
  if (!router)
    return -1;
  router->routes = NULL;
  router->count = 0;
  router->capacity = 0;
  return 0;
}

int cdd_c_hydrate_router_register(cdd_c_hydrate_router_t *router,
                                  unsigned long long query_id_hash,
                                  const struct c_orm_meta *struct_meta,
                                  cdd_c_specific_hydrator_fn hydrate_fn) {
  cdd_c_hydrate_route_t *new_routes;
  size_t new_cap, i;

  if (!router || !hydrate_fn)
    return -1;

  for (i = 0; i < router->count; ++i) {
    if (router->routes[i].query_id_hash == query_id_hash) {
      /* Update existing route */
      router->routes[i].struct_meta = struct_meta;
      router->routes[i].hydrate_fn = hydrate_fn;
      return 0;
    }
  }

  if (router->count >= router->capacity) {
    new_cap = router->capacity == 0 ? 8 : router->capacity * 2;
    new_routes = (cdd_c_hydrate_route_t *)realloc(
        router->routes, new_cap * sizeof(cdd_c_hydrate_route_t));
    if (!new_routes)
      return -1;
    router->routes = new_routes;
    router->capacity = new_cap;
  }

  router->routes[router->count].query_id_hash = query_id_hash;
  router->routes[router->count].struct_meta = struct_meta;
  router->routes[router->count].hydrate_fn = hydrate_fn;
  router->count++;

  return 0;
}

int cdd_c_hydrate_router_dispatch(const cdd_c_hydrate_router_t *router,
                                  unsigned long long query_id_hash,
                                  const cdd_c_abstract_struct_t *row,
                                  void *out_struct) {
  size_t i;
  int res;

  if (!router || !row || !out_struct) {
    cdd_c_hydrate_router_set_last_error("Invalid arguments to router_dispatch");
    return -1;
  }

  cdd_c_hydrate_router_set_last_error(NULL); /* Clear previous error */

  for (i = 0; i < router->count; ++i) {
    if (router->routes[i].query_id_hash == query_id_hash) {
      /* Hand execution cleanly off to the codegen handler */
      res = router->routes[i].hydrate_fn(out_struct, row);
      if (res != 0) {
        cdd_c_hydrate_router_set_last_error(
            "Hydration function returned error");
      }
      return res;
    }
  }

  /* Target not found - indicates consumer must utilize fallback
   * cdd_c_abstract_hydrate mechanics */
  cdd_c_hydrate_router_set_last_error("Route not found for query ID");
  return -1;
}

int cdd_c_hydrate_router_free(cdd_c_hydrate_router_t *router) {
  if (!router)
    return -1;
  if (router->routes)
    free(router->routes);
  router->routes = NULL;
  router->count = 0;
  router->capacity = 0;
  return 0;
}
