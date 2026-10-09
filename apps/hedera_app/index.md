### Technical Breakdown of the `hedera_app` Implementation

This application enables the Cypherock X1 wallet to securely manage a user's Hedera (HBAR) account by performing two primary cryptographic functions: exporting a public key and signing a transaction.

#### 1. Core Architecture & App Registration
*   **Entry Point (`hedera_main.c`)**:
    *   Defines the application descriptor `hedera_app_desc` which includes a unique App ID (e.g., 24) and the version. This descriptor is registered with the main firmware controller in `src/controller_main.c`.
    *   Implements `hedera_main()`, which acts as the central router for all incoming requests for the Hedera app. It decodes the top-level `hedera_query_t` message to determine the requested action (e.g., `get_public_keys`, `sign_txn`) and calls the appropriate handler.

*   **Communication Layer (`hedera_api.c`, `hedera_api.h`)**:
    *   Provides a standardized interface for handling Protobuf-based communication with the host.
    *   **Encoding/Decoding**: Wraps `nanopb`'s `pb_encode` and `pb_decode` functions. `decode_hedera_query` safely parses incoming APDU data into C structs, while `hedera_send_result` serializes C structs back into a byte stream to send to the host.
    *   **Error Handling**: Implements a consistent error-reporting mechanism (`hedera_send_error`) that sends a `common_error_t` message back to the host, ensuring the host knows precisely why an operation failed.

#### 2. Cryptographic Operations & Key Management
*   **Key Derivation (`hedera_pub_key.c`, `hedera_txn.c`)**:
    *   **Algorithm**: Utilizes the **Ed25519** signature scheme, which is Hedera's standard. This is specified in calls to `derive_hdnode_from_path`.
    *   **Derivation Path**: Strictly enforces the BIP-44 path `m/44'/3030'/0'/0'/i'` in `hedera_helpers.c` via `hedera_derivation_path_guard()`. This prevents the wallet from signing with keys from non-standard paths, which is a critical security measure. The `3030'` component is Hedera's registered coin type.
    *   **Seed Management**: The app is stateless regarding secrets. It never stores the seed. For every cryptographic operation, it calls the core firmware function `reconstruct_seed()` to temporarily reconstruct the master seed in RAM, uses it, and then immediately erases it by calling `memzero()`.

#### 3. Public Key Export (`hedera_pub_key.c`)
*   **Function**: Implements the `hedera_get_pub_keys` flow.
*   **Process**:
    1.  Receives a wallet ID and one or more derivation paths from the host.
    2.  Prompts the user for consent to export a public key for the specified wallet.
    3.  Calls `reconstruct_seed()` to get the master seed.
    4.  For each path, it calls `derive_hdnode_from_path()` to derive the corresponding Ed25519 key pair.
    5.  The **raw 32-byte public key** is extracted.
    6.  **User Verification (WYSIWYS)**: If a "user-verified" export is requested, the raw 32-byte public key is converted into a **64-character hexadecimal string** (`hedera_format_pubkey`) and displayed on the device screen for the user to verify. This is crucial because Hedera account IDs (`0.0.123`) are not cryptographically derived from the key.
    7.  The raw 32-byte public key(s) are sent back to the host.

#### 4. Transaction Signing (`hedera_txn.c`)
*   **Function**: Implements the `hedera_sign_transaction` flow, the app's most critical feature.
*   **Process**:
    1.  **Initiation**: Receives a wallet ID and a derivation path. It prompts the user to confirm they want to start a signing process for that wallet.
    2.  **Data Reception**: Receives the **raw, serialized Protobuf `TransactionBody`** from the host. This byte array is stored in `hedera_txn_context->raw_txn_bytes`.
    3.  **Parsing for Display**: It uses `pb_decode` and the `nanopb`-generated `Hedera_TransactionBody_fields` descriptor to parse `raw_txn_bytes` into the `hedera_txn_context->txn` C struct. This decoding is **only for display and verification purposes**.
    4.  **User Verification (WYSIWYS)**: This is the core security step.
        *   It iterates through the decoded `txn` struct.
        *   It displays critical information on the screen for user approval, one piece at a time:
            *   **Operator Account ID**: The account paying the fee (`txn.transactionID.accountID`).
            *   **Transaction Type**: Determined by `txn.which_data`. The code specifically handles `cryptoTransfer`.
            *   **Transfers**: For a crypto transfer, it loops through the `accountAmounts` list, displaying each recipient's account ID and the amount.
            *   **Amount Formatting**: Calls `hedera_format_tinybars_to_hbar_string` to convert the integer `amount` (in tinybars) into a human-readable HBAR string (e.g., "12.345 HBAR").
            *   **Transaction Fee**: Formats and displays `txn.transactionFee`.
            *   **Memo**: Displays the `txn.memo` if it exists.
    5.  **Cryptographic Signing**:
        *   Upon user approval, it calls `reconstruct_seed()` and `derive_hdnode_from_path()` to get the required Ed25519 private key.
        *   It calls `ed25519_sign()` from the device's crypto library.
        *   **Crucially, it signs the original `raw_txn_bytes` received in step 2.** It does NOT sign a hash.
        *   The result is a **64-byte raw signature**.
    6.  **Response**: The raw 64-byte signature is sent back to the host.

#### 5. Helper Utilities (`hedera_helpers.c`, `hedera_context.h`)
*   **Context**: Defines all Hedera-specific constants (`HEDERA_NAME`, coin type, key sizes) in one place for easy maintenance.
*   **Formatting**: Provides functions to convert raw data types (64-bit integers for amounts, Protobuf structs for account IDs, byte arrays for keys) into user-friendly strings for the UI. This isolates display logic from cryptographic logic.
*   **Validation**: Includes the `hedera_derivation_path_guard` to ensure that the app only ever operates on valid, standard derivation paths, preventing many types of attacks.
