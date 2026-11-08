/**
 * @file openapi_metadata.c
 * @brief Metadata, tags, discriminator, XML, and server parsing.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/** @brief Reflected fields for `OpenAPI_Contact`. */
static const struct cdd_field_info g_openapi_contact_fields[] = {
    {"name",
     "name",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_Contact, name),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"url",
     "url",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_Contact, url),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"email",
     "email",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_Contact, email),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info OpenAPI_Contact_type_info = {
    "OpenAPI_Contact",
    sizeof(struct OpenAPI_Contact),
    0,
    g_openapi_contact_fields,
    sizeof(g_openapi_contact_fields) / sizeof(g_openapi_contact_fields[0]),
    0};

/** @brief Reflected fields for `OpenAPI_License`. */
static const struct cdd_field_info g_openapi_license_fields[] = {
    {"name",
     "name",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_License, name),
     sizeof(char *),
     0,
     0,
     1,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"identifier",
     "identifier",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_License, identifier),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"url",
     "url",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_License, url),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info OpenAPI_License_type_info = {
    "OpenAPI_License",
    sizeof(struct OpenAPI_License),
    0,
    g_openapi_license_fields,
    sizeof(g_openapi_license_fields) / sizeof(g_openapi_license_fields[0]),
    0};

/** @brief Reflected fields for `OpenAPI_Info`. */
static const struct cdd_field_info g_openapi_info_fields[] = {
    {"title",
     "title",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_Info, title),
     sizeof(char *),
     0,
     0,
     1,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"summary",
     "summary",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_Info, summary),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"description",
     "description",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_Info, description),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"terms_of_service",
     "termsOfService",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_Info, terms_of_service),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"version",
     "version",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_Info, version),
     sizeof(char *),
     0,
     0,
     1,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info OpenAPI_Info_type_info = {
    "OpenAPI_Info",
    sizeof(struct OpenAPI_Info),
    0,
    g_openapi_info_fields,
    sizeof(g_openapi_info_fields) / sizeof(g_openapi_info_fields[0]),
    0};

/** @brief Reflected fields for `OpenAPI_ExternalDocs`. */
static const struct cdd_field_info g_openapi_external_docs_fields[] = {
    {"description",
     "description",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_ExternalDocs, description),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"url",
     "url",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_ExternalDocs, url),
     sizeof(char *),
     0,
     0,
     1,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info OpenAPI_ExternalDocs_type_info = {
    "OpenAPI_ExternalDocs",
    sizeof(struct OpenAPI_ExternalDocs),
    0,
    g_openapi_external_docs_fields,
    sizeof(g_openapi_external_docs_fields) /
        sizeof(g_openapi_external_docs_fields[0]),
    0};

/** @brief Reflected fields for `OpenAPI_Tag`. */
static const struct cdd_field_info g_openapi_tag_fields[] = {
    {"summary",
     "summary",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_Tag, summary),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"description",
     "description",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_Tag, description),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"parent",
     "parent",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_Tag, parent),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"kind",
     "kind",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_Tag, kind),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info OpenAPI_Tag_type_info = {
    "OpenAPI_Tag",
    sizeof(struct OpenAPI_Tag),
    0,
    g_openapi_tag_fields,
    sizeof(g_openapi_tag_fields) / sizeof(g_openapi_tag_fields[0]),
    0};

/** @brief Reflected fields for `OpenAPI_ServerVariable`. */
static const struct cdd_field_info g_openapi_server_variable_fields[] = {
    {"default_value",
     "default",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_ServerVariable, default_value),
     sizeof(char *),
     0,
     0,
     1,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"description",
     "description",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_ServerVariable, description),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info OpenAPI_ServerVariable_type_info = {
    "OpenAPI_ServerVariable",
    sizeof(struct OpenAPI_ServerVariable),
    0,
    g_openapi_server_variable_fields,
    sizeof(g_openapi_server_variable_fields) /
        sizeof(g_openapi_server_variable_fields[0]),
    0};

/** @brief Reflected fields for `OpenAPI_Server`. */
static const struct cdd_field_info g_openapi_server_fields[] = {
    {"description",
     "description",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_Server, description),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"name",
     "name",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_Server, name),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info OpenAPI_Server_type_info = {
    "OpenAPI_Server",
    sizeof(struct OpenAPI_Server),
    0,
    g_openapi_server_fields,
    sizeof(g_openapi_server_fields) / sizeof(g_openapi_server_fields[0]),
    0};

/** @brief Reflected fields for `OpenAPI_Xml`. */
static const struct cdd_field_info g_openapi_xml_fields[] = {
    {"name",
     "name",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_Xml, name),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"namespace_uri",
     "namespace",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_Xml, namespace_uri),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"prefix",
     "prefix",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_Xml, prefix),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info OpenAPI_Xml_type_info = {
    "OpenAPI_Xml",
    sizeof(struct OpenAPI_Xml),
    0,
    g_openapi_xml_fields,
    sizeof(g_openapi_xml_fields) / sizeof(g_openapi_xml_fields[0]),
    0};

/** @brief Reflected fields for `OpenAPI_Discriminator`. */
static const struct cdd_field_info g_openapi_discriminator_fields[] = {
    {"property_name",
     "propertyName",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_Discriminator, property_name),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"default_mapping",
     "defaultMapping",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_Discriminator, default_mapping),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info OpenAPI_Discriminator_type_info = {
    "OpenAPI_Discriminator",
    sizeof(struct OpenAPI_Discriminator),
    0,
    g_openapi_discriminator_fields,
    sizeof(g_openapi_discriminator_fields) /
        sizeof(g_openapi_discriminator_fields[0]),
    0};

/** @brief Reflected fields for `OpenAPI_OAuthFlow`. */
static const struct cdd_field_info g_openapi_oauth_flow_fields[] = {
    {"authorization_url",
     "authorizationUrl",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_OAuthFlow, authorization_url),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"token_url",
     "tokenUrl",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_OAuthFlow, token_url),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"refresh_url",
     "refreshUrl",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_OAuthFlow, refresh_url),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"device_authorization_url",
     "deviceAuthorizationUrl",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_OAuthFlow, device_authorization_url),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info OpenAPI_OAuthFlow_type_info = {
    "OpenAPI_OAuthFlow",
    sizeof(struct OpenAPI_OAuthFlow),
    0,
    g_openapi_oauth_flow_fields,
    sizeof(g_openapi_oauth_flow_fields) /
        sizeof(g_openapi_oauth_flow_fields[0]),
    0};

/** @brief Reflected fields for `OpenAPI_Example`. */
static const struct cdd_field_info g_openapi_example_fields[] = {
    {"summary",
     "summary",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_Example, summary),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"description",
     "description",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_Example, description),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"serialized_value",
     "serializedValue",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_Example, serialized_value),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"external_value",
     "externalValue",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_Example, external_value),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info OpenAPI_Example_type_info = {
    "OpenAPI_Example",
    sizeof(struct OpenAPI_Example),
    0,
    g_openapi_example_fields,
    sizeof(g_openapi_example_fields) / sizeof(g_openapi_example_fields[0]),
    0};

/** @brief Reflected fields for `OpenAPI_Link`. */
static const struct cdd_field_info g_openapi_link_fields[] = {
    {"operation_ref",
     "operationRef",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_Link, operation_ref),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"operation_id",
     "operationId",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_Link, operation_id),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"description",
     "description",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_Link, description),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"summary",
     "summary",
     "char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct OpenAPI_Link, summary),
     sizeof(char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info OpenAPI_Link_type_info = {
    "OpenAPI_Link",
    sizeof(struct OpenAPI_Link),
    0,
    g_openapi_link_fields,
    sizeof(g_openapi_link_fields) / sizeof(g_openapi_link_fields[0]),
    0};

/**
 * @brief Table-driven helper to populate reflected `char *` string fields from
 * a `JSON_Object`.
 *
 * @param[in] obj Source JSON object.
 * @param[in] ti Runtime reflection descriptor.
 * @param[in,out] target Target C struct pointer.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on allocation
 * failure.
 */
cdd_c_error_t openapi_parse_reflected_strings(const JSON_Object *obj,
                                              const struct cdd_type_info *ti,
                                              void *target) {
  size_t i;
  for (i = 0; i < ti->n_fields; ++i) {
    const struct cdd_field_info *f = &ti->fields[i];
    const char *val = NULL;
    cdd_c_error_t rc = cdd_json_object_get_string(obj, f->wire_name, &val);
    if (val) {
      char *s_out = NULL;
      char *prev = NULL;
      memcpy(&prev, (unsigned char *)target + f->offset, sizeof(prev));
      if (prev)
        C_CDD_FREE(prev);
      rc = c_cdd_strdup(val, &s_out);
      if (rc != CDD_C_SUCCESS)
        return CDD_C_ERROR_MEMORY;
      memcpy((unsigned char *)target + f->offset, &s_out, sizeof(s_out));
    }
  }
  return CDD_C_SUCCESS;
}

cdd_c_error_t
OpenAPI_Contact_from_jsonObject(const JSON_Object *contact_obj,
                                struct OpenAPI_Contact *out_contact) {
  cdd_c_error_t rc = openapi_parse_reflected_strings(
      contact_obj, &OpenAPI_Contact_type_info, out_contact);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return collect_extensions(contact_obj, &out_contact->extensions_json);
}

cdd_c_error_t
OpenAPI_License_from_jsonObject(const JSON_Object *license_obj,
                                struct OpenAPI_License *out_license) {
  const char *lic_name = NULL;
  const char *lic_identifier = NULL;
  const char *lic_url = NULL;
  cdd_c_error_t rc;
  rc = cdd_json_object_get_string(license_obj, "name", &lic_name);
  rc = cdd_json_object_get_string(license_obj, "identifier", &lic_identifier);
  rc = cdd_json_object_get_string(license_obj, "url", &lic_url);

  if (!lic_name || !*lic_name)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (lic_identifier && lic_url)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = openapi_parse_reflected_strings(license_obj, &OpenAPI_License_type_info,
                                       out_license);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return collect_extensions(license_obj, &out_license->extensions_json);
}

cdd_c_error_t OpenAPI_Info_from_jsonObject(const JSON_Object *info_obj,
                                           struct OpenAPI_Info *out_info) {
  JSON_Object *contact_obj = NULL;
  JSON_Object *license_obj = NULL;
  cdd_c_error_t rc;
  rc = openapi_parse_reflected_strings(info_obj, &OpenAPI_Info_type_info,
                                       out_info);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = collect_extensions(info_obj, &out_info->extensions_json);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = cdd_json_object_get_object(info_obj, "contact", &contact_obj);
  if (contact_obj) {
    rc = OpenAPI_Contact_from_jsonObject(contact_obj, &out_info->contact);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  rc = cdd_json_object_get_object(info_obj, "license", &license_obj);
  if (license_obj) {
    rc = OpenAPI_License_from_jsonObject(license_obj, &out_info->license);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Internal helper to parse the OpenAPI Info object.
 *
 * @param[in] root_obj The root OpenAPI JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t parse_info_inner(const JSON_Object *root_obj,
                                      struct OpenAPI_Spec *out) {
  JSON_Object *info_obj = NULL;

  if (!root_obj || !out)
    return CDD_C_SUCCESS;

  cdd_json_object_get_object(root_obj, "info", &info_obj);
  if (!info_obj)
    return CDD_C_SUCCESS;

  return OpenAPI_Info_from_jsonObject(info_obj, &out->info);
}

/**
 * @brief Parses info from the given input.
 *
 * @param[in] root_obj The root OpenAPI JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t parse_info(const JSON_Object *root_obj,
                         struct OpenAPI_Spec *out) {
  cdd_c_error_t rc = parse_info_inner(root_obj, out);
  if (rc != CDD_C_SUCCESS)
    openapi_spec_free(out);
  return rc;
}

/**
 * @brief Internal helper to parse an ExternalDocs object.
 *
 * @param[in] obj The source externalDocs JSON object.
 * @param[out] out Output OpenAPI_ExternalDocs structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t
parse_external_docs_inner(const JSON_Object *obj,
                          struct OpenAPI_ExternalDocs *out) {
  const char *url = NULL;
  cdd_c_error_t rc;

  if (!obj || !out)
    return CDD_C_SUCCESS;

  rc = cdd_json_object_get_string(obj, "url", &url);
  if (!url || !*url)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = openapi_parse_reflected_strings(obj, &OpenAPI_ExternalDocs_type_info,
                                       out);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = collect_extensions(obj, &out->extensions_json);
  if (rc != CDD_C_SUCCESS)
    return rc;

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses external docs from the given input.
 *
 * @param[in] obj The source externalDocs JSON object.
 * @param[out] out Output OpenAPI_ExternalDocs structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t parse_external_docs(const JSON_Object *obj,
                                  struct OpenAPI_ExternalDocs *out) {
  cdd_c_error_t rc = parse_external_docs_inner(obj, out);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(out->url);
    C_CDD_FREE(out->description);
    C_CDD_FREE(out->extensions_json);
    memset(out, 0, sizeof(*out));
  }
  return rc;
}

/**
 * @brief Internal helper to parse a Discriminator object.
 *
 * @param[in] obj The source discriminator JSON object.
 * @param[out] out Output OpenAPI_Discriminator structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t
parse_discriminator_object_inner(const JSON_Object *obj,
                                 struct OpenAPI_Discriminator *out) {
  JSON_Object *mapping_obj = NULL;
  size_t i, count, used;
  cdd_c_error_t rc;

  if (!obj || !out)
    return CDD_C_SUCCESS;

  rc = openapi_parse_reflected_strings(obj, &OpenAPI_Discriminator_type_info,
                                       out);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = cdd_json_object_get_object(obj, "mapping", &mapping_obj);
  if (mapping_obj) {
    count = 0;
    cdd_json_object_get_count(mapping_obj, &count);
    used = 0;
    for (i = 0; i < count; ++i) {
      const char *name = NULL;
      cdd_json_object_get_name(mapping_obj, i, &name);
      if (strncmp(name, "x-", 2) == 0)
        continue;
      used++;
    }
    if (used > 0) {
      out->mapping = (struct OpenAPI_DiscriminatorMap *)C_CDD_CALLOC(
          used, sizeof(struct OpenAPI_DiscriminatorMap));
      if (!out->mapping)
        return CDD_C_ERROR_MEMORY;
      out->n_mapping = used;
      used = 0;
      for (i = 0; i < count; ++i) {
        const char *name = NULL;
        const char *val = NULL;
        cdd_json_object_get_name(mapping_obj, i, &name);
        if (strncmp(name, "x-", 2) == 0)
          continue;
        rc = cdd_json_object_get_string(mapping_obj, name, &val);
        if (!val)
          continue;
        rc = c_cdd_strdup(name, &out->mapping[used].value);
        if (rc != CDD_C_SUCCESS)
          return CDD_C_ERROR_MEMORY;
        rc = c_cdd_strdup(val, &out->mapping[used].schema);
        if (rc != CDD_C_SUCCESS)
          return CDD_C_ERROR_MEMORY;
        used++;
      }
      out->n_mapping = used;
    }
  }

  rc = collect_extensions(obj, &out->extensions_json);
  if (rc != CDD_C_SUCCESS)
    return rc;

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses discriminator object from the given input.
 *
 * @param[in] obj The source discriminator JSON object.
 * @param[out] out Output OpenAPI_Discriminator structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t parse_discriminator_object(const JSON_Object *obj,
                                         struct OpenAPI_Discriminator *out) {
  cdd_c_error_t rc = parse_discriminator_object_inner(obj, out);
  if (rc != CDD_C_SUCCESS) {
    size_t i;
    C_CDD_FREE(out->property_name);
    C_CDD_FREE(out->default_mapping);
    C_CDD_FREE(out->extensions_json);
    if (out->mapping) {
      for (i = 0; i < out->n_mapping; ++i) {
        C_CDD_FREE(out->mapping[i].value);
        C_CDD_FREE(out->mapping[i].schema);
      }
      C_CDD_FREE(out->mapping);
    }
    memset(out, 0, sizeof(*out));
  }
  return rc;
}

/**
 * @brief Internal helper to parse an XML metadata object.
 *
 * @param[in] obj The source XML JSON object.
 * @param[out] out Output OpenAPI_Xml structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t parse_xml_object_inner(const JSON_Object *obj,
                                            struct OpenAPI_Xml *out) {
  const char *node_type = NULL;
  int has_attr = 0, has_wrap = 0;
  int attr_val = -1, wrap_val = -1;
  cdd_c_error_t rc;

  if (!obj || !out)
    return CDD_C_SUCCESS;

  rc = cdd_json_object_get_string(obj, "nodeType", &node_type);
  if (node_type) {
    rc = parse_xml_node_type(node_type, &out->node_type);
    out->node_type_set = 1;
  }

  rc = openapi_parse_reflected_strings(obj, &OpenAPI_Xml_type_info, out);
  if (rc != CDD_C_SUCCESS)
    return rc;

  cdd_json_object_has_value(obj, "attribute", &has_attr);
  if (has_attr) {
    out->attribute_set = 1;
    cdd_json_object_get_boolean(obj, "attribute", &attr_val);
    out->attribute = (attr_val == 1);
  }

  cdd_json_object_has_value(obj, "wrapped", &has_wrap);
  if (has_wrap) {
    out->wrapped_set = 1;
    cdd_json_object_get_boolean(obj, "wrapped", &wrap_val);
    out->wrapped = (wrap_val == 1);
  }

  rc = collect_extensions(obj, &out->extensions_json);
  if (rc != CDD_C_SUCCESS)
    return rc;

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses xml object from the given input.
 *
 * @param[in] obj The source XML JSON object.
 * @param[out] out Output OpenAPI_Xml structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t parse_xml_object(const JSON_Object *obj,
                               struct OpenAPI_Xml *out) {
  cdd_c_error_t rc = parse_xml_object_inner(obj, out);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(out->name);
    C_CDD_FREE(out->namespace_uri);
    C_CDD_FREE(out->prefix);
    C_CDD_FREE(out->extensions_json);
    memset(out, 0, sizeof(*out));
  }
  return rc;
}

/**
 * @brief Internal helper to parse top-level OpenAPI tags.
 *
 * @param[in] root_obj The root OpenAPI JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t parse_tags_inner(const JSON_Object *root_obj,
                                      struct OpenAPI_Spec *out) {
  JSON_Array *tags_arr = NULL;
  size_t count = 0, i;

  if (!root_obj || !out)
    return CDD_C_SUCCESS;

  cdd_json_object_get_array(root_obj, "tags", &tags_arr);
  if (!tags_arr)
    return CDD_C_SUCCESS;

  cdd_json_array_get_count(tags_arr, &count);
  if (count == 0)
    return CDD_C_SUCCESS;

  out->tags =
      (struct OpenAPI_Tag *)C_CDD_CALLOC(count, sizeof(struct OpenAPI_Tag));
  if (!out->tags)
    return CDD_C_ERROR_MEMORY;
  out->n_tags = count;

  for (i = 0; i < count; ++i) {
    JSON_Object *tag_obj = NULL;
    const char *name = NULL;
    JSON_Object *ext = NULL;
    cdd_c_error_t rc;

    cdd_json_array_get_object(tags_arr, i, &tag_obj);
    cdd_json_object_get_string(tag_obj, "name", &name);
    cdd_json_object_get_object(tag_obj, "externalDocs", &ext);

    if (!name || !*name)
      return CDD_C_ERROR_INVALID_ARGUMENT;

    {
      size_t k;
      for (k = 0; k < i; ++k) {
        if (strcmp(out->tags[k].name, name) == 0)
          return CDD_C_ERROR_INVALID_ARGUMENT;
      }
    }
    rc = c_cdd_strdup(name, &out->tags[i].name);
    if (rc != CDD_C_SUCCESS)
      return CDD_C_ERROR_MEMORY;
    rc = openapi_parse_reflected_strings(tag_obj, &OpenAPI_Tag_type_info,
                                         &out->tags[i]);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (ext) {
      rc = parse_external_docs(ext, &out->tags[i].external_docs);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    rc = collect_extensions(tag_obj, &out->tags[i].extensions_json);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses tags from the given input.
 *
 * @param[in] root_obj The root OpenAPI JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t parse_tags(const JSON_Object *root_obj,
                         struct OpenAPI_Spec *out) {
  cdd_c_error_t rc = parse_tags_inner(root_obj, out);
  if (rc != CDD_C_SUCCESS)
    openapi_spec_free(out);
  return rc;
}

/**
 * @brief Finds the index of a Tag by name in `spec->tags`.
 *
 * @param[in] spec OpenAPI specification.
 * @param[in] name Tag name to search for.
 * @param[out] out_idx Index of the tag.
 * @return CDD_C_SUCCESS on success, or error code if not found/invalid arg.
 */
cdd_c_error_t tag_index_by_name(const struct OpenAPI_Spec *spec,
                                const char *name, size_t *out_idx) {
  size_t i;
  if (!spec || !name || !out_idx)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  for (i = 0; i < spec->n_tags; ++i) {
    if (strcmp(spec->tags[i].name, name) == 0) {
      *out_idx = i;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_ERROR_NOT_FOUND;
}

/**
 * @brief Detects parent cycles in `spec->tags` starting from `idx`.
 *
 * @param[in] spec OpenAPI specification.
 * @param[in] idx Starting tag index.
 * @param[in,out] state Visited state array (`0` unvisited, `1` visiting, `2`
 * done).
 * @param[out] out_has_cycle Output 1 if cycle detected, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if
 * out_has_cycle is NULL.
 */
cdd_c_error_t detect_tag_cycle(const struct OpenAPI_Spec *spec, size_t idx,
                               int *state, int *out_has_cycle) {
  size_t parent_idx = 0;
  const char *parent;
  if (!out_has_cycle)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_has_cycle = 0;
  if (!spec || !state || idx >= spec->n_tags)
    return CDD_C_SUCCESS;
  if (state[idx] == 1) {
    *out_has_cycle = 1;
    return CDD_C_SUCCESS;
  }
  if (state[idx] == 2)
    return CDD_C_SUCCESS;
  state[idx] = 1;
  parent = spec->tags[idx].parent;
  if (parent && *parent) {
    if (tag_index_by_name(spec, parent, &parent_idx) == CDD_C_SUCCESS) {
      int sub_cycle = 0;
      detect_tag_cycle(spec, parent_idx, state, &sub_cycle);
      if (sub_cycle) {
        *out_has_cycle = 1;
        return CDD_C_SUCCESS;
      }
    }
  }
  state[idx] = 2;
  return CDD_C_SUCCESS;
}

/**
 * @brief Validates that no tag parent cycles exist in `spec->tags`.
 *
 * @param[in] spec OpenAPI specification.
 * @return CDD_C_SUCCESS if valid, or error code on cycle/OOM.
 */
cdd_c_error_t validate_tag_parents(const struct OpenAPI_Spec *spec) {
  size_t i;
  int *state;
  if (!spec || !spec->tags || spec->n_tags == 0)
    return CDD_C_SUCCESS;

  state = (int *)C_CDD_CALLOC(spec->n_tags, sizeof(int));
  if (!state)
    return CDD_C_ERROR_MEMORY;
  for (i = 0; i < spec->n_tags; ++i) {
    int has_cycle = 0;
    detect_tag_cycle(spec, i, state, &has_cycle);
    if (has_cycle) {
      C_CDD_FREE(state);
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
  }
  C_CDD_FREE(state);
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a server variable name is defined in `srv->variables`.
 *
 * @param[in] srv Server structure.
 * @param[in] name Variable name to check.
 * @param[out] out_defined Output 1 if defined, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if
 * out_defined is NULL.
 */
cdd_c_error_t server_variable_defined(const struct OpenAPI_Server *srv,
                                      const char *name, int *out_defined) {
  size_t i;
  if (!out_defined)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_defined = 0;
  if (!srv || !name || !srv->variables)
    return CDD_C_SUCCESS;
  for (i = 0; i < srv->n_variables; ++i) {
    if (strcmp(srv->variables[i].name, name) == 0) {
      *out_defined = 1;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a server variable name has already been seen in a list.
 *
 * @param[in] seen Array of seen variable names.
 * @param[in] seen_count Number of elements in `seen`.
 * @param[in] name Variable name to check.
 * @param[out] out_seen Output 1 if seen, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if
 * out_seen is NULL.
 */
cdd_c_error_t server_variable_seen(char **seen, size_t seen_count,
                                   const char *name, int *out_seen) {
  size_t i;
  if (!out_seen)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_seen = 0;
  if (!seen || !name)
    return CDD_C_SUCCESS;
  for (i = 0; i < seen_count; ++i) {
    if (strcmp(seen[i], name) == 0) {
      *out_seen = 1;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Validates `{var}` placeholders in a Server URL against its `variables`
 * map.
 *
 * @param[in] srv Server structure to validate.
 * @return CDD_C_SUCCESS if valid, or error code on mismatch/OOM.
 */
cdd_c_error_t validate_server_url_variables(const struct OpenAPI_Server *srv) {
  const char *url;
  size_t i;
  char **seen = NULL;
  size_t seen_count = 0;
  size_t seen_cap = 0;

  if (!srv || !srv->url)
    return CDD_C_SUCCESS;

  url = srv->url;
  for (i = 0; url[i]; ++i) {
    if (url[i] == '{') {
      const char *end = strchr(url + i + 1, '}');
      size_t len;
      char *name;
      char **tmp;
      int is_def = 0;
      int is_seen = 0;
      if (!end)
        goto invalid;
      len = (size_t)(end - (url + i + 1));
      if (len == 0)
        goto invalid;
      name = (char *)C_CDD_MALLOC(len + 1);
      if (!name)
        goto oom;
      memcpy(name, url + i + 1, len);
      name[len] = '\0';
      server_variable_defined(srv, name, &is_def);
      if (!is_def) {
        C_CDD_FREE(name);
        goto invalid;
      }
      server_variable_seen(seen, seen_count, name, &is_seen);
      if (is_seen) {
        C_CDD_FREE(name);
        goto invalid;
      }
      if (seen_count == seen_cap) {
        size_t new_cap = seen_cap ? seen_cap * 2 : 4;
        tmp = (char **)C_CDD_REALLOC(seen, new_cap * sizeof(char *));
        if (!tmp) {
          C_CDD_FREE(name);
          goto oom;
        }
        seen = tmp;
        seen_cap = new_cap;
      }
      seen[seen_count++] = name;
      i = (size_t)(end - url);
      continue;
    }
    if (url[i] == '}')
      goto invalid;
  }

  for (i = 0; i < seen_count; ++i)
    C_CDD_FREE(seen[i]);
  C_CDD_FREE(seen);
  return CDD_C_SUCCESS;

invalid:
  for (i = 0; i < seen_count; ++i)
    C_CDD_FREE(seen[i]);
  C_CDD_FREE(seen);
  return CDD_C_ERROR_INVALID_ARGUMENT;

oom:
  for (i = 0; i < seen_count; ++i)
    C_CDD_FREE(seen[i]);
  C_CDD_FREE(seen);
  return CDD_C_ERROR_MEMORY;
}

/**
 * @brief Internal helper to parse an OpenAPI Server object.
 *
 * @param[in] srv_obj The source server JSON object.
 * @param[out] out_srv Output OpenAPI_Server structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t parse_server_object_inner(const JSON_Object *srv_obj,
                                               struct OpenAPI_Server *out_srv) {
  const char *url = NULL;
  JSON_Object *vars = NULL;
  cdd_c_error_t rc;

  if (!srv_obj || !out_srv)
    return CDD_C_SUCCESS;

  rc = cdd_json_object_get_string(srv_obj, "url", &url);
  rc = cdd_json_object_get_object(srv_obj, "variables", &vars);

  if (!url || !*url)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (url_has_query_or_fragment(url))
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = c_cdd_strdup(url, &out_srv->url);
  if (rc != CDD_C_SUCCESS)
    return CDD_C_ERROR_MEMORY;

  rc = openapi_parse_reflected_strings(srv_obj, &OpenAPI_Server_type_info,
                                       out_srv);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = collect_extensions(srv_obj, &out_srv->extensions_json);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (vars) {
    size_t vcount = 0;
    size_t v;
    cdd_json_object_get_count(vars, &vcount);
    if (vcount > 0) {
      out_srv->variables = (struct OpenAPI_ServerVariable *)C_CDD_CALLOC(
          vcount, sizeof(struct OpenAPI_ServerVariable));
      if (!out_srv->variables)
        return CDD_C_ERROR_MEMORY;
      out_srv->n_variables = vcount;
      for (v = 0; v < vcount; ++v) {
        const char *vname = NULL;
        JSON_Value *v_val = NULL;
        JSON_Object *v_obj = NULL;
        struct OpenAPI_ServerVariable *curr = &out_srv->variables[v];
        cdd_json_object_get_name(vars, v, &vname);
        cdd_json_object_get_value_at(vars, v, &v_val);
        cdd_json_value_get_object(v_val, &v_obj);
        rc = c_cdd_strdup(vname, &curr->name);
        if (rc != CDD_C_SUCCESS)
          return CDD_C_ERROR_MEMORY;
        if (v_obj) {
          const char *def_val = NULL;
          JSON_Array *enum_arr = NULL;
          cdd_json_object_get_string(v_obj, "default", &def_val);
          cdd_json_object_get_array(v_obj, "enum", &enum_arr);
          if (!def_val || !*def_val)
            return CDD_C_ERROR_INVALID_ARGUMENT;
          rc = openapi_parse_reflected_strings(
              v_obj, &OpenAPI_ServerVariable_type_info, curr);
          if (rc != CDD_C_SUCCESS)
            return rc;
          if (enum_arr) {
            size_t ecount = 0;
            size_t e;
            int found_default = 0;
            cdd_json_array_get_count(enum_arr, &ecount);
            if (ecount == 0)
              return CDD_C_ERROR_INVALID_ARGUMENT;
            {
              curr->enum_values = (char **)C_CDD_CALLOC(ecount, sizeof(char *));
              if (!curr->enum_values)
                return CDD_C_ERROR_MEMORY;
              curr->n_enum_values = ecount;
              for (e = 0; e < ecount; ++e) {
                const char *e_val = NULL;
                cdd_json_array_get_string(enum_arr, e, &e_val);
                if (e_val) {
                  rc = c_cdd_strdup(e_val, &curr->enum_values[e]);
                  if (rc != CDD_C_SUCCESS)
                    return CDD_C_ERROR_MEMORY;
                  if (strcmp(e_val, def_val) == 0)
                    found_default = 1;
                }
              }
              if (!found_default)
                return CDD_C_ERROR_INVALID_ARGUMENT;
            }
          }
          rc = collect_extensions(v_obj, &curr->extensions_json);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      }
    }
  }

  rc = validate_server_url_variables(out_srv);
  if (rc != CDD_C_SUCCESS)
    return rc;

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses server object from the given input.
 *
 * @param[in] srv_obj The source server JSON object.
 * @param[out] out_srv Output OpenAPI_Server structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t parse_server_object(const JSON_Object *srv_obj,
                                  struct OpenAPI_Server *out_srv) {
  cdd_c_error_t rc = parse_server_object_inner(srv_obj, out_srv);
  if (rc != CDD_C_SUCCESS) {
    size_t v, e;
    C_CDD_FREE(out_srv->url);
    C_CDD_FREE(out_srv->description);
    C_CDD_FREE(out_srv->name);
    C_CDD_FREE(out_srv->extensions_json);
    if (out_srv->variables) {
      for (v = 0; v < out_srv->n_variables; ++v) {
        struct OpenAPI_ServerVariable *var = &out_srv->variables[v];
        C_CDD_FREE(var->name);
        C_CDD_FREE(var->default_value);
        C_CDD_FREE(var->description);
        C_CDD_FREE(var->extensions_json);
        if (var->enum_values) {
          for (e = 0; e < var->n_enum_values; ++e)
            C_CDD_FREE(var->enum_values[e]);
          C_CDD_FREE(var->enum_values);
        }
      }
      C_CDD_FREE(out_srv->variables);
    }
    memset(out_srv, 0, sizeof(*out_srv));
  }
  return rc;
}

/**
 * @brief Internal helper to parse an array of OpenAPI Server objects.
 *
 * @param[in] parent The parent JSON object.
 * @param[in] key The servers array key name.
 * @param[out] out_servers Output array of OpenAPI_Server structures.
 * @param[out] out_count Output count of servers.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t
parse_servers_array_inner(const JSON_Object *parent, const char *key,
                          struct OpenAPI_Server **out_servers,
                          size_t *out_count) {
  JSON_Array *servers = NULL;
  size_t count = 0, i;

  if (!parent || !key || !out_servers || !out_count)
    return CDD_C_SUCCESS;

  *out_servers = NULL;
  *out_count = 0;

  cdd_json_object_get_array(parent, key, &servers);
  if (!servers)
    return CDD_C_SUCCESS;

  cdd_json_array_get_count(servers, &count);
  if (count == 0)
    return CDD_C_SUCCESS;

  *out_servers = (struct OpenAPI_Server *)C_CDD_CALLOC(
      count, sizeof(struct OpenAPI_Server));
  if (!*out_servers)
    return CDD_C_ERROR_MEMORY;
  *out_count = count;

  for (i = 0; i < count; ++i) {
    JSON_Object *srv_obj = NULL;
    cdd_json_array_get_object(servers, i, &srv_obj);
    if (srv_obj) {
      cdd_c_error_t rc = parse_server_object(srv_obj, &(*out_servers)[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  {
    size_t j;
    for (i = 0; i < count; ++i) {
      const char *name_i = (*out_servers)[i].name;
      if (!name_i || !*name_i)
        continue;
      for (j = i + 1; j < count; ++j) {
        const char *name_j = (*out_servers)[j].name;
        if (!name_j || !*name_j)
          continue;
        if (strcmp(name_i, name_j) == 0)
          return CDD_C_ERROR_INVALID_ARGUMENT;
      }
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses servers array from the given input.
 *
 * @param[in] parent The parent JSON object.
 * @param[in] key The servers array key name.
 * @param[out] out_servers Output array of OpenAPI_Server structures.
 * @param[out] out_count Output count of servers.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t parse_servers_array(const JSON_Object *parent, const char *key,
                                  struct OpenAPI_Server **out_servers,
                                  size_t *out_count) {
  cdd_c_error_t rc =
      parse_servers_array_inner(parent, key, out_servers, out_count);
  if (rc != CDD_C_SUCCESS && *out_servers) {
    openapi_free_servers_array(*out_servers, *out_count);
    *out_servers = NULL;
    *out_count = 0;
  }
  return rc;
}
