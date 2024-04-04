# Cypherock_X1_Wallet
The X1Wallet ships with a minimal funtionality (allowing for device authentication and [setup process](#device-setup-flow)) and later updates to a fully-functional firmware ([go through feature list](#functionality-of-x1wallet)) via the desktop application which communicates with the secure servers.

### Responsible disclosure policy
At Cypherock, we believe that coordinated vulnerability disclosure is the right approach to better protect users. Please go through the [bug bounty](https://www.cypherock.com/bug-bounty) page on the Cypherock website to learn about the process to be followed when submitting a vulnerability report. Please report suspected vulnerabilities in private to bounty@cypherock.com.

## Device setup flow
The X1Wallet ships with a firmware to take the user through the device setup flow. The device setup flow includes following steps:
- Some introductory instructions and information about basic operations of the device (such as operating the joystick to navigate, tap cards with device NFC).
- Next comes the setup screen where user is supposed to open the Desktop application to continue the setup process of a new device. The further process involves following operations:
  - The device is verified with the Cypherock servers for its authenticity.
  - Upon successful device verification, next comes the card verification process. The desktop application performs the verification of cards with the servers for their authenticity as well. This process happens one-by-one for each of the four cards (each card needs to be verified individually).
  - Upon successful verification, each card is then paired with the device individually. The pairing between a card and the devices establishes a mutual authentication and encryption schemes between the components. This ensures that any data exchange happens securely via NFC eliminating any risks of NFC spoofing.

Website link for getting started with new device: [cypherock.com/gs](https://www.cypherock.com/get-started/)

## Functionality of X1Wallet
After the device is setup ([steps to setup device](#device-setup-flow)), the X1Wallet is ready to use by the end-users. To use the functionality, the user must have access to the X1Wallet and CyCards; cySync is required for on-chain operations such as send operations and balance viewing.

### List of features:
#### 1. Create wallet (on CyCards)
This is the most secure way (and recommended option) for users to create and own a wallet. If this method is used for creating a wallet for storing funds, users can rest assured that none of the private (or citical) information leaves the secure operating environment of the X1Wallet ever (provided that the user does not knowingly/unknowingly exposes the private information).

The X1Wallet can create a wallet from scratch and set it up for the user. The entire process of creating a wallet is as follows:
   - From the main menu, choose "Create wallet" -> "Generate new wallet"
   - In next steps, user is asked for basic wallet configurations (name, PIN, passphrase, etc.)
   - After processing, user is expected to go through list of mnemonics and cross-verify with 3 words.
   - Upon success, the wallet is supposed to saved on CyCards (**NOTE: User will need all the 4 CyCards to successfully complete the step.**)
   - As the final step of wallet creation, user is asked to tap all the CyCards one-by-one (___this is a mandatory step before the new wallet is ready to use___). Once the data on CyCards is verified to be correct, the wallet is ready for normal use.

#### 2. Restore 3rd party (or external) wallet (to CyCards)
This is not a recommended method to own a wallet on X1Wallet because it is possible that the menmonics are already spoofed.

X1Wallet supports the feature to import any BIP39 compliant wallets that have been created at other places (such as Hot wallets like MEW/Metamask, Cold wallets like Coldcard, etc.). The supported method of import is via BIP39 mnemonic word list of the wallet that is to be imported. ***X1Wallet allows importing a mnemonic with word list count of 12, 18 & 24.*** The complete process of restoring a wallet is as follows:
   - From the main menu, choose "Create wallet" -> "Generate new wallet"
   - In next steps, user is asked for basic wallet configurations (name, PIN, passphrase, etc.)
   - Now, user is expected to enter the list of words on-by-one and go through the entered word-list.
   - Upon success, the wallet is supposed to saved on CyCards (**NOTE: User will need all the 4 CyCards to successfully complete the step.**)
   - As the final step of wallet creation, user is asked to tap all the CyCards one-by-one (___this is a mandatory step before the new wallet is ready to use___). Once the data on CyCards is verified to be correct, the wallet is ready for normal use.

#### 3. View wallet word-list/seed (of existing wallets)
A user or owner of X1Wallet has an option to view the critical information of their wallet (the mnemonic word list). Since, the word list is a very sensitive piece of information, it is important that the end user handles it accordingly. With the wordlist, user has the flexibility to export their wallet to a new X1Wallet (or other platforms if the need be; it is not recommended to use the wallet on any other platform for users benefit).

To view the word-list, a user needs to have one CyCard (and knowledge of PIN, if the wallet is PIN protected).

#### 4. Import wallet to cySync (Desktop application)
Since, the X1Wallet is a cold storage, it cannot directly interact with Blockchain on the internet to view the balances. To facilitate this, the desktop application, enables users to view their account balances and funds on their wallet/accounts. In order to enable the tracking of funds for any particular wallet (for a particular type of crypto), users have to sync their X1Wallet with cySync and then [enable coin support](#5-enable-coin-support-for-a-wallet-on-cysync-desktop-application) of that particular cryptocurrency. To sync a wallet with cySync, user will trigger the request via cySync and confirm the request on the X1Wallet.

***NOTE: This process only sends the public information about the wallet. No private critical information leaves the X1Wallet.***

#### 5. Enable coin support for a wallet on cySync (Desktop application)
X1Wallet allows tracking a cryptocurrency balances linked to a users wallet. This is possible via cySync interface. To enable a coin tracking support, the user will require to have one CyCard with the wallet. To enable coin tracking support (make sure that the desired wallet is [synced with cySync](#4-import-wallet-to-cysync-desktop-application)), user will trigger the request via cySync and confirm the request on the X1Wallet.

***NOTE: This process only sends the public information about the accounts for the selected crypto. No private critical information leaves the X1Wallet.***

#### 6. Receive funds into wallet address present on CyCards
To receive funds into their X1Wallet, first the user must have at least one active wallet on their X1Wallet device ([create a wallet](#1-create-wallet-on-cycards) if you do not have one). If you already have a wallet, then make sure to have access to one CyCard for receiving funds. Now make sure the [wallet is synced with cySync](#4-import-wallet-to-cysync-desktop-application) and [coin tracking support](#5-enable-coin-support-for-a-wallet-on-cysync-desktop-application) is enabled for the preferred coin.

Trigger the request to generate a receiving public address for your wallet. ***It is recommended to verify the address on the X1Wallet and never skip this step for security reasons.***

#### 7. Send funds from wallet present on CyCards
To enable transfer of funds from user's wallet to a different wallet, following things are necessary:
   - Correct knowledge of receiver's public address for that particular crypto
   - Access to one CyCard
   - Wallets with sufficient funds ([create a wallet](#1-create-wallet-on-cycards) if you do not have one and [receive funds](#6-receive-funds-into-wallet-address-present-on-cycards) before proceeding)
   - Knowledge of PIN in case of PIN protected wallets
   - Wallet synced with cySync ([sync wallet with cySync](#4-import-wallet-to-cysync-desktop-application)) and coin tracking support enabled on cySync ([enabled coin tracking support](#5-enable-coin-support-for-a-wallet-on-cysync-desktop-application))

The operation can be started from cySync and confirm the request on X1Wallet to continue. The user is expected to verify the transaction details such as receiver's address and amount being transferred along with a required fee used by the network for processing the transaction. If any descrepancy is observed by the user, the transaction should not be signed and cancelled immediately.

***NOTE: The transaction signing starts once the user has tapped the CyCard. It is recommended to not tap the CyCard if any descrepancy is observed by the user.***

## Building project

Refer the [steps to setup](./HACKING.md) build environment and how to perform build steps.

# Documents
### [Device authentication](docs/device_provision_auth.md)
### [Shamir secret sharing](docs/shamir_secret_sharing.md)
### [USB communication](docs/usb_communication.md)
### [Secure NFC communication](docs/nfc_secure_communication.md)
### [CyLock (Proof of Work)](docs/cylock__proof_of_work.md)
### [Bootloader Doc(with Memory map)](docs/bootloader.md)
