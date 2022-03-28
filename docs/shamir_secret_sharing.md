# Shamir Secret Sharing

SSS theory
----------

The X1 Wallet implementation of SSS follows the original concepts and thus essentially is a secret sharing algorithm that operates on each byte of original data (treated as secret in the algorithm) and splits it into N number of shares of each byte.

The sharing scheme that is used in X1 Wallet is (2, 5)-threshold scheme or 2-of-5 sharing scheme, wherein, any two of the shares from the 5 generated shares can be used to retrieve the original data (or secret).

  

Following good practices, the implementation uses GF(256) for its ubiquity (which is also recommended in most of the standards for secret sharing) and lagrange’s polynomial for computational improvements.

### Interpolate

Given a set of m points (xi, yi), 1 ≤ i ≤ m, such that no two xi values equal, there exists a polynomial that assumes the value yi at each point xi. The polynomial of lowest degree that satisfies these conditions is uniquely determined and can be obtained using the Lagrange interpolation formula given below.

  

Since Shamir's secret sharing scheme is applied separately to each of the n bytes of the shared secret, we work with yi as a vector of n values, where yi\[k\] = fk(xi), 1 ≤ k ≤ n, and fk is the polynomial in the k-th instance of the scheme.

![](https://lh6.googleusercontent.com/JQgZUvMDkHcZeAv7WnkXwqdeaYPmEAGig9z5RdO9-igworVa0uIM-OnJ38cGVIoEydzDJGwzU0wj0nFBKqJVVSaaDxcU0mgchPpVzHnQZndQnkBVPlE1C46jK26tIE5qVJc-1mIA)

  

Compatibility with BIP39
------------------------

The original data that is the _secret_ for the SSS implementation is 256-bit entropy (used to generate the BIP39 compatible master seed). It is a 256-bit number generated from two sources of entropy (Microprocessor and ATECC608A microchip). Since the shares can be used to recover the original seed bytes, the implementation is compatible with the BIP39 standard.

The application has support for 12, 18 & 24 words mnemonics (having entropy of 128, 192 & 256 bits respectively). To support these, the secret, if shorter than 32 bytes, is padded with random bytes to make it 32-bytes long. The SSS shares are always generated for a 32-byte secret (entropy padded with random bytes). To recover the original seed, the length of mnemonics is stored along with the shares. So that during the recovery process, only the actual entropy is used (first 128/192/256 bits) and the rest of the random bytes are discarded before the generation of master seed.

  

### **Storing shares on the CyCard**

Once the shares for all the cards are generated, the next step is to encrypt the shares. The encryption is performed only if the user has set any PIN (to lock access to the wallet).

The shares are encrypted using a symmetric encryption algorithm. We use the ChachaPoly (_implementation from trezor’s crypto library_) encryption algorithm for this purpose.

Alongside the encrypted shares, we also store the related NONCE used for the encryption and the calculated MAC. The format is as follows:

<SHARES\_FOR\_THE\_CARD><NONCE><MAC>

The SHARES are 32 bytes in length each

The NONCE is 16 bytes in length (12 bytes for the actual nonce value which is padded to 16 bytes)

The MAC is 16 bytes in length.

  

This makes the total size of each share to be 64 (32 + 16 + 16) bytes on the CyCard (for each wallet).

  

The key that is used for encrypting the shares is derived from the PIN (this is always set as we only encrypt if the user has set any PIN). And the actual _encryption key is single sha256 hash bytes of the original PIN_ entered by the user.

### **Share structure on device**

The device has two structures for handling shares

*   One when it fetches from card (it is probably in encrypted form with mac & nonce)
*   <SHARES\_FOR\_THE\_CARD><NONCE><MAC>
*   Same as the card structure, total of 64-bytes for each individual share.
*   The other is after decrypting the share for use (the mac and nonce are redacted)
*   <SHARES\_FOR\_THE\_CARD>
*   It is 32-byte for each individual share.

The structure of each individual share on X1 Wallet is the same as defined above but since the device needs to regenerate the original seed, the device stores these shares in an array of 4.

  

Depending on the active flow, the device may have one/two/four/five of the encrypted shares or one/two/five of the decrypted shares at any given time.

  

Five shares (encrypted & decrypted): During _new wallet_, _restore wallet_ & _wallet verification_ only

Two shares (encrypted & decrypted): During _import coin_, _send transaction_ and _view seed_ only

Two share (encrypted): During _receive transaction_ only on wallets having PIN

Two share (decrypted): During _receive transaction_ on wallets not having PIN

One share: On all other operations/flows

Intent for encrypting shares w/ first hash of PIN
-------------------------------------------------

Share creation on X1 Wallet is implemented based on plain logic of SSS which does not have a built-in integrity check. Encrypting the individual shares helped us add an integrity check to each of the individual shares. Additionally, we use _sha256(PIN) for encrypting shares_ (64 bytes of data; <SHARES\_FOR\_THE\_CARD><NONCE><MAC>) and _sha256(sha256(PIN)) for authentication_. So if the _double-sha256_ of PIN is breached, then it does not reveal the sha256(PIN) (which is never communicated and stays locally on the X1 Wallet only) hence the encrypted shares will be useless.

Share generation and verification (process)
-------------------------------------------

This entire process takes place locally on the X1 Wallet and the desktop cannot communicate w/ the hardware during this process. Essentially meaning the private data during the process cannot leave the X1 Wallet.

  

Share generation and verification are two sub-processes for one task that is either new wallet creation or restoring old wallet (seed generation does happen in this case) onto the hardware.

### **Share generation**
![](SSS-shares.drawio.svg)  

[SSS shares.drawio.svg](https://t37308523.p.clickup-attachments.com/t37308523/d73701ee-9b22-4245-a3aa-fdf43bd339e7/SSS%20shares.drawio.svg)

###   

```
m := 2	// threshold
n := 5	// shares
byte_array := master_seed
card_shares[n][legnth_of(byte_array)]
For j in byte_array
do: 
      coefficients[n] := RNG(n)    // bytes of random data
      For i in 1 to n
      do: 
            temp := 0
            For k in 0 to (n-1)
            do:
                  temp ^ galois_mul(galois_pow(i+1, n-1-k), coefficients[k])
            card_shares[i][j] := byte_array[j] ^ temp
```

### **Share verification**

```
m := 2	// threshold
n := 5	// shares
seed[32] := {0}
card_shares[n][32]
combination[] := [(0,1), (0,2), (0,3), (0,4), (1,2), (1,3), (1,4), (2,3), (3,4)]  // verify all pairs of shares
For pair in combination
do:
      share1 := card_shares[pair.first]
      share2 := card_shares[pair.second]
      secret := recover_secret(share1, share2)
      If pair == (0,1)
          seed := secret
      else
          If seed != secret
              return failure
return success
```

  

Recovering secret
-----------------

  

![](SSS-regeneration.drawio.svg)

  

[SSS-regeneration.drawio.svg](https://t37308523.p.clickup-attachments.com/t37308523/2b6ef646-c9cc-42b8-9129-a27ee32b62b5/SSS-regeneration.drawio.svg)

  

Source of entropy for SSS
-------------------------

The X1 Wallet uses two sources of entropy for generating the random bytes of data. The SSS implementation used in the X1 Wallet splits each byte into desired number of shares (5 in the case of X1Wallet for 4 smart cards + 1 Device share) which take the one random byte for calculation (generated from two. sources of entropy).

The two sources of entropy being:

*   Microprocessor
*   ATECC608A microchip