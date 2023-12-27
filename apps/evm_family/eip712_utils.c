#include "eip712_utils.h"

#include "evm_api.h"

queue_node *new_queue_node(evm_sign_typed_data_node_t *tree_node,
                           const char *prefix) {
  queue_node *temp = (queue_node *)cy_malloc(sizeof(queue_node));
  temp->tree_node = tree_node;
  temp->prefix = cy_malloc(strlen(prefix) + 1);
  strcpy(temp->prefix, prefix);
  temp->next = NULL;
  return temp;
}

queue *create_queue() {
  queue *q = (queue *)cy_malloc(sizeof(queue));
  q->front = q->rear = NULL;
  q->count = 0;
  return q;
}

int is_empty(queue *q) {
  return (q->count == 0);
}

void enqueue(queue *q,
             evm_sign_typed_data_node_t *tree_node,
             const char *prefix) {
  queue_node *temp = new_queue_node(tree_node, prefix);
  q->count++;
  if (q->rear == NULL) {
    q->front = q->rear = temp;
    return;
  }
  q->rear->next = temp;
  q->rear = temp;
}

queue_node *dequeue(queue *q) {
  if (is_empty(q))
    return NULL;
  queue_node *temp = q->front;
  q->front = q->front->next;
  if (q->front == NULL)
    q->rear = NULL;
  q->count--;
  return temp;
}

static void twos_complement_of_byte_array(uint8_t *arr, size_t size) {
  size_t i;

  // flip all the bits
  for (i = 0; i < size; i++)
    arr[i] = ~arr[i];

  // add 1 to the least significant bit
  for (i = size - 1; i >= 0; i--) {
    if (arr[i] == 255) {
      arr[i] = 0;
    } else {
      arr[i]++;
      break;
    }
  }
}

void fill_string_with_data(const evm_sign_typed_data_node_t *data_node,
                           char *output,
                           const size_t output_size) {
  memzero(output, output_size);
  char buffer[BUFFER_SIZE] = {0};
  switch (data_node->type) {
    case EVM_EIP_712_DATA_TYPE_ARRAY:
    case EVM_EIP_712_DATA_TYPE_STRUCT:
      snprintf(
          buffer, sizeof(buffer), "Contains %ld elements", data_node->size);
      break;
    case EVM_EIP_712_DATA_TYPE_UINT: {
      char hex_string[65] = {0};
      byte_array_to_hex_string(data_node->data->bytes,
                               data_node->size,
                               hex_string,
                               sizeof(hex_string));
      convert_byte_array_to_decimal_string(
          data_node->size * 2, 0, hex_string, buffer, sizeof(buffer));
    } break;
    case EVM_EIP_712_DATA_TYPE_INT: {
      char hex_string[65] = {0};
      uint8_t array[32] = {0};
      uint8_t offset = 0;
      memcpy(array, data_node->data->bytes, sizeof(array));

      // if signed integer get 2's complement
      if (array[0] & 0x80) {
        twos_complement_of_byte_array(array, data_node->size);
        snprintf(buffer, sizeof(buffer), "-");
        offset++;
      }

      byte_array_to_hex_string(
          array, data_node->size, hex_string, sizeof(hex_string));
      convert_byte_array_to_decimal_string(data_node->size * 2,
                                           0,
                                           hex_string,
                                           buffer + offset,
                                           sizeof(buffer) - offset);
    } break;
    case EVM_EIP_712_DATA_TYPE_BOOL:
      if (data_node->data->bytes[0] == 1)
        snprintf(buffer, sizeof(buffer), "true");
      else
        snprintf(buffer, sizeof(buffer), "false");
      break;
    case EVM_EIP_712_DATA_TYPE_STRING:
      snprintf(buffer, data_node->data->size + 1, "%s", data_node->data->bytes);
      break;
    case EVM_EIP_712_DATA_TYPE_BYTES:
    case EVM_EIP_712_DATA_TYPE_ADDRESS:
    default:
      snprintf(buffer, sizeof(buffer), "0x");
      byte_array_to_hex_string(data_node->data->bytes,
                               data_node->data->size,
                               buffer + 2,
                               sizeof(buffer) - 1);
      break;
  }
  snprintf(output, output_size, "%s: %s", data_node->struct_name, buffer);
}

int encode_data(const evm_sign_typed_data_node_t *data_node,
                uint8_t *output,
                const size_t output_size,
                size_t *bytes_written) {
  if (output_size < HASH_SIZE)
    return EIP712_MEMORY_LIMIT_EXCEEDED;
  switch (data_node->type) {
    {
      case EVM_EIP_712_DATA_TYPE_UINT: {
        uint8_t abi_status = Abi_Encode(
            Abi_uint256_e, data_node->size, data_node->data->bytes, output);
        if (abi_status != ABI_PROCESS_COMPLETE) {
          LOG_ERROR("ABI:%d", abi_status);
          printf("ABI:%d", abi_status);
          return EIP712_ABI_ENCODING_FAILED;
        }
        *bytes_written += HASH_SIZE;
      } break;
      case EVM_EIP_712_DATA_TYPE_INT: {
        uint8_t abi_status = Abi_Encode(
            Abi_int256_e, data_node->size, data_node->data->bytes, output);
        if (abi_status != ABI_PROCESS_COMPLETE) {
          LOG_ERROR("ABI:%d", abi_status);
          printf("ABI:%d", abi_status);
          return EIP712_ABI_ENCODING_FAILED;
        }
        *bytes_written += HASH_SIZE;
      } break;
      case EVM_EIP_712_DATA_TYPE_BOOL: {
        uint8_t abi_status = Abi_Encode(
            Abi_bool_e, data_node->size, data_node->data->bytes, output);
        if (abi_status != ABI_PROCESS_COMPLETE) {
          LOG_ERROR("ABI:%d", abi_status);
          printf("ABI:%d", abi_status);
          return EIP712_ABI_ENCODING_FAILED;
        }
        *bytes_written += HASH_SIZE;
      } break;
      case EVM_EIP_712_DATA_TYPE_ADDRESS: {
        uint8_t abi_status = Abi_Encode(
            Abi_address_e, data_node->size, data_node->data->bytes, output);
        if (abi_status != ABI_PROCESS_COMPLETE) {
          LOG_ERROR("ABI:%d", abi_status);
          printf("ABI:%d", abi_status);
          return EIP712_ABI_ENCODING_FAILED;
        }
        *bytes_written += HASH_SIZE;
      } break;
      case EVM_EIP_712_DATA_TYPE_BYTES: {
        if (strncmp(data_node->struct_name,
                    DYNAMIC_BYTES_ID,
                    sizeof(DYNAMIC_BYTES_ID)) != 0) {
          uint8_t abi_status = Abi_Encode(
              Abi_bytes_e, data_node->size, data_node->data->bytes, output);
          if (abi_status != ABI_PROCESS_COMPLETE) {
            LOG_ERROR("ABI:%d", abi_status);
            printf("ABI:%d", abi_status);
            return EIP712_ABI_ENCODING_FAILED;
          }
          *bytes_written += HASH_SIZE;
          break;
        }
      }
      case EVM_EIP_712_DATA_TYPE_STRING:
        keccak_256(data_node->data->bytes, data_node->size, output);
        *bytes_written += HASH_SIZE;
        break;
      case EVM_EIP_712_DATA_TYPE_ARRAY: {
        size_t result_size = data_node->children_count * HASH_SIZE;
        uint8_t *result = malloc(result_size);
        if (result == NULL)
          return EIP712_MEMORY_ALLOCATION_FAILED;
        memzero(result, result_size);
        int status = EIP712_OK;
        size_t dummy = 0;
        for (int i = 0; i < data_node->children_count; i++) {
          if (data_node->children[i].type == EVM_EIP_712_DATA_TYPE_STRUCT)
            status =
                hash_struct(&data_node->children[i], result + i * HASH_SIZE);
          else
            status = encode_data(&data_node->children[i],
                                 result + i * HASH_SIZE,
                                 HASH_SIZE,
                                 &dummy);
          if (status != EIP712_OK) {
            free(result);
            return status;
          }
        }
        keccak_256(result, result_size, output);
        *bytes_written += HASH_SIZE;
        free(result);
      } break;
      case EVM_EIP_712_DATA_TYPE_STRUCT: {
        size_t result_size = data_node->children_count * HASH_SIZE;
        if (result_size > output_size)
          return EIP712_MEMORY_LIMIT_EXCEEDED;
        uint8_t *result = malloc(result_size);
        if (result == NULL)
          return EIP712_MEMORY_ALLOCATION_FAILED;
        memzero(result, result_size);
        int status = EIP712_OK;
        size_t dummy = 0;
        for (int i = 0; i < data_node->children_count; i++) {
          if (data_node->children[i].type == EVM_EIP_712_DATA_TYPE_STRUCT)
            status =
                hash_struct(&data_node->children[i], result + i * HASH_SIZE);
          else
            status = encode_data(&data_node->children[i],
                                 result + i * HASH_SIZE,
                                 HASH_SIZE,
                                 &dummy);

          if (status != EIP712_OK) {
            free(result);
            return status;
          }
        }
        memcpy(output, result, result_size);
        *bytes_written += result_size;
        free(result);
      } break;

      default:
        break;
    }
  }

  return EIP712_OK;
}

int hash_struct(const evm_sign_typed_data_node_t *data_node, uint8_t *output) {
  if (data_node->type != EVM_EIP_712_DATA_TYPE_STRUCT)
    return EIP712_INVALID_DATA;
  size_t data_size = HASH_SIZE + (data_node->children_count) * HASH_SIZE;
  size_t used_size = 0;
  uint8_t *data = malloc(data_size);
  memcpy(data, data_node->type_hash->bytes, data_node->type_hash->size);
  int status = encode_data(data_node,
                           data + data_node->type_hash->size,
                           (data_size - data_node->type_hash->size),
                           &used_size);
  keccak_256(data, data_size, output);
  free(data);
  return status;
}
