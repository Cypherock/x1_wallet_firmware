/**
 * @file eip712_validate.c
 * @brief Validation of host supplied EIP-712 typed data before it is shown or
 * signed.
 */

#include "eip712_validate.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sha3.h"

#define EIP712_TYPE_HASH_SIZE 32
/// Distinct struct types accepted in one typed data payload
#define EIP712_MAX_STRUCT_TYPES 32
/// Longest accepted type name (struct names, "uint256", "Order[10]", ...)
#define EIP712_MAX_TYPE_NAME 128
#define EIP712_DOMAIN_TYPE "EIP712Domain"
/// marks a dependency list whose definitions are not all in the payload
#define EIP712_DEPS_UNKNOWN SIZE_MAX

typedef struct {
  const evm_sign_typed_data_node_t *types[EIP712_MAX_STRUCT_TYPES];
  size_t count;
} eip712_registry_t;

static bool parse_size_suffix(const char *digits, uint32_t *value) {
  if ('\0' == *digits) {
    return false;
  }
  uint32_t result = 0;
  for (const char *p = digits; *p != '\0'; p++) {
    if (*p < '0' || *p > '9' || result > 1000) {
      return false;
    }
    result = result * 10 + (uint32_t)(*p - '0');
  }
  *value = result;
  return true;
}

/// "uint8".."uint256" / "int8".."int256" with the given byte width
static bool is_int_type_name(const char *name,
                             const char *prefix,
                             uint32_t byte_width) {
  size_t prefix_len = strlen(prefix);
  uint32_t bits = 0;
  if (0 != strncmp(name, prefix, prefix_len) ||
      !parse_size_suffix(name + prefix_len, &bits)) {
    return false;
  }
  if (0 == bits || 256 < bits || 0 != bits % 8) {
    return false;
  }
  return 0 == byte_width || bits / 8 == byte_width;
}

/// "bytes1".."bytes32" with the given byte width
static bool is_fixed_bytes_type_name(const char *name, uint32_t byte_width) {
  uint32_t size = 0;
  if (0 != strncmp(name, "bytes", 5) || !parse_size_suffix(name + 5, &size)) {
    return false;
  }
  if (0 == size || 32 < size) {
    return false;
  }
  return 0 == byte_width || size == byte_width;
}

static bool is_primitive_type_name(const char *name) {
  return 0 == strcmp(name, "address") || 0 == strcmp(name, "bool") ||
         0 == strcmp(name, "string") || 0 == strcmp(name, "bytes") ||
         is_fixed_bytes_type_name(name, 0) ||
         is_int_type_name(name, "uint", 0) || is_int_type_name(name, "int", 0);
}

/**
 * Splits "Base[]" or "Base[N]" into Base and N. Nested arrays are rejected.
 */
static bool parse_array_type_name(const char *name,
                                  char *base,
                                  size_t base_size,
                                  bool *is_fixed,
                                  uint32_t *fixed_len) {
  size_t len = strnlen(name, EIP712_MAX_TYPE_NAME);
  if (len < 3 || len >= EIP712_MAX_TYPE_NAME || ']' != name[len - 1]) {
    return false;
  }
  const char *open = strchr(name, '[');
  if (NULL == open || open == name || open != strrchr(name, '[')) {
    return false;
  }
  size_t base_len = (size_t)(open - name);
  if (base_len >= base_size) {
    return false;
  }
  memcpy(base, name, base_len);
  base[base_len] = '\0';

  char digits[16] = {0};
  size_t digits_len = len - base_len - 2;
  if (digits_len >= sizeof(digits)) {
    return false;
  }
  memcpy(digits, open + 1, digits_len);
  *is_fixed = 0 < digits_len;
  *fixed_len = 0;
  return !*is_fixed || parse_size_suffix(digits, fixed_len);
}

/// Base type of a field type name ("Person[]" -> "Person")
static bool get_base_type_name(const char *name, char *base, size_t size) {
  const char *open = strchr(name, '[');
  size_t len = NULL == open ? strnlen(name, size) : (size_t)(open - name);
  if (0 == len || len >= size) {
    return false;
  }
  memcpy(base, name, len);
  base[len] = '\0';
  return true;
}

static bool validate_leaf(const evm_sign_typed_data_node_t *node) {
  if (NULL == node->data || 0 != node->children_count) {
    return false;
  }
  const char *type_name = node->struct_name;
  const pb_size_t data_size = node->data->size;
  switch (node->type) {
    case EVM_EIP_712_DATA_TYPE_UINT:
      return data_size == node->size && 0 < node->size && 32 >= node->size &&
             is_int_type_name(type_name, "uint", node->size);
    case EVM_EIP_712_DATA_TYPE_INT:
      return data_size == node->size && 0 < node->size && 32 >= node->size &&
             is_int_type_name(type_name, "int", node->size);
    case EVM_EIP_712_DATA_TYPE_BOOL:
      // the host currently sends bool data empty; it is shown and signed as
      // false (see eip712_bool_value)
      return 1 == node->size && 1 >= data_size &&
             (0 == data_size || 1 >= node->data->bytes[0]) &&
             0 == strcmp(type_name, "bool");
    case EVM_EIP_712_DATA_TYPE_ADDRESS:
      return data_size == node->size && 20 == node->size &&
             0 == strcmp(type_name, "address");
    case EVM_EIP_712_DATA_TYPE_STRING:
      // size is not checked: the host sets it to the character count, which
      // differs from the utf-8 byte count; the data bytes are shown and hashed
      return 0 == strcmp(type_name, "string");
    case EVM_EIP_712_DATA_TYPE_BYTES: {
      if (data_size != node->size) {
        return false;
      }
      if (0 == strcmp(type_name, "bytes")) {
        return true;
      }
      // bytesN may be given shorter than N; it is right padded with zeros
      uint32_t declared = 0;
      return 0 < node->size && 0 == strncmp(type_name, "bytes", 5) &&
             parse_size_suffix(type_name + 5, &declared) && 0 < declared &&
             32 >= declared && node->size <= declared;
    }
    default:
      return false;
  }
}

static bool validate_node(const evm_sign_typed_data_node_t *node) {
  if (NULL == node->name || NULL == node->struct_name ||
      EIP712_MAX_TYPE_NAME <=
          strnlen(node->struct_name, EIP712_MAX_TYPE_NAME)) {
    return false;
  }

  switch (node->type) {
    case EVM_EIP_712_DATA_TYPE_STRUCT: {
      if (is_primitive_type_name(node->struct_name) ||
          NULL != strchr(node->struct_name, '[') || NULL == node->type_hash ||
          EIP712_TYPE_HASH_SIZE != node->type_hash->size ||
          node->size != node->children_count) {
        return false;
      }
      for (pb_size_t i = 0; i < node->children_count; i++) {
        if (!validate_node(&node->children[i])) {
          return false;
        }
      }
      return true;
    }

    case EVM_EIP_712_DATA_TYPE_ARRAY: {
      // the host currently sends arrays of primitives (and nested arrays)
      // without elements; they are hashed as empty arrays as before
      if (0 == node->children_count) {
        return NULL != strchr(node->struct_name, '[');
      }
      char base[EIP712_MAX_TYPE_NAME] = {0};
      bool is_fixed = false;
      uint32_t fixed_len = 0;
      if (!parse_array_type_name(
              node->struct_name, base, sizeof(base), &is_fixed, &fixed_len) ||
          node->size != node->children_count ||
          (is_fixed && fixed_len != node->size)) {
        return false;
      }
      bool base_is_struct = !is_primitive_type_name(base);
      for (pb_size_t i = 0; i < node->children_count; i++) {
        const evm_sign_typed_data_node_t *child = &node->children[i];
        if (!validate_node(child) || NULL == child->struct_name ||
            0 != strcmp(child->struct_name, base) ||
            base_is_struct != (EVM_EIP_712_DATA_TYPE_STRUCT == child->type)) {
          return false;
        }
      }
      return true;
    }

    default:
      return validate_leaf(node);
  }
}

static bool same_definition(const evm_sign_typed_data_node_t *a,
                            const evm_sign_typed_data_node_t *b) {
  if (a->children_count != b->children_count) {
    return false;
  }
  for (pb_size_t i = 0; i < a->children_count; i++) {
    if (0 != strcmp(a->children[i].name, b->children[i].name) ||
        0 != strcmp(a->children[i].struct_name, b->children[i].struct_name)) {
      return false;
    }
  }
  return true;
}

static const evm_sign_typed_data_node_t *find_type(
    const eip712_registry_t *registry,
    const char *name) {
  for (size_t i = 0; i < registry->count; i++) {
    if (0 == strcmp(registry->types[i]->struct_name, name)) {
      return registry->types[i];
    }
  }
  return NULL;
}

/**
 * Records one definition per struct type name; every instance of a type must
 * declare the same fields.
 */
static bool collect_types(const evm_sign_typed_data_node_t *node,
                          eip712_registry_t *registry) {
  if (EVM_EIP_712_DATA_TYPE_STRUCT == node->type) {
    const evm_sign_typed_data_node_t *known =
        find_type(registry, node->struct_name);
    if (NULL == known) {
      if (EIP712_MAX_STRUCT_TYPES <= registry->count) {
        return false;
      }
      registry->types[registry->count++] = node;
    } else if (!same_definition(known, node)) {
      return false;
    }
  }
  for (pb_size_t i = 0; i < node->children_count; i++) {
    if (!collect_types(&node->children[i], registry)) {
      return false;
    }
  }
  return true;
}

static bool is_listed(const char **list, size_t count, const char *name) {
  for (size_t i = 0; i < count; i++) {
    if (0 == strcmp(list[i], name)) {
      return true;
    }
  }
  return false;
}

/// Struct types referenced (directly or not) by def, excluding the primary
static bool collect_dependencies(const eip712_registry_t *registry,
                                 const evm_sign_typed_data_node_t *def,
                                 const char **deps,
                                 size_t *deps_count) {
  char base[EIP712_MAX_TYPE_NAME] = {0};
  const char *primary = def->struct_name;
  *deps_count = 0;

  // worklist: def first, then every dependency found so far
  for (size_t next = 0; NULL != def; next++) {
    for (pb_size_t i = 0; i < def->children_count; i++) {
      if (!get_base_type_name(
              def->children[i].struct_name, base, sizeof(base))) {
        return false;
      }
      if (is_primitive_type_name(base) || 0 == strcmp(base, primary) ||
          is_listed(deps, *deps_count, base)) {
        continue;
      }
      const evm_sign_typed_data_node_t *dep = find_type(registry, base);
      if (NULL == dep) {
        // only referenced through an empty array: the payload carries no
        // definition to compute the type hash from
        *deps_count = EIP712_DEPS_UNKNOWN;
        return true;
      }
      if (EIP712_MAX_STRUCT_TYPES <= *deps_count) {
        return false;
      }
      deps[(*deps_count)++] = dep->struct_name;
    }
    def = next < *deps_count ? find_type(registry, deps[next]) : NULL;
  }
  return true;
}

static void hash_type_definition(SHA3_CTX *ctx,
                                 const evm_sign_typed_data_node_t *def) {
  keccak_Update(
      ctx, (const uint8_t *)def->struct_name, strlen(def->struct_name));
  keccak_Update(ctx, (const uint8_t *)"(", 1);
  for (pb_size_t i = 0; i < def->children_count; i++) {
    if (0 < i) {
      keccak_Update(ctx, (const uint8_t *)",", 1);
    }
    const evm_sign_typed_data_node_t *field = &def->children[i];
    keccak_Update(
        ctx, (const uint8_t *)field->struct_name, strlen(field->struct_name));
    keccak_Update(ctx, (const uint8_t *)" ", 1);
    keccak_Update(ctx, (const uint8_t *)field->name, strlen(field->name));
  }
  keccak_Update(ctx, (const uint8_t *)")", 1);
}

/// typeHash = keccak256(encodeType(struct)) as defined by EIP-712
static bool check_type_hash(const eip712_registry_t *registry,
                            const evm_sign_typed_data_node_t *node) {
  const char *deps[EIP712_MAX_STRUCT_TYPES] = {0};
  size_t deps_count = 0;
  if (!collect_dependencies(registry, node, deps, &deps_count)) {
    return false;
  }
  if (EIP712_DEPS_UNKNOWN == deps_count) {
    // TODO: requires the host to send every struct definition; until then the
    // host's type hash is used for such structs
    return true;
  }

  // dependencies are sorted by name after the primary type
  for (size_t i = 1; i < deps_count; i++) {
    const char *key = deps[i];
    size_t j = i;
    while (0 < j && 0 < strcmp(deps[j - 1], key)) {
      deps[j] = deps[j - 1];
      j--;
    }
    deps[j] = key;
  }

  SHA3_CTX ctx = {0};
  uint8_t computed[EIP712_TYPE_HASH_SIZE] = {0};
  keccak_256_Init(&ctx);
  hash_type_definition(&ctx, node);
  for (size_t i = 0; i < deps_count; i++) {
    hash_type_definition(&ctx, find_type(registry, deps[i]));
  }
  keccak_Final(&ctx, computed);

  return 0 == memcmp(computed, node->type_hash->bytes, EIP712_TYPE_HASH_SIZE);
}

static bool check_type_hashes(const eip712_registry_t *registry,
                              const evm_sign_typed_data_node_t *node) {
  if (EVM_EIP_712_DATA_TYPE_STRUCT == node->type &&
      !check_type_hash(registry, node)) {
    return false;
  }
  for (pb_size_t i = 0; i < node->children_count; i++) {
    if (!check_type_hashes(registry, &node->children[i])) {
      return false;
    }
  }
  return true;
}

uint8_t eip712_bool_value(const evm_sign_typed_data_node_t *node) {
  return (NULL != node->data && 0 < node->data->size) ? node->data->bytes[0]
                                                      : 0;
}

bool eip712_validate_typed_data(
    const evm_sign_typed_data_struct_t *typed_data) {
  if (NULL == typed_data) {
    return false;
  }
  const evm_sign_typed_data_node_t *domain = &typed_data->domain;
  const evm_sign_typed_data_node_t *message = &typed_data->message;

  // the domain is always labelled "EIP712Domain" on screen
  if (EVM_EIP_712_DATA_TYPE_STRUCT != domain->type ||
      EVM_EIP_712_DATA_TYPE_STRUCT != message->type ||
      NULL == domain->struct_name ||
      0 != strcmp(domain->struct_name, EIP712_DOMAIN_TYPE) ||
      NULL == message->struct_name || !validate_node(domain)) {
    return false;
  }

  // primaryType "EIP712Domain": the message is an empty placeholder that is
  // not part of the digest (see evm_get_typed_struct_data_digest)
  bool message_hashed =
      !(0 == strcmp(message->struct_name, EIP712_DOMAIN_TYPE) &&
        0 == message->children_count && 0 == message->size);
  if (message_hashed && !validate_node(message)) {
    return false;
  }

  eip712_registry_t *registry = calloc(1, sizeof(eip712_registry_t));
  if (NULL == registry) {
    return false;
  }
  bool result = collect_types(domain, registry) &&
                (!message_hashed || collect_types(message, registry)) &&
                check_type_hashes(registry, domain) &&
                (!message_hashed || check_type_hashes(registry, message));
  free(registry);
  return result;
}
