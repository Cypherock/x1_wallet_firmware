# Cardano

Purpose: 1852'

CoinType: 1815'

Account: Hardened

Change
  * 0 - External Payment
  * 2 - Stake

Index:
  * n - External Payment
  * 0 - Stake

Payment derivation path:
```
root / 1852' / 1815' /0' / 0 / i
```

Stake derivation path:
```
root / 1852' / 1815' /0' / 2 / 0
```

# Explorer
## Mainnet Explorer
https://explorer.cardano.org

## Testnet Explorer
https://preview.cardanoscan.io/

https://preprod.cardanoscan.io/

# Faucet
https://docs.cardano.org/cardano-testnets/tools/faucet

https://sancho.network/faucet

# Setting local cardano node via Docker:

1. Pull image
```sh
docker pull ghcr.io/intersectmbo/cardano-node:latest
```

2. Make data and ip directory
```sh
mkdir -p ./{data,ipc}

# this make sures our host machine has enough permissions
chown -R $(id -u):$(id -g) data ipc
```

3. Fetch testnet config
```sh
docker run -d \
  --name cardano-node \
  -e NETWORK=preview \
  -v $PWD/data:/data \
  -v $PWD/ipc:/ipc \
  ghcr.io/intersectmbo/cardano-node:latest
```

NOTE: most of the operations done in cardano are via the cli

Setup and usage instructions for the cli are given here:

[https://developers.cardano.org/docs/get-started/infrastructure/cardano-cli/basic-operations/get-started](https://developers.cardano.org/docs/get-started/infrastructure/cardano-cli/basic-operations/get-started)

You might also need to do the following in order for it to work
```sh
export CARDANO_NODE_SOCKET_PATH=$PWD/ipc/node.socket

# for preview
export CARDANO_TESTNET_MAGIC=2
# for preprod
export CARDANO_TESTNET_MAGIC=1

```

# Key Derivation

## References
[https://developers.cardano.org/docs/learn/core-concepts/addresses](https://developers.cardano.org/docs/learn/core-concepts/addresses)

[https://www.youtube.com/watch?v=NjPf_b9UQNs](https://www.youtube.com/watch?v=NjPf_b9UQNs)

[https://developers.cardano.org/docs/get-started/infrastructure/cardano-cli/basic-operations/get-started#generating-keys-and-addresses](https://developers.cardano.org/docs/get-started/infrastructure/cardano-cli/basic-operations/get-started#generating-keys-and-addresses)



Cardano has two different derivation path for two different kind of keys.

1. Payment Key

derivation path:
```
root / 1852' / 1815' /0' / 0 / i
```

This is used to then compute the receive address, and the private key is used to sign transactions.

2. Stake Key

derivation path:
```
root / 1852' / 1815' /0' / 2 / 0
```

This is never used for signing transactions, it is only used for delegations and rewards.

## Key derivation in firmware

In the firmware, it expects the host to only ever send the payment key derivation path, the firmware then computes the stake derivation path. see `stake_derv_from_payment` in `cardano_helpers.c`. And then continues computing stake public key (and address) and payment public key (and address) normally.

When derivation for multiple keys is performed, the firmware assumes that the `account` parameter does not changes in all those derivation paths. This allows firmware to cache the stake derivation and reuse it for all the given keys.

## Address from public keys

### Stake address
```
raw_stake = 0xe0 | network_id + blake2b_hash(stake_public_key)
network_id can be:
   0xe0 = mainnet
   0xf0 = testnet

raw_stake_5bit = convert_to_5bit_symbols(raw_stake)

stake_addr = bech32_encode(raw_stake_5bit, prefix="stake")
```

see `get_stake_addr` in `cardano_helpers.c`

### Payment address

```
raw_payment = blake2b_hash(payment_public_key)
raw_stake = blake2b_hash(stake_public_key)
raw_full = HEADER + raw_payment + raw_stake

HEADER can be:
    - 0x01 - mainnet
    - 0x00 - testnet

raw_full_5bit = convert_to_5bit_symbols(raw_full)
payment_addr = bech32_encode(raw_full_5bit)
```

see `get_payment_addr` in `cardano_helpers.c`


# Transaction Signing

## References
[https://developers.cardano.org/docs/learn/core-concepts/transactions](https://developers.cardano.org/docs/learn/core-concepts/transactions)

[https://www.youtube.com/watch?v=OSNf1MgAbII](https://www.youtube.com/watch?v=OSNf1MgAbII)

[https://developers.cardano.org/docs/get-started/infrastructure/cardano-cli/basic-operations/simple-transactions](https://developers.cardano.org/docs/get-started/infrastructure/cardano-cli/basic-operations/simple-transactions)


## Transaction signing in firmware

All transaction in cardano are cbor encoded.

Transaction structure when decoded:
```
transaction = [
  transaction_body,      // Core transaction data
  transaction_witness_set, // Signatures and scripts
  validity_flag,         // Transaction validity
  auxiliary_data         // Metadata (optional)
]
```
see [https://developers.cardano.org/docs/learn/core-concepts/transactions#cbor-and-cardano-specifications](https://developers.cardano.org/docs/learn/core-concepts/transactions#cbor-and-cardano-specifications)

Among the fields, only transaction_body is signed. It also contains the fields we need to verify from the user when signing on the device.

NOTE: The device expects only `transaction_body` to be sent for signing, it will decode that, extract required fields and then sign it. see `cardano_parse_and_hash_txn_from_cbor` in `cardano_txn.c`

NOTE: Amount and fees in the transaction body is in `lovelace` which can be converted to ada:
```
1 lovelace = 1 * 1e-6 ada
```
see `ada_from_lovelace` in `cardano_helpers.c`

## Signing process


`transaction_body` contains the transaction body in cbor encoded form (canoncial).
```
hashed_transaction = blake2b_hash(transaction_body)
ed25519_sign(hashed_transaction)
```

see `sign_txn` in `cardano_txn.c`
