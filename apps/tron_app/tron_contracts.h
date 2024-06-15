#include <tron_txn_helpers.h>

typedef struct trc20_contracts {
  /// 20-byte hex coded public address of the contract
  const uint8_t address[TRON_INITIAL_ADDRESS_LENGTH];
  /// Symbol (short alphabetical representation) of the contract token
  const char *symbol;
  /// Decimal value used to display the amount in token transfer in token units
  const uint8_t decimal;
} trc20_contracts_t;

/// Number of entries in whitelisted contracts list
#define TRC20_WHITELISTED_CONTRACTS_COUNT 9
#define TRC20_FUNCTION_SELECTOR_COUNT 1